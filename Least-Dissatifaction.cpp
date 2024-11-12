#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <bits/stdc++.h>

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


int leastDissatisfaction(const Graph& graph, int maxMatchingSize) {
    int left = 1, right = graph.numHouses, result = right;

    while (left <= right) {
        int mid = (left + right) / 2;

        // Create a restricted graph with only top 'mid' preferences for each agent
        Graph restrictedGraph(graph.numAgents, graph.numHouses);
        for (int a = 1; a <= graph.numAgents; ++a) {
            for (int i = 0; i < min(mid, (int)graph.adj[a].size()); ++i) {
                restrictedGraph.adj[a].push_back(graph.adj[a][i]);
            }
        }

        // Run the maximal matching algorithm on this restricted graph
        vector<int> matchA, matchH;
        int currentMaxMatchingSize = hopcroftKarp(restrictedGraph, matchA, matchH);

        // Check if we have a maximal matchingut
         
        if (maxMatchingSize ==currentMaxMatchingSize ) {
            result = mid;       // Store the minimum value of k that works
            right = mid - 1;    // Try for a smaller k
        } else {
            left = mid + 1;     // Increase k if matching is not maximal
        }
    }

    return result;
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
    int maxMatchingSize = hopcroftKarp(graph, matchA, matchH);
    cout<<leastDissatisfaction(graph,maxMatchingSize)<<endl;

    return 0;
}
