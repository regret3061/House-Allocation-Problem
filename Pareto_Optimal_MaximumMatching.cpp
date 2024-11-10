#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <algorithm>

using namespace std;

// Graph structure
struct Graph {
    int numAgents, numHouses;
    vector<vector<int>> adj;  // adj[a] contains houses that agent a finds acceptable
    Graph(int a, int h) : numAgents(a), numHouses(h), adj(a + 1) {} // +1 for one-based indexing
};

// Phase 1: Hopcroft-Karp Algorithm to find maximal matching
bool bfs(vector<int>& matchA, vector<int>& matchH, vector<int>& dist, const Graph& graph) {
    queue<int> Q;
    for (int a = 1; a <= graph.numAgents; a++) {
        if (matchA[a] == 0) { // Unmatched agents have matchA[a] = 0 in one-based indexing
            dist[a] = 0;
            Q.push(a);
        } else {
            dist[a] = INT_MAX;
        }
    }
    dist[0] = INT_MAX; // Placeholder for unmatched state in one-based indexing

    while (!Q.empty()) {
        int a = Q.front();
        Q.pop();
        if (dist[a] < dist[0]) {
            for (int h : graph.adj[a]) {
                if (dist[matchH[h]] == INT_MAX) {
                    dist[matchH[h]] = dist[a] + 1;
                    Q.push(matchH[h]);
                }
            }
        }
    }
    return dist[0] != INT_MAX;
}

bool dfs(int a, vector<int>& matchA, vector<int>& matchH, vector<int>& dist, const Graph& graph) {
    if (a != 0) {
        for (int h : graph.adj[a]) {
            if (dist[matchH[h]] == dist[a] + 1) {
                if (dfs(matchH[h], matchA, matchH, dist, graph)) {
                    matchH[h] = a;
                    matchA[a] = h;
                    return true;
                }
            }
        }
        dist[a] = INT_MAX;
        return false;
    }
    return true;
}

int hopcroftKarp(const Graph& graph, vector<int>& matchA, vector<int>& matchH) {
    matchA.assign(graph.numAgents + 1, 0); // One-based, 0 means unmatched
    matchH.assign(graph.numHouses + 1, 0); // One-based, 0 means unmatched
    vector<int> dist(graph.numAgents + 1);

    int matchingSize = 0;
    while (bfs(matchA, matchH, dist, graph)) {
        for (int a = 1; a <= graph.numAgents; a++) {
            if (matchA[a] == 0 && dfs(a, matchA, matchH, dist, graph)) {
                matchingSize++;
            }
        }
    }
    return matchingSize;
}

// Phase 2: Make the matching trade-in-free
void makeTradeInFree(vector<int>& matchA, vector<int>& matchH, const Graph& graph) {
    vector<list<pair<int, int>>> prefLists(graph.numHouses + 1); // One-based indexing
    vector<int> curRank(graph.numAgents + 1, -1);
    queue<int> unmatchedHouses;

    for (int a = 1; a <= graph.numAgents; ++a) {
        if (matchA[a] != 0) {
            int h = matchA[a];
            prefLists[h].push_back({a, curRank[a]});
            curRank[a] = find(graph.adj[a].begin(), graph.adj[a].end(), h) - graph.adj[a].begin();
        }
    }

    for (int h = 1; h <= graph.numHouses; ++h) {
        if (matchH[h] == 0 && !prefLists[h].empty()) {
            unmatchedHouses.push(h);
        }
    }

    while (!unmatchedHouses.empty()) {
        int h = unmatchedHouses.front();
        unmatchedHouses.pop();

        while (!prefLists[h].empty()) {
            auto [a, rank] = prefLists[h].front();
            prefLists[h].pop_front();
            if (rank < curRank[a]) {
                int oldH = matchA[a];
                matchA[a] = h;
                matchH[h] = a;
                if (!prefLists[oldH].empty()) {
                    unmatchedHouses.push(oldH);
                }
                break;
            }
        }
    }
}

// Phase 3: Coalition-Free Matching using Top Trading Cycles (TTC) Method
void makeCoalitionFree(vector<int>& matchA, vector<int>& matchH, const Graph& graph) {
    vector<int> preferencePointer(graph.numAgents + 1, 0); // Points to the current preference in agent's list

    while (true) {
        // Step 1: Each agent points to their most preferred house not yet reassigned
        vector<int> agentPointsTo(graph.numAgents + 1, 0); // agent -> house
        vector<int> housePointsTo(graph.numHouses + 1, 0); // house -> agent

        for (int a = 1; a <= graph.numAgents; ++a) {
            if (matchA[a] != 0) { // Skip if already matched optimally
                int preferredHouse = graph.adj[a][preferencePointer[a]]; // Next preferred house
                agentPointsTo[a] = preferredHouse;
                housePointsTo[preferredHouse] = matchH[preferredHouse]; // The agent currently assigned to the house
            }
        }

        // Step 2: Detect cycles in the agent-house assignments
        unordered_set<int> visited;
        bool cycleFound = false;

        for (int a = 1; a <= graph.numAgents; ++a) {
            if (!visited.count(a) && matchA[a] != 0) {
                unordered_set<int> cycleAgents;
                int current = a;

                // Find a cycle by following preferences
                while (current != 0 && !visited.count(current)) {
                    visited.insert(current);
                    cycleAgents.insert(current);
                    int nextHouse = agentPointsTo[current];
                    current = housePointsTo[nextHouse]; // Move to the next agent in the cycle (or 0 if unmatched)
                }

                // Check if a cycle was formed
                if (current != 0 && cycleAgents.count(current)) {
                    cycleFound = true;

                    // Step 3: Resolve the cycle by reassigning agents to houses within the cycle
                    int cycleStart = current;
                    do {
                        int assignedHouse = agentPointsTo[cycleStart];
                        matchA[cycleStart] = assignedHouse;
                        matchH[assignedHouse] = cycleStart;
                        cycleStart = housePointsTo[assignedHouse];
                    } while (cycleStart != current);

                    break; // Exit to restart cycle detection
                }
            }
        }

        // If no more cycles are found, stop the process
        if (!cycleFound) break;

        // Step 4: Update preference pointers to the next available choices for unmatched agents
        for (int a = 1; a <= graph.numAgents; ++a) {
            if (matchA[a] != 0) {
                preferencePointer[a]++;
            }
        }
    }
}

// Main function to execute the algorithm
int main() {
    int numAgents, numHouses, numPref;
    cin >> numAgents >> numHouses;
    Graph graph(numAgents, numHouses);
    for (int i = 0; i < numAgents; i++) {
        cin >> numPref;
        for (int j = 0; j < numPref; j++) {
            int h;
            cin >> h;
            graph.adj[i + 1].push_back(h); // add the agent-house edge
        }
    }

    vector<int> matchA, matchH;

    // Phase 1: Find maximal matching
    int maxMatchingSize = hopcroftKarp(graph, matchA, matchH);
    cout << "Phase 1: Maximal Matching\n";
    for (int a = 1; a <= numAgents; ++a) {
        if (matchA[a] != 0) {
            cout << "Agent " << a << " is assigned to House " << matchA[a] << "\n";
        }
    }

    // Phase 2: Make the matching trade-in-free
    makeTradeInFree(matchA, matchH, graph);
    cout << "Phase 2: Trade-In-Free Matching\n";
    for (int a = 1; a <= numAgents; ++a) {
        if (matchA[a] != 0) {
            cout << "Agent " << a << " is assigned to House " << matchA[a] << "\n";
        }
    }

    // Phase 3: Make the matching coalition-free
    makeCoalitionFree(matchA, matchH, graph);
    cout << "Phase 3: Pareto Optimal Matching\n";
    for (int a = 1; a <= numAgents; ++a) {
        if (matchA[a] != 0) {
            cout << "Agent " << a << " is assigned to House " << matchA[a] << "\n";
        }
    }

    return 0;
}
