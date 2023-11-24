#include "directed_graph.h"
#include <iostream>

struct Point {
    int x{};
    int y{};

    bool operator==(const Point& other) const {
        return x == other.x and y == other.y;
    }
};

std::ostream& operator<<(std::ostream& os, const Point p) {
    os << "(" << p.x << "," << p.y << ")";
    return os;
}

int main() {
    directed_graph<int> graph;
    // Insert some nodes and edges.
    graph.insert(11);
    graph.insert(22);
    graph.insert(33);
    graph.insert(44);
    graph.insert(55);
    graph.insert(66);
    graph.insert(77);
    graph.insert_edge(11, 33);
    graph.insert_edge(11, 77);
    graph.insert_edge(22, 33);
    graph.insert_edge(22, 44);
    graph.insert_edge(22, 55);
    graph.insert_edge(22, 66);
    graph.insert_edge(33, 44);
    graph.insert_edge(33, 77);
    graph.insert_edge(44, 55);
    graph.insert_edge(44, 66);
    graph.insert_edge(44, 77);
    std::wcout << to_dot(graph, L"Graph1");

    // Remove an edge and a node.
    graph.erase_edge(22, 44);
    graph.erase(44);
    std::wcout << to_dot(graph, L"Graph1");

    // Print the size of the graph.
    std::cout << "Size: " << graph.size() << std::endl;

    // Try to insert a node
    auto [_, inserted]{graph.insert(2002)};
    if (!inserted) { std::cout << "Duplicate element!\n"; }

    for (auto iter{graph.cbegin()}; iter != graph.cend(); ++iter) {
        std::cout << *iter << " ";
    }
    std::cout << std::endl;

    for (auto& node: graph) { std::cout << node << std::endl; }
    std::cout << std::endl;

    auto result{std::find(std::begin(graph), std::end(graph), 33)};
    if (result != std::end(graph)) {
        std::cout << "Found " << *result << std::endl;
    } else {
        std::cout << "Not found!" << std::endl;
    }

    auto count{std::count_if(std::begin(graph), std::end(graph),
                             [](const auto& node) { return node > 22; })};
    std::cout << count << std::endl;

    for (auto iter{graph.rbegin()}; iter != graph.rend(); ++iter) {
        std::cout << *iter << " ";
    }
    std::cout << std::endl;

    std::cout << "Adjacency list for node 22: ";

    auto iterBegin{graph.cbegin(22)};
    auto iterEnd{graph.cend(22)};

    if (iterBegin == iterEnd) {
        std::cout << "Value 22 not found." << std::endl;
        std::cout << std::boolalpha << (iterBegin == iterEnd) << '\n';
    } else {
        for (auto iter{iterBegin}; iter != iterEnd; ++iter) {
            std::cout << *iter << " ";
        }
    }
    std::cout << '\n';

    return 0;
}
