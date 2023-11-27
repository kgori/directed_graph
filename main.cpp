#include "directed_graph.h"
#include "weighted_directed_graph.h"
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <vector>

struct Point {
    int x{};
    int y{};

    bool operator==(const Point& other) const {
        return x == other.x and y == other.y;
    }
};

template<typename T, typename A>
void dfs(const weighted_directed_graph<T, A>& graph, const T& start_node) {
    std::stack<T> stack;
    std::set<T> visited;

    stack.push(start_node);
    visited.insert(start_node);

    while (!stack.empty()) {
        T current_node = stack.top();
        stack.pop();
        // Process current_node here
        std::cout << "DFS:" << current_node << '\n';

        for (const auto& neighbor:
             graph.get_adjacent_nodes_values(current_node)) {
            if (visited.find(neighbor) == visited.end()) {
                stack.push(neighbor);
                visited.insert(neighbor);
            }
        }
    }
}

template<typename T, typename A>
void bfs(const weighted_directed_graph<T, A>& graph, const T& start_node) {
    std::queue<T> queue;
    std::set<T> visited;

    queue.push(start_node);
    visited.insert(start_node);

    while (!queue.empty()) {
        T current_node = queue.front();
        queue.pop();
        // Process current_node here
        std::cout << "BFS:" << current_node << '\n';

        for (const auto& neighbor:
             graph.get_adjacent_nodes_values(current_node)) {
            if (visited.find(neighbor) == visited.end()) {
                queue.push(neighbor);
                visited.insert(neighbor);
            }
        }
    }
}

template<typename T, typename A>
std::map<T, double> dijkstra(const weighted_directed_graph<T, A>& graph,
                             const T& start_node) {
    std::priority_queue<std::pair<double, T>, std::vector<std::pair<double, T>>,
                        std::greater<>>
        pq;
    std::map<T, double> distances;
    std::set<T> visited;

    pq.push({ 0, start_node });
    distances[start_node] = 0;

    while (!pq.empty()) {
        double dist = pq.top().first;
        T current_node = pq.top().second;
        pq.pop();

        if (visited.find(current_node) != visited.end()) { continue; }
        visited.insert(current_node);

        std::cout << "IJK:" << current_node << '\n';

        for (const auto& [neighbor_node, edge_weight]:
             graph.get_adjacent_nodes_values_and_weights(current_node)) {
            double new_dist = dist + edge_weight;
            if (distances.find(neighbor_node) == distances.end() ||
                new_dist < distances[neighbor_node]) {
                distances[neighbor_node] = new_dist;
                pq.push({ new_dist, neighbor_node });
            }
        }
    }

    return distances;
}

template<typename T, typename A>
std::vector<T> dijkstra(const weighted_directed_graph<T, A>& graph,
                        const T& start_node, const T& end_node) {
    if (std::find(std::begin(graph), std::end(graph), start_node) ==
        std::end(graph)) {
        std::cerr << "Start node [" << start_node << "] is not in this graph";
        return std::vector<T>{};
    }

    if (std::find(std::begin(graph), std::end(graph), end_node) ==
        std::end(graph)) {
        std::cerr << "End node [" << end_node << "] is not in this graph";
        return std::vector<T>{};
    }

    std::priority_queue<std::pair<double, T>, std::vector<std::pair<double, T>>,
                        std::greater<>>
        pq;
    std::map<T, double> distances;
    std::map<T, T>
        previous;// Map to store the previous node in the shortest path.
    std::set<T> visited;

    pq.push({ 0, start_node });
    distances[start_node] = 0;

    while (!pq.empty()) {
        double dist = pq.top().first;
        T current_node = pq.top().second;
        pq.pop();

        if (visited.find(current_node) != visited.end()) { continue; }
        visited.insert(current_node);

        if (current_node == end_node) { break; }

        for (const auto& [neighbor_node, edge_weight]:
             graph.get_adjacent_nodes_values_and_weights(current_node)) {
            double new_dist = dist + edge_weight;
            if (distances.find(neighbor_node) == distances.end() ||
                new_dist < distances[neighbor_node]) {
                distances[neighbor_node] = new_dist;
                previous[neighbor_node] = current_node;
                pq.push({ new_dist, neighbor_node });
            }
        }
    }

    // Reconstruct the path
    std::vector<T> path;
    T current = end_node;
    while (current != start_node) {
        path.push_back(current);
        current = previous[current];
    }
    path.push_back(
        start_node);// Final step - complete the path with the start node
    std::reverse(path.begin(), path.end());

    return path;
}

std::ostream& operator<<(std::ostream& os, const Point p) {
    os << "(" << p.x << "," << p.y << ")";
    return os;
}

int main() {
    weighted_directed_graph<int> graph;
    // Insert some nodes and edges.
    graph.insert(11);
    graph.insert(22);
    graph.insert(33);
    graph.insert(44);
    graph.insert(55);
    graph.insert(66);
    graph.insert(77);
    graph.insert(88);
    graph.insert_edge(11, 22, 2.0);
    graph.insert_edge(11, 55, 1.0);
    graph.insert_edge(22, 33, 3.0);
    graph.insert_edge(22, 66, 1.0);
    graph.insert_edge(33, 44, 1.0);
    graph.insert_edge(44, 88, 9.0);
    graph.insert_edge(55, 66, 1.0);
    graph.insert_edge(55, 77, 3.0);
    graph.insert_edge(66, 77, 1.0);
    graph.insert_edge(77, 44, 1.0);
    std::wcout << to_dot(graph, L"Graph1");

    // Remove an edge and a node.
    graph.erase_edge(44, 88);
    graph.erase(88);
    std::wcout << to_dot(graph, L"Graph1");

    // Print the size of the graph.
    std::cout << "Size: " << graph.size() << std::endl;

    // Try to insert a node
    auto [_, inserted]{ graph.insert(2002) };
    if (!inserted) { std::cout << "Duplicate element!\n"; }

    for (auto iter{ graph.cbegin() }; iter != graph.cend(); ++iter) {
        std::cout << *iter << " ";
    }
    std::cout << std::endl;

    for (auto& node: graph) { std::cout << node << std::endl; }
    std::cout << std::endl;

    auto result{ std::find(std::begin(graph), std::end(graph), 44) };
    if (result != std::end(graph)) {
        std::cout << "Found " << *result << std::endl;
    } else {
        std::cout << "Not found!" << std::endl;
    }

    auto count{ std::count_if(std::begin(graph), std::end(graph),
                              [](const auto& node) { return node > 22; }) };
    std::cout << count << std::endl;

    for (auto iter{ graph.rbegin() }; iter != graph.rend(); ++iter) {
        std::cout << *iter << " ";
    }
    std::cout << std::endl;

    std::cout << "Adjacency list for node 22: ";

    auto iterBegin{ graph.cbegin(22) };
    auto iterEnd{ graph.cend(22) };

    if (iterBegin == iterEnd) {
        std::cout << "Value 22 not found." << std::endl;
        std::cout << std::boolalpha << (iterBegin == iterEnd) << '\n';
    } else {
        for (auto iter{ iterBegin }; iter != iterEnd; ++iter) {
            auto [a, b] = *iter;
            std::cout << "(" << a << "=" << b << ") ";
        }
    }
    std::cout << '\n';

    std::vector<int> shortest_path = dijkstra(graph, 11, 45);
    for (int node: shortest_path) { std::cout << "IJK:" << node << '\n'; }

    std::cout << std::endl;
    return 0;
}
