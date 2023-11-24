//
// Created by Kevin Gori on 14/11/2023.
//

#pragma once
#include <algorithm>
#include <format>
#include <iterator>
#include <set>
#include <sstream>
#include <vector>

template<typename T>
class directed_graph;

template<typename DirectedGraph>
class const_directed_graph_iterator;

template<typename DirectedGraph>
class directed_graph_iterator;

template<typename GraphType>
class const_adjacent_nodes_iterator;

template<typename GraphType>
class adjacent_nodes_iterator;

namespace details {
    template<typename T, typename A = std::allocator<T>>
    class graph_node {
    public:
        // Constructors
        graph_node(directed_graph<T, A>* graph, const T& t);

        graph_node(directed_graph<T, A>* graph, T&& t);

        graph_node(directed_graph<T, A>* graph, const T& t, const A& allocator);

        graph_node(directed_graph<T, A>* graph, T&& t, const A& allocator);

        ~graph_node();

        // Copy and move constructors
        graph_node(const graph_node& src);

        graph_node(graph_node&& src) noexcept;

        // Copy and move assignment
        graph_node& operator=(const graph_node& rhs);

        graph_node& operator=(graph_node&& rhs) noexcept;

        // Getters
        [[nodiscard]] T& value() noexcept;

        [[nodiscard]] const T& value() const noexcept;

        // C++20 defaulted equality comparison
        bool operator==(const graph_node&) const = default;

    private:
        friend class directed_graph<T, A>;

        // Pointer to the graph this node belongs to
        directed_graph<T, A>* m_graph;

        // Type alias for the container type used to store nodes
        using adjacency_list_type = std::set<size_t>;

        // A ref to the adjacency list
        [[nodiscard]] adjacency_list_type& get_adjacent_nodes_indices();

        [[nodiscard]] const adjacency_list_type&
        get_adjacent_nodes_indices() const;

        A m_allocator;
        T* m_data{nullptr};
        adjacency_list_type m_adjacentNodeIndices;
    };

    template<typename T>
    graph_node<T>::graph_node(directed_graph<T>* graph, const T& t)
        : m_graph{graph}, m_data{t} {}

    template<typename T>
    graph_node<T>::graph_node(directed_graph<T>* graph, T&& t)
        : m_graph{graph}, m_data{std::move(t)} {}

    template<typename T>
    T& graph_node<T>::value() noexcept {
        return m_data;
    }

    template<typename T>
    const T& graph_node<T>::value() const noexcept {
        return m_data;
    };

    template<typename T>
    typename graph_node<T>::adjacency_list_type&
    graph_node<T>::get_adjacent_nodes_indices() {
        return m_adjacentNodeIndices;
    }

    template<typename T>
    const typename graph_node<T>::adjacency_list_type&
    graph_node<T>::get_adjacent_nodes_indices() const {
        return m_adjacentNodeIndices;
    }
}// namespace details

template<typename T>
class directed_graph {
public:
    // Necessary type aliases for directed_graph to function as an STL container
    using value_type = T;
    using reference = value_type&;
    using const_reference = const value_type&;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    // Aliases required for iterator support - both are const_ on purpose,
    // to be like std::set in disallowing modification of elements (and
    // the implementation of the directed graph class uss std::set internally)
    using iterator = const_directed_graph_iterator<directed_graph>;
    using const_iterator = const_directed_graph_iterator<directed_graph>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using iterator_adjacent_nodes = adjacent_nodes_iterator<directed_graph>;
    using const_iterator_adjacent_nodes =
        const_adjacent_nodes_iterator<directed_graph>;
    using reverse_iterator_adjacent_nodes =
        std::reverse_iterator<iterator_adjacent_nodes>;
    using const_reverse_iterator_adjacent_nodes =
        std::reverse_iterator<const_iterator_adjacent_nodes>;

    // Iterator methods
    iterator begin() noexcept;

    iterator end() noexcept;

    const_iterator begin() const noexcept;

    const_iterator end() const noexcept;

    const_iterator cbegin() const noexcept;

    const_iterator cend() const noexcept;

    reverse_iterator rbegin() noexcept;

    reverse_iterator rend() noexcept;

    const_reverse_iterator rbegin() const noexcept;

    const_reverse_iterator rend() const noexcept;

    const_reverse_iterator crbegin() const noexcept;

    const_reverse_iterator crend() const noexcept;

    iterator_adjacent_nodes begin(const T& node_value) noexcept;

    iterator_adjacent_nodes end(const T& node_value) noexcept;

    const_iterator_adjacent_nodes begin(const T& node_value) const noexcept;

    const_iterator_adjacent_nodes end(const T& node_value) const noexcept;

    const_iterator_adjacent_nodes cbegin(const T& node_value) const noexcept;

    const_iterator_adjacent_nodes cend(const T& node_value) const noexcept;

    reverse_iterator_adjacent_nodes rbegin(const T& node_value) noexcept;

    reverse_iterator_adjacent_nodes rend(const T& node_value) noexcept;

    const_reverse_iterator_adjacent_nodes
    rbegin(const T& node_value) const noexcept;

    const_reverse_iterator_adjacent_nodes
    rend(const T& node_value) const noexcept;

    const_reverse_iterator_adjacent_nodes
    crbegin(const T& node_value) const noexcept;

    const_reverse_iterator_adjacent_nodes
    crend(const T& node_value) const noexcept;

    // Returns true if the node is successfully inserted. If the node is
    // already present, then false is returned.
    std::pair<iterator, bool> insert(T&& node_value);

    std::pair<iterator, bool> insert(const T& node_value);

    iterator insert(const_iterator hint, T&& node_value);

    iterator insert(const_iterator hint, const T& node_value);

    template<typename Iter>
    void insert(Iter first, Iter last);

    // Returns true if the given node is erased
    bool erase(const T& node_value);

    iterator erase(const_iterator pos);

    iterator erase(const_iterator first, const_iterator last);

    // Returns true if the edge was inserted successfully
    bool insert_edge(const T& from_node_value, const T& to_node_value);

    // Returns true if the edge is removed successfully
    bool erase_edge(const T& from_node_value, const T& to_node_value);

    // Empties the graph
    void clear() noexcept;

    // Returns a reference to the object at index. No bounds checking.
    reference operator[](size_t index);

    const_reference operator[](size_t index) const;

    // Bounds-checking equivalents to operator[]
    reference at(size_type index);

    const_reference at(size_type index) const;

    // Graphs are equal if they contain the same nodes and edges,
    // regardless of order
    bool operator==(const directed_graph& rhs) const;

    bool operator!=(const directed_graph& rhs) const;

    // Swaps all nodes between this graph and other_graph
    void swap(directed_graph& other_graph) noexcept;

    [[nodiscard]] size_type size() const noexcept;

    [[nodiscard]] size_type max_size() const noexcept;

    [[nodiscard]] bool empty() const noexcept;

    // Returns a set with the values of the nodes connected to the node with
    // node_value
    [[nodiscard]] std::set<T>
    get_adjacent_nodes_values(const T& node_value) const;

private:
    friend class const_directed_graph_iterator<directed_graph>;
    friend class directed_graph_iterator<directed_graph>;

    using nodes_container_type = std::vector<details::graph_node<T>>;
    nodes_container_type m_nodes;

    // Returns an iterator at the searched-for value, or the end iterator
    // if the value is not found.
    typename nodes_container_type::iterator findNode(const T& node_value);

    typename nodes_container_type::const_iterator
    findNode(const T& node_value) const;

    size_t
    get_index_of_node(const typename nodes_container_type::const_iterator& node)
        const noexcept;

    void remove_all_links_to(
        typename nodes_container_type::const_iterator node_iter);

    std::set<T> get_adjacent_nodes_values(
        const typename details::graph_node<T>::adjacency_list_type& indices)
        const;
};

template<typename T>
typename directed_graph<T>::nodes_container_type::iterator
directed_graph<T>::findNode(const T& node_value) {
    return std::find_if(
        std::begin(m_nodes), std::end(m_nodes),
        [&node_value](const auto& node) { return node.value() == node_value; });
}

template<typename T>
typename directed_graph<T>::nodes_container_type::const_iterator
directed_graph<T>::findNode(const T& node_value) const {
    return const_cast<directed_graph<T>*>(this)->findNode(node_value);
}

template<typename T>
std::pair<typename directed_graph<T>::iterator, bool>
directed_graph<T>::insert(T&& node_value) {
    auto iter{findNode(node_value)};
    if (iter != std::end(m_nodes)) {
        // Value is already in the graph
        return {iterator{iter, this}, false};
    }
    m_nodes.emplace_back(this, std::move(node_value));
    return {iterator{--std::end(m_nodes), this}, true};
}

template<typename T>
std::pair<typename directed_graph<T>::iterator, bool>
directed_graph<T>::insert(const T& node_value) {
    T copy{node_value};
    return insert(std::move(copy));
}

template<typename T>
typename directed_graph<T>::iterator
directed_graph<T>::insert(const_iterator hint, T&& node_value) {
    // Ignore the hint, just forward to standard insert.
    return insert(node_value).first;
}

template<typename T>
typename directed_graph<T>::iterator
directed_graph<T>::insert(const_iterator hint, const T& node_value) {
    return insert(node_value).first;
}

// Nested templates - can't use template<typename T, typename Iter>
template<typename T>
template<typename Iter>
void directed_graph<T>::insert(Iter first, Iter last) {
    // Copy each element in the range by using an insert_iterator
    std::copy(first, last, std::insert_iterator{*this, begin()});
}

template<typename T>
bool directed_graph<T>::insert_edge(const T& from_node_value,
                                    const T& to_node_value) {
    const auto from{findNode(from_node_value)};
    const auto to{findNode(to_node_value)};
    if (from == std::end(m_nodes) || to == std::end(m_nodes)) { return false; }
    const size_t to_index{get_index_of_node(to)};
    return from->get_adjacent_nodes_indices().insert(to_index).second;
}

template<typename T>
size_t directed_graph<T>::get_index_of_node(
    const typename nodes_container_type::const_iterator& node) const noexcept {
    const auto index{std::distance(std::cbegin(m_nodes), node)};
    return static_cast<size_t>(index);
}

template<typename T>
void directed_graph<T>::remove_all_links_to(
    typename nodes_container_type::const_iterator node_iter) {
    const size_t node_index{get_index_of_node(node_iter)};

    // Iterate over all adjacency lists of all nodes
    for (auto&& node: m_nodes) {
        auto& adjacencyIndices{node.get_adjacent_nodes_indices()};
        // Remove references from to-be-deleted node
        adjacencyIndices.erase(node_index);
        // Modify adjacency indices to account for deletion
        // Some inefficiency, as data is converted to a vector,
        // indices are adjusted, and the set is wiped and rebuilt
        // from the vector (values in sets are immutable).
        std::vector<size_t> indices(std::begin(adjacencyIndices),
                                    std::end(adjacencyIndices));
        std::for_each(std::begin(indices), std::end(indices),
                      [node_index](size_t& index) {
                          if (index > node_index) { --index; }
                      });
        adjacencyIndices.clear();
        adjacencyIndices.insert(std::begin(indices), std::end(indices));
    }
}

template<typename T>
bool directed_graph<T>::erase(const T& node_value) {
    auto iter{findNode(node_value)};
    if (iter == std::end(m_nodes)) { return false; }
    remove_all_links_to(iter);
    m_nodes.erase(iter);
    return true;
}

template<typename T>
typename directed_graph<T>::iterator
directed_graph<T>::erase(const_iterator pos) {
    if (pos.m_nodeIterator == std::end(m_nodes)) {
        return iterator{std::end(m_nodes), this};
    }
    remove_all_links_to(pos.m_nodeIterator);
    return iterator{m_nodes.erase(pos.m_nodeIterator), this};
}

template<typename T>
typename directed_graph<T>::iterator
directed_graph<T>::erase(const_iterator first, const_iterator last) {
    for (auto iter{first}; iter != last; ++iter) {
        if (iter.m_nodeIterator != std::end(m_nodes)) {
            remove_all_links_to(iter.m_nodeIterator);
        }
    }
    return iterator{m_nodes.erase(first.m_nodeIterator, last.m_nodeIterator),
                    this};
}

template<typename T>
bool directed_graph<T>::erase_edge(const T& from_node_value,
                                   const T& to_node_value) {
    const auto from{findNode(from_node_value)};
    const auto to{findNode(to_node_value)};
    if (from == std::end(m_nodes) || to == std::end(m_nodes)) { return false; }
    const size_t to_index{get_index_of_node(to)};
    from->get_adjacent_nodes_indices().erase(to_index);
    return true;
}

template<typename T>
void directed_graph<T>::clear() noexcept {
    m_nodes.clear();
}

template<typename T>
void directed_graph<T>::swap(directed_graph& other_graph) noexcept {
    m_nodes.swap(other_graph.m_nodes);
}

template<typename T>
typename directed_graph<T>::reference
directed_graph<T>::operator[](size_t index) {
    return m_nodes[index].value();
}

template<typename T>
typename directed_graph<T>::const_reference
directed_graph<T>::operator[](size_type index) const {
    return m_nodes[index].value();
}

template<typename T>
typename directed_graph<T>::reference directed_graph<T>::at(size_type index) {
    return m_nodes.at(index).value();
}

template<typename T>
typename directed_graph<T>::const_reference
directed_graph<T>::at(directed_graph::size_type index) const {
    return m_nodes.at(index).value();
}

template<typename T>
bool directed_graph<T>::operator==(const directed_graph& rhs) const {
    if (m_nodes.size() != rhs.m_nodes.size()) { return false; }

    for (auto&& node: m_nodes) {
        const auto rhsNodeIter{rhs.findNode(node.value())};
        if (rhsNodeIter == std::end(rhs.m_nodes)) { return false; }
        const auto adjacent_values_lhs{
            get_adjacent_nodes_values(node.get_adjacent_nodes_indices())};
        const auto adjacent_values_rhs{rhs.get_adjacent_nodes_values(
            rhsNodeIter->get_adjacent_nodes_indices())};
        if (adjacent_values_lhs != adjacent_values_rhs) { return false; }
    }
    return true;
}

template<typename T>
std::set<T> directed_graph<T>::get_adjacent_nodes_values(
    const typename details::graph_node<T>::adjacency_list_type& indices) const {
    std::set<T> values;
    for (auto&& index: indices) { values.insert(m_nodes[index].value()); }
    return values;
}

template<typename T>
bool directed_graph<T>::operator!=(const directed_graph<T>& rhs) const {
    return !(*this == rhs);
}

template<typename T>
std::set<T>
directed_graph<T>::get_adjacent_nodes_values(const T& node_value) const {
    auto iter{findNode(node_value)};
    if (iter == std::end(m_nodes)) { return std::set<T>{}; }
    return get_adjacent_nodes_values(iter->get_adjacent_nodes_indices());
}

template<typename T>
typename directed_graph<T>::size_type directed_graph<T>::size() const noexcept {
    return m_nodes.size();
}

template<typename T>
bool directed_graph<T>::empty() const noexcept {
    return m_nodes.empty();
}

template<typename T>
typename directed_graph<T>::size_type
directed_graph<T>::max_size() const noexcept {
    return m_nodes.max_size();
}

template<typename T>
typename directed_graph<T>::iterator_adjacent_nodes
directed_graph<T>::begin(const T& node_value) noexcept {
    auto iter{findNode(node_value)};
    if (iter == std::end(m_nodes)) {
        // Default-construct an end iterator, and return
        return iterator_adjacent_nodes{};
    }
    return iterator_adjacent_nodes{
        std::begin(iter->get_adjacent_nodes_indices()), this};
}

template<typename T>
typename directed_graph<T>::iterator_adjacent_nodes
directed_graph<T>::end(const T& node_value) noexcept {
    auto iter{findNode(node_value)};
    if (iter == std::end(m_nodes)) { return iterator_adjacent_nodes{}; }
    return iterator_adjacent_nodes{std::end(iter->get_adjacent_nodes_indices()),
                                   this};
}

template<typename T>
typename directed_graph<T>::const_iterator_adjacent_nodes
directed_graph<T>::cbegin(const T& node_value) const noexcept {
    auto iter{findNode(node_value)};
    if (iter == std::end(m_nodes)) {
        // Default-construct an end iterator, and return
        return const_iterator_adjacent_nodes{};
    }
    return const_iterator_adjacent_nodes{
        std::begin(iter->get_adjacent_nodes_indices()), this};
}

template<typename T>
typename directed_graph<T>::const_iterator_adjacent_nodes
directed_graph<T>::cend(const T& node_value) const noexcept {
    auto iter{findNode(node_value)};
    if (iter == std::end(m_nodes)) { return const_iterator_adjacent_nodes{}; }
    return const_iterator_adjacent_nodes{
        std::end(iter->get_adjacent_nodes_indices()), this};
}

template<typename T>
typename directed_graph<T>::const_iterator_adjacent_nodes
directed_graph<T>::begin(const T& node_value) const noexcept {
    return cbegin(node_value);
}

template<typename T>
typename directed_graph<T>::const_iterator_adjacent_nodes
directed_graph<T>::end(const T& node_value) const noexcept {
    return cend(node_value);
}

template<typename T>
typename directed_graph<T>::reverse_iterator_adjacent_nodes
directed_graph<T>::rbegin(const T& node_value) noexcept {
    return reverse_iterator_adjacent_nodes{end(node_value)};
}

template<typename T>
typename directed_graph<T>::reverse_iterator_adjacent_nodes
directed_graph<T>::rend(const T& node_value) noexcept {
    return reverse_iterator_adjacent_nodes{begin(node_value)};
}

template<typename T>
typename directed_graph<T>::const_reverse_iterator_adjacent_nodes
directed_graph<T>::rbegin(const T& node_value) const noexcept {
    return const_reverse_iterator_adjacent_nodes{end(node_value)};
}

template<typename T>
typename directed_graph<T>::const_reverse_iterator_adjacent_nodes
directed_graph<T>::rend(const T& node_value) const noexcept {
    return const_reverse_iterator_adjacent_nodes{begin(node_value)};
}

template<typename T>
typename directed_graph<T>::const_reverse_iterator_adjacent_nodes
directed_graph<T>::crbegin(const T& node_value) const noexcept {
    return rbegin(node_value);
}

template<typename T>
typename directed_graph<T>::const_reverse_iterator_adjacent_nodes
directed_graph<T>::crend(const T& node_value) const noexcept {
    return rend(node_value);
}

template<typename T>
typename directed_graph<T>::iterator directed_graph<T>::begin() noexcept {
    return iterator{std::begin(m_nodes), this};
}

template<typename T>
typename directed_graph<T>::iterator directed_graph<T>::end() noexcept {
    return iterator{std::end(m_nodes), this};
}

template<typename T>
typename directed_graph<T>::const_iterator
directed_graph<T>::begin() const noexcept {
    return const_cast<directed_graph*>(this)->begin();
}

template<typename T>
typename directed_graph<T>::const_iterator
directed_graph<T>::end() const noexcept {
    return const_cast<directed_graph*>(this)->end();
}

template<typename T>
typename directed_graph<T>::const_iterator
directed_graph<T>::cbegin() const noexcept {
    return begin();
}

template<typename T>
typename directed_graph<T>::const_iterator
directed_graph<T>::cend() const noexcept {
    return end();
}

template<typename T>
typename directed_graph<T>::reverse_iterator
directed_graph<T>::rbegin() noexcept {
    return reverse_iterator{end()};
}

template<typename T>
typename directed_graph<T>::reverse_iterator
directed_graph<T>::rend() noexcept {
    return reverse_iterator{begin()};
}

template<typename T>
typename directed_graph<T>::const_reverse_iterator
directed_graph<T>::rbegin() const noexcept {
    return const_reverse_iterator{end()};
}

template<typename T>
typename directed_graph<T>::const_reverse_iterator
directed_graph<T>::rend() const noexcept {
    return const_reverse_iterator{begin()};
}

template<typename T>
typename directed_graph<T>::const_reverse_iterator
directed_graph<T>::crbegin() const noexcept {
    return rbegin();
}

template<typename T>
typename directed_graph<T>::const_reverse_iterator
directed_graph<T>::crend() const noexcept {
    return rend();
}

template<typename T>
std::wstring to_dot(const directed_graph<T>& graph,
                    std::wstring_view graph_name) {
    std::wstringstream wss;
    wss << std::format(L"digraph {} {{", graph_name.data()) << std::endl;
    for (auto&& node: graph) {
        const auto b{graph.cbegin(node)};
        const auto e{graph.cend(node)};
        if (b == e) {
            wss << node << std::endl;
        } else {
            for (auto iter{b}; iter != e; ++iter) {
                wss << std::format(L"{} -> {}", node, *iter) << std::endl;
            }
        }
    }
    wss << "}" << std::endl;
    return wss.str();
}

template<typename DirectedGraph>
class const_directed_graph_iterator {
public:
    using value_type = typename DirectedGraph::value_type;
    using difference_type = ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;
    using pointer = const value_type*;
    using reference = const value_type&;
    using iterator_type =
        typename DirectedGraph::nodes_container_type::const_iterator;

    // This constructor is a necessary feature of a bidirectional iterator,
    // so we can just default it.
    const_directed_graph_iterator() = default;

    const_directed_graph_iterator(iterator_type it, const DirectedGraph* graph);

    reference operator*() const;

    pointer operator->() const;

    const_directed_graph_iterator& operator++();

    const_directed_graph_iterator operator++(int);

    const_directed_graph_iterator& operator--();

    const_directed_graph_iterator operator--(int);

    bool operator==(const const_directed_graph_iterator&) const = default;

protected:
    friend class directed_graph<value_type>;

    iterator_type m_nodeIterator;
    const DirectedGraph* m_graph{nullptr};

    // Helper methods for ++ and --
    void increment();

    void decrement();
};

template<typename DirectedGraph>
const_directed_graph_iterator<DirectedGraph>::const_directed_graph_iterator(
    iterator_type it, const DirectedGraph* graph)
    : m_nodeIterator{it}, m_graph{graph} {}

template<typename DirectedGraph>
typename const_directed_graph_iterator<DirectedGraph>::reference
const_directed_graph_iterator<DirectedGraph>::operator*() const {
    return m_nodeIterator->value();
}

template<typename DirectedGraph>
typename const_directed_graph_iterator<DirectedGraph>::pointer
const_directed_graph_iterator<DirectedGraph>::operator->() const {
    return &(m_nodeIterator->value());
}

// Defer details to the increment helper
template<typename DirectedGraph>
const_directed_graph_iterator<DirectedGraph>&
const_directed_graph_iterator<DirectedGraph>::operator++() {
    increment();
    return *this;
}

template<typename DirectedGraph>
const_directed_graph_iterator<DirectedGraph>
const_directed_graph_iterator<DirectedGraph>::operator++(int) {
    auto oldIt{*this};
    increment();
    return oldIt;
}

template<typename DirectedGraph>
const_directed_graph_iterator<DirectedGraph>&
const_directed_graph_iterator<DirectedGraph>::operator--() {
    decrement();
    return *this;
}

template<typename DirectedGraph>
const_directed_graph_iterator<DirectedGraph>
const_directed_graph_iterator<DirectedGraph>::operator--(int) {
    auto oldIt{*this};
    decrement();
    return oldIt;
}

template<typename DirectedGraph>
void const_directed_graph_iterator<DirectedGraph>::increment() {
    ++m_nodeIterator;
}

template<typename DirectedGraph>
void const_directed_graph_iterator<DirectedGraph>::decrement() {
    --m_nodeIterator;
}

template<typename DirectedGraph>
class directed_graph_iterator
    : public const_directed_graph_iterator<DirectedGraph> {
public:
    using value_type = DirectedGraph::value_type;
    using difference_type = ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_type =
        typename DirectedGraph::nodes_container_type::iterator;

    directed_graph_iterator() = default;

    directed_graph_iterator(iterator_type it, const DirectedGraph* graph);

    reference operator*();

    pointer operator->();

    directed_graph_iterator& operator++();

    directed_graph_iterator operator++(int);

    directed_graph_iterator& operator--();

    directed_graph_iterator operator--(int);
};

template<typename DirectedGraph>
directed_graph_iterator<DirectedGraph>::directed_graph_iterator(
    directed_graph_iterator::iterator_type it, const DirectedGraph* graph)
    : const_directed_graph_iterator<DirectedGraph>{it, graph} {}

template<typename DirectedGraph>
typename directed_graph_iterator<DirectedGraph>::reference
directed_graph_iterator<DirectedGraph>::operator*() {
    return const_cast<reference>(this->m_nodeIterator->value());
}

template<typename DirectedGraph>
typename directed_graph_iterator<DirectedGraph>::pointer
directed_graph_iterator<DirectedGraph>::operator->() {
    return const_cast<pointer>(&(this->m_nodeIterator->value()));
}

template<typename DirectedGraph>
directed_graph_iterator<DirectedGraph>&
directed_graph_iterator<DirectedGraph>::operator++() {
    this->increment();
    return *this;
}

template<typename DirectedGraph>
directed_graph_iterator<DirectedGraph>
directed_graph_iterator<DirectedGraph>::operator++(int) {
    auto oldIt{*this};
    this->increment();
    return oldIt;
}

template<typename DirectedGraph>
directed_graph_iterator<DirectedGraph>&
directed_graph_iterator<DirectedGraph>::operator--() {
    this->decrement();
    return *this;
}

template<typename DirectedGraph>
directed_graph_iterator<DirectedGraph>
directed_graph_iterator<DirectedGraph>::operator--(int) {
    auto oldIt{*this};
    this->decrement();
    return oldIt;
}

template<typename GraphType>
class const_adjacent_nodes_iterator {
public:
    using value_type = typename GraphType::value_type;
    using difference_type = ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;
    using pointer = const value_type*;
    using reference = const value_type&;
    using iterator_type = std::set<size_t>::const_iterator;

    // Bidirectional iterators need to provide a default constructor
    const_adjacent_nodes_iterator() = default;

    // No transfer of ownership
    const_adjacent_nodes_iterator(iterator_type it, const GraphType* graph);

    reference operator*() const;

    // Return type must be something that -> can be applied to, so, a pointer
    pointer operator->() const;

    const_adjacent_nodes_iterator& operator++();
    const_adjacent_nodes_iterator operator++(int);

    const_adjacent_nodes_iterator& operator--();
    const_adjacent_nodes_iterator operator--(int);

    bool operator==(const const_adjacent_nodes_iterator& rhs) const;
    bool operator!=(const const_adjacent_nodes_iterator& rhs) const;

protected:
    iterator_type m_adjacentNodeIterator;
    const GraphType* m_graph{nullptr};

    void increment();
    void decrement();
};

template<typename GraphType>
const_adjacent_nodes_iterator<GraphType>::const_adjacent_nodes_iterator(
    iterator_type it, const GraphType* graph)
    : m_adjacentNodeIterator{it}, m_graph{graph} {}

// Return a reference to the node.
template<typename GraphType>
typename const_adjacent_nodes_iterator<GraphType>::reference
const_adjacent_nodes_iterator<GraphType>::operator*() const {
    // Return an reference to the actual node, not the index to the node.
    return (*m_graph)[*m_adjacentNodeIterator];
}

template<typename GraphType>
typename const_adjacent_nodes_iterator<GraphType>::pointer
const_adjacent_nodes_iterator<GraphType>::operator->() const {
    return &((*m_graph)[*m_adjacentNodeIterator]);
}

template<typename GraphType>
const_adjacent_nodes_iterator<GraphType>&
const_adjacent_nodes_iterator<GraphType>::operator++() {
    increment();
    return *this;
}

template<typename GraphType>
const_adjacent_nodes_iterator<GraphType>
const_adjacent_nodes_iterator<GraphType>::operator++(int) {
    auto oldIt{*this};
    increment();
    return oldIt;
}

template<typename GraphType>
const_adjacent_nodes_iterator<GraphType>&
const_adjacent_nodes_iterator<GraphType>::operator--() {
    decrement();
    return *this;
}

template<typename GraphType>
const_adjacent_nodes_iterator<GraphType>
const_adjacent_nodes_iterator<GraphType>::operator--(int) {
    auto oldIt{*this};
    decrement();
    return oldIt;
}

// Undefined behaviour if the iterator is already the past-the-end iterator
template<typename GraphType>
void const_adjacent_nodes_iterator<GraphType>::increment() {
    ++m_adjacentNodeIterator;
}

template<typename GraphType>
void const_adjacent_nodes_iterator<GraphType>::decrement() {
    --m_adjacentNodeIterator;
}

template<typename GraphType>
bool const_adjacent_nodes_iterator<GraphType>::operator==(
    const const_adjacent_nodes_iterator& rhs) const {
    if (!m_graph && !rhs.m_graph) {
        // Both are end iterators
        return true;
    }
    return (m_graph == rhs.m_graph &&
            m_adjacentNodeIterator == rhs.m_adjacentNodeIterator);
}

template<typename GraphType>
bool const_adjacent_nodes_iterator<GraphType>::operator!=(
    const const_adjacent_nodes_iterator& rhs) const {
    return !(*this == rhs);
}

// Subclass the const_adjacent_nodes_iterator to make the non-const
template<typename GraphType>
class adjacent_nodes_iterator
    : public const_adjacent_nodes_iterator<GraphType> {
public:
    using value_type = typename GraphType::value_type;
    using difference_type = ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_type = std::set<size_t>::iterator;

    adjacent_nodes_iterator() = default;

    adjacent_nodes_iterator(iterator_type it, const GraphType* graph);

    reference operator*();
    pointer operator->();

    adjacent_nodes_iterator& operator++();
    adjacent_nodes_iterator operator++(int);
    adjacent_nodes_iterator& operator--();
    adjacent_nodes_iterator operator--(int);
};

template<typename GraphType>
adjacent_nodes_iterator<GraphType>::adjacent_nodes_iterator(
    iterator_type it, const GraphType* graph)
    : const_adjacent_nodes_iterator<GraphType>{it, graph} {}

template<typename GraphType>
typename adjacent_nodes_iterator<GraphType>::reference
adjacent_nodes_iterator<GraphType>::operator*() {
    return const_cast<reference>(
        (*(this->m_graph))[*(this->m_adjacentNodeIterator)]);
}

template<typename GraphType>
typename adjacent_nodes_iterator<GraphType>::pointer
adjacent_nodes_iterator<GraphType>::operator->() {
    return const_cast<pointer>(
        &((*(this->m_graph))[*(this->m_adjacentNodeIterator)]));
}

template<typename GraphType>
adjacent_nodes_iterator<GraphType>&
adjacent_nodes_iterator<GraphType>::operator++() {
    this->increment();
    return *this;
}

template<typename GraphType>
adjacent_nodes_iterator<GraphType>
adjacent_nodes_iterator<GraphType>::operator++(int) {
    auto oldIt{*this};
    this->increment();
    return oldIt;
}

template<typename GraphType>
adjacent_nodes_iterator<GraphType>&
adjacent_nodes_iterator<GraphType>::operator--() {
    this->decrement();
    return *this;
}

template<typename GraphType>
adjacent_nodes_iterator<GraphType>
adjacent_nodes_iterator<GraphType>::operator--(int) {
    auto oldIt{*this};
    this->decrement();
    return oldIt;
}
