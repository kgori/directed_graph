//
// Created by Kevin Gori on 24/11/2023.
//
#pragma once

#include <algorithm>
#include <format>
#include <iterator>
#include <set>
#include <sstream>
#include <vector>

template<typename T, typename A>
class weighted_directed_graph;

template<typename GraphType>
class const_graph_iterator;

template<typename GraphType>
class graph_iterator;

template<typename GraphType>
class const_adjacent_weighted_nodes_iterator;

template<typename GraphType>
class adjacent_weighted_nodes_iterator;

namespace details {
    // Forward declaration
    class graph_edge;

    // Putting the allocation logic in a separate base class makes the
    // graph node constructor exception safe. The naive version could
    // throw between allocating memory and constructing a T{}, which results
    // in a leak because the destructor isn't called.
    template<typename T, typename A = std::allocator<T>>
    class weighted_graph_node_allocator {
    protected:
        explicit weighted_graph_node_allocator(const A& allocator);

        // Copy and move constructors
        weighted_graph_node_allocator(const weighted_graph_node_allocator&) =
            delete;
        weighted_graph_node_allocator(
            weighted_graph_node_allocator&& src) noexcept;

        // Copy and move assignment operators
        weighted_graph_node_allocator&
        operator=(const weighted_graph_node_allocator&) = delete;
        weighted_graph_node_allocator&
        operator=(weighted_graph_node_allocator&&) noexcept = delete;

        ~weighted_graph_node_allocator();

        A m_allocator;
        T* m_data{ nullptr };
    };

    template<typename T, typename A>
    weighted_graph_node_allocator<T, A>::weighted_graph_node_allocator(
        const A& allocator)
        : m_allocator{ allocator } {
        m_data = m_allocator.allocate(1);
    }

    template<typename T, typename A>
    weighted_graph_node_allocator<T, A>::weighted_graph_node_allocator(
        weighted_graph_node_allocator&& src) noexcept
        : m_allocator{ std::move(src.m_allocator) },
          m_data{ std::exchange(src.m_data, nullptr) } {}

    template<typename T, typename A>
    weighted_graph_node_allocator<T, A>::~weighted_graph_node_allocator() {
        m_allocator.deallocate(m_data, 1);
        m_data = nullptr;
    }

    template<typename T, typename A = std::allocator<T>>
    class weighted_graph_node : private weighted_graph_node_allocator<T, A> {
    public:
        // Constructors
        weighted_graph_node(weighted_directed_graph<T, A>& graph, const T& t);

        weighted_graph_node(weighted_directed_graph<T, A>& graph, T&& t);

        weighted_graph_node(weighted_directed_graph<T, A>& graph, const T& t,
                            const A& allocator);

        weighted_graph_node(weighted_directed_graph<T, A>& graph, T&& t,
                            const A& allocator);

        ~weighted_graph_node();

        // Copy and move constructors
        weighted_graph_node(const weighted_graph_node& src);

        weighted_graph_node(weighted_graph_node&& src) noexcept;

        // Copy and move assignment
        weighted_graph_node& operator=(const weighted_graph_node& rhs);

        weighted_graph_node& operator=(weighted_graph_node&& rhs) noexcept;

        // Getters
        [[nodiscard]] T& value() noexcept;

        [[nodiscard]] const T& value() const noexcept;

        bool operator==(const weighted_graph_node& rhs) const;
        bool operator!=(const weighted_graph_node& rhs) const;

    private:
        friend class weighted_directed_graph<T, A>;

        // A reference to the graph this node belongs to
        weighted_directed_graph<T, A>& m_graph;

        // Type alias for the container type used to store nodes
        using adjacency_list_type = std::set<details::graph_edge>;

        // A ref to the adjacency list
        [[nodiscard]] adjacency_list_type& get_adjacent_nodes_indices();

        [[nodiscard]] const adjacency_list_type&
        get_adjacent_nodes_indices() const;

        adjacency_list_type m_adjacentNodeIndices;
    };

    template<typename T, typename A>
    weighted_graph_node<T, A>::weighted_graph_node(
        weighted_directed_graph<T, A>& graph, const T& t, const A& allocator)
        : m_graph{ graph }, weighted_graph_node_allocator<T, A>{ allocator } {
        new (this->m_data) T{ t };// Placement new
    }

    template<typename T, typename A>
    weighted_graph_node<T, A>::weighted_graph_node(
        weighted_directed_graph<T, A>& graph, T&& t, const A& allocator)
        : m_graph{ graph }, weighted_graph_node_allocator<T, A>{ allocator } {
        new (this->m_data) T{ std::move(t) };
    }

    template<typename T, typename A>
    weighted_graph_node<T, A>::weighted_graph_node(
        weighted_directed_graph<T, A>& graph, const T& t)
        : weighted_graph_node<T, A>{ graph, t, A{} } {}

    template<typename T, typename A>
    weighted_graph_node<T, A>::weighted_graph_node(
        weighted_directed_graph<T, A>& graph, T&& t)
        : weighted_graph_node<T, A>{ graph, std::move(t), A{} } {}

    // destructor required - memory management gets a little hairy now allocators are involved
    template<typename T, typename A>
    weighted_graph_node<T, A>::~weighted_graph_node() {
        if (this->m_data) { this->m_data->~T(); }
    }

    template<typename T, typename A>
    weighted_graph_node<T, A>::weighted_graph_node(
        const weighted_graph_node& src)
        : weighted_graph_node_allocator<T, A>{ src.m_allocator },
          m_graph{ src.m_graph },
          m_adjacentNodeIndices{ src.m_adjacentNodeIndices } {
        new (this->m_data) T{ *(src.m_data) };
    }

    template<typename T, typename A>
    weighted_graph_node<T, A>::weighted_graph_node(
        weighted_graph_node&& src) noexcept
        : weighted_graph_node_allocator<T, A>{ std::move(src) },
          m_graph{ src.m_graph },
          m_adjacentNodeIndices{ std::move(src.m_adjacentNodeIndices) } {}

    template<typename T, typename A>
    weighted_graph_node<T, A>&
    weighted_graph_node<T, A>::operator=(const weighted_graph_node& rhs) {
        if (this != &rhs) {
            m_graph = rhs.m_graph;
            m_adjacentNodeIndices = rhs.m_adjacentNodeIndices;
            new (this->m_data) T{ *(rhs.m_data) };
        }
        return *this;
    }

    template<typename T, typename A>
    weighted_graph_node<T, A>&
    weighted_graph_node<T, A>::operator=(weighted_graph_node&& rhs) noexcept {
        m_graph = rhs.m_graph;
        m_adjacentNodeIndices = std::move(rhs.m_adjacentNodeIndices);
        this->m_data = std::exchange(rhs.m_data, nullptr);
        return *this;
    }

    template<typename T, typename A>
    T& weighted_graph_node<T, A>::value() noexcept {
        return *(this->m_data);
    }

    template<typename T, typename A>
    const T& weighted_graph_node<T, A>::value() const noexcept {
        return *(this->m_data);
    };

    template<typename T, typename A>
    bool weighted_graph_node<T, A>::operator==(
        const weighted_graph_node& rhs) const {
        return &m_graph == &rhs.m_graph && *(this->m_data) == *(rhs.m_data) &&
               m_adjacentNodeIndices == rhs.m_adjacentNodeIndices;
    }

    template<typename T, typename A>
    bool weighted_graph_node<T, A>::operator!=(
        const weighted_graph_node& rhs) const {
        return !(*this == rhs);
    }

    template<typename T, typename A>
    typename weighted_graph_node<T, A>::adjacency_list_type&
    weighted_graph_node<T, A>::get_adjacent_nodes_indices() {
        return m_adjacentNodeIndices;
    }

    template<typename T, typename A>
    const typename weighted_graph_node<T, A>::adjacency_list_type&
    weighted_graph_node<T, A>::get_adjacent_nodes_indices() const {
        return m_adjacentNodeIndices;
    }

    class graph_edge {
    public:
        graph_edge(std::size_t to, double weight)
            : m_to{ to }, m_weight{ weight } {}

        auto operator<=>(const graph_edge&) const = default;

        std::size_t index() const { return m_to; }

        double weight() const { return m_weight; }

        void decrement() { m_to--; }

    private:
        std::size_t m_to;
        double m_weight;
    };
}// namespace details

template<typename T, typename A = std::allocator<T>>
class weighted_directed_graph {
public:
    // Necessary type aliases for weighted_directed_graph to function as an STL container
    using value_type = T;
    using allocator_type = A;
    using reference = value_type&;
    using const_reference = const value_type&;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using pair_allocator = typename std::allocator_traits<
        A>::template rebind_alloc<std::pair<T, double>>;

    // Constructors
    // Let the compiler default the default constructor, but make it noexcept
    // iff the allocator's default constructor is also noexcept
    weighted_directed_graph() noexcept(noexcept(A{})) = default;
    explicit weighted_directed_graph(const A& allocator) noexcept;

    // Aliases required for iterator support - both are const_ on purpose,
    // to be like std::set in disallowing modification of elements (and
    // the implementation of the directed graph class uss std::set internally)
    using iterator = const_graph_iterator<weighted_directed_graph>;
    using const_iterator = const_graph_iterator<weighted_directed_graph>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using iterator_adjacent_nodes =
        adjacent_weighted_nodes_iterator<weighted_directed_graph>;
    using const_iterator_adjacent_nodes =
        const_adjacent_weighted_nodes_iterator<weighted_directed_graph>;
    using reverse_iterator_adjacent_nodes =
        std::reverse_iterator<iterator_adjacent_nodes>;
    using const_reverse_iterator_adjacent_nodes =
        std::reverse_iterator<const_iterator_adjacent_nodes>;

    // debug aliases
    using public_node_type = details::weighted_graph_node<T, A>;
    using public_nodes_container_type =
        std::vector<details::weighted_graph_node<T, A>>;

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
    bool insert_edge(const T& from_node_value, const T& to_node_value,
                     double weight);

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
    bool operator==(const weighted_directed_graph& rhs) const;

    bool operator!=(const weighted_directed_graph& rhs) const;

    // Swaps all nodes between this graph and other_graph
    void swap(weighted_directed_graph& other_graph) noexcept;

    [[nodiscard]] size_type size() const noexcept;

    [[nodiscard]] size_type max_size() const noexcept;

    [[nodiscard]] bool empty() const noexcept;

    // Returns a set with the values of the nodes connected to the node with
    // node_value
    [[nodiscard]] std::set<T, std::less<>, A>
    get_adjacent_nodes_values(const T& node_value) const;

    [[nodiscard]] std::set<std::pair<T, double>, std::less<>, pair_allocator>
    get_adjacent_nodes_values_and_weights(const T& node_value) const;

private:
    friend class details::weighted_graph_node<T, A>;
    friend class const_graph_iterator<weighted_directed_graph>;
    friend class graph_iterator<weighted_directed_graph>;

    using nodes_container_type =
        std::vector<details::weighted_graph_node<T, A>>;

    nodes_container_type m_nodes;
    A m_allocator;

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

    [[nodiscard]] std::set<T, std::less<>, A> get_adjacent_nodes_values(
        const typename details::weighted_graph_node<T, A>::adjacency_list_type&
            indices) const;

    [[nodiscard]] std::set<std::pair<T, double>, std::less<>, pair_allocator>
    get_adjacent_nodes_values_and_weights(
        const typename details::weighted_graph_node<T, A>::adjacency_list_type&
            indices) const;
};

// Stand-alone swap uses swap() method internally. I think this is provided
// to better handle some ADL edge cases?
template<typename T, typename A>
void swap(weighted_directed_graph<T, A>& first,
          weighted_directed_graph<T, A>& second) noexcept {
    first.swap(second);
}

template<typename T, typename A>
weighted_directed_graph<T, A>::weighted_directed_graph(
    const A& allocator) noexcept
    : m_nodes{ allocator }, m_allocator{ allocator } {}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::nodes_container_type::iterator
weighted_directed_graph<T, A>::findNode(const T& node_value) {
    return std::find_if(
        std::begin(m_nodes), std::end(m_nodes),
        [&node_value](const auto& node) { return node.value() == node_value; });
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::nodes_container_type::const_iterator
weighted_directed_graph<T, A>::findNode(const T& node_value) const {
    return const_cast<weighted_directed_graph<T, A>*>(this)->findNode(
        node_value);
}

template<typename T, typename A>
std::pair<typename weighted_directed_graph<T, A>::iterator, bool>
weighted_directed_graph<T, A>::insert(T&& node_value) {
    auto iter{ findNode(node_value) };
    if (iter != std::end(m_nodes)) {
        // Value is already in the graph
        return { iterator{ iter, this }, false };
    }
    m_nodes.emplace_back(*this, std::move(node_value), m_allocator);
    return { iterator{ --std::end(m_nodes), this }, true };
}

template<typename T, typename A>
std::pair<typename weighted_directed_graph<T, A>::iterator, bool>
weighted_directed_graph<T, A>::insert(const T& node_value) {
    T copy{ node_value };
    return insert(std::move(copy));
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::iterator
weighted_directed_graph<T, A>::insert(const_iterator hint, T&& node_value) {
    // Ignore the hint, just forward to standard insert.
    return insert(node_value).first;
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::iterator
weighted_directed_graph<T, A>::insert(const_iterator hint,
                                      const T& node_value) {
    return insert(node_value).first;
}

// Nested templates - can't use template<typename T, typename Iter>
template<typename T, typename A>
template<typename Iter>
void weighted_directed_graph<T, A>::insert(Iter first, Iter last) {
    // Copy each element in the range by using an insert_iterator
    std::copy(first, last, std::insert_iterator{ *this, begin() });
}

template<typename T, typename A>
bool weighted_directed_graph<T, A>::insert_edge(const T& from_node_value,
                                                const T& to_node_value,
                                                double weight) {
    const auto from{ findNode(from_node_value) };
    const auto to{ findNode(to_node_value) };
    if (from == std::end(m_nodes) || to == std::end(m_nodes)) { return false; }
    const size_t to_index{ get_index_of_node(to) };
    return from->get_adjacent_nodes_indices()
        .insert({ to_index, weight })
        .second;
}

template<typename T, typename A>
size_t weighted_directed_graph<T, A>::get_index_of_node(
    const typename nodes_container_type::const_iterator& node) const noexcept {
    const auto index{ std::distance(std::cbegin(m_nodes), node) };
    return static_cast<size_t>(index);
}

template<typename T, typename A>
void weighted_directed_graph<T, A>::remove_all_links_to(
    typename nodes_container_type::const_iterator node_iter) {
    const size_t node_index{ get_index_of_node(node_iter) };

    // Iterate over all adjacency lists of all nodes
    for (auto&& node: m_nodes) {
        auto& adjacencyIndices{ node.get_adjacent_nodes_indices() };
        // Remove references from to-be-deleted node
        std::erase_if(adjacencyIndices, [node_index](auto const& e) {
            return e.index() == node_index;
        });
        //adjacencyIndices.erase(node_index);
        // Modify adjacency indices to account for deletion
        // Some inefficiency, as data is converted to a vector,
        // indices are adjusted, and the set is wiped and rebuilt
        // from the vector (values in sets are immutable).
        std::vector<details::graph_edge> edges(std::begin(adjacencyIndices),
                                               std::end(adjacencyIndices));
        std::for_each(std::begin(edges), std::end(edges),
                      [node_index](auto& e) {
                          if (e.index() > node_index) { e.decrement(); }
                      });
        adjacencyIndices.clear();
        adjacencyIndices.insert(std::begin(edges), std::end(edges));
    }
}

template<typename T, typename A>
bool weighted_directed_graph<T, A>::erase(const T& node_value) {
    auto iter{ findNode(node_value) };
    if (iter == std::end(m_nodes)) { return false; }
    remove_all_links_to(iter);
    m_nodes.erase(iter);
    return true;
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::iterator
weighted_directed_graph<T, A>::erase(const_iterator pos) {
    if (pos.m_nodeIterator == std::end(m_nodes)) {
        return iterator{ std::end(m_nodes), this };
    }
    remove_all_links_to(pos.m_nodeIterator);
    return iterator{ m_nodes.erase(pos.m_nodeIterator), this };
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::iterator
weighted_directed_graph<T, A>::erase(const_iterator first,
                                     const_iterator last) {
    for (auto iter{ first }; iter != last; ++iter) {
        if (iter.m_nodeIterator != std::end(m_nodes)) {
            remove_all_links_to(iter.m_nodeIterator);
        }
    }
    return iterator{ m_nodes.erase(first.m_nodeIterator, last.m_nodeIterator),
                     this };
}

template<typename T, typename A>
bool weighted_directed_graph<T, A>::erase_edge(const T& from_node_value,
                                               const T& to_node_value) {
    const auto from{ findNode(from_node_value) };
    const auto to{ findNode(to_node_value) };
    if (from == std::end(m_nodes) || to == std::end(m_nodes)) { return false; }
    const size_t to_index{ get_index_of_node(to) };
    std::erase_if(from->get_adjacent_nodes_indices(),
                  [to_index](auto const& e) { return e.index() == to_index; });
    return true;
}

template<typename T, typename A>
void weighted_directed_graph<T, A>::clear() noexcept {
    m_nodes.clear();
}

template<typename T, typename A>
void weighted_directed_graph<T, A>::swap(
    weighted_directed_graph& other_graph) noexcept {
    using std::swap;
    m_nodes.swap(other_graph.m_nodes);
    swap(m_allocator, other_graph.m_allocator);
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::reference
weighted_directed_graph<T, A>::operator[](size_t index) {
    return m_nodes[index].value();
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::const_reference
weighted_directed_graph<T, A>::operator[](size_type index) const {
    return m_nodes[index].value();
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::reference
weighted_directed_graph<T, A>::at(size_type index) {
    return m_nodes.at(index).value();
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::const_reference
weighted_directed_graph<T, A>::at(
    weighted_directed_graph::size_type index) const {
    return m_nodes.at(index).value();
}

template<typename T, typename A>
bool weighted_directed_graph<T, A>::operator==(
    const weighted_directed_graph& rhs) const {
    if (m_nodes.size() != rhs.m_nodes.size()) { return false; }

    for (auto&& node: m_nodes) {
        const auto rhsNodeIter{ rhs.findNode(node.value()) };
        if (rhsNodeIter == std::end(rhs.m_nodes)) { return false; }
        const auto adjacent_values_lhs{ get_adjacent_nodes_values(
            node.get_adjacent_nodes_indices()) };
        const auto adjacent_values_rhs{ rhs.get_adjacent_nodes_values(
            rhsNodeIter->get_adjacent_nodes_indices()) };
        if (adjacent_values_lhs != adjacent_values_rhs) { return false; }
    }
    return true;
}

template<typename T, typename A>
std::set<T, std::less<>, A>
weighted_directed_graph<T, A>::get_adjacent_nodes_values(
    const typename details::weighted_graph_node<T, A>::adjacency_list_type&
        indices) const {
    std::set<T, std::less<>, A> values(m_allocator);
    for (auto&& edge: indices) { values.insert(m_nodes[edge.index()].value()); }
    return values;
}

template<typename T, typename A>
std::set<std::pair<T, double>, std::less<>,
         typename std::allocator_traits<A>::template rebind_alloc<
             std::pair<T, double>>>
weighted_directed_graph<T, A>::get_adjacent_nodes_values_and_weights(
    const typename details::weighted_graph_node<T, A>::adjacency_list_type&
        indices) const {
    using PairAllocator = typename std::allocator_traits<
        A>::template rebind_alloc<std::pair<T, double>>;
    std::set<std::pair<T, double>, std::less<>, PairAllocator> values(
        m_allocator);
    for (auto&& edge: indices) {
        T value = m_nodes[edge.index()].value();
        double weight = edge.weight();
        values.insert({ value, weight });
    }
    return values;
}

template<typename T, typename A>
bool weighted_directed_graph<T, A>::operator!=(
    const weighted_directed_graph& rhs) const {
    return !(*this == rhs);
}

template<typename T, typename A>
std::set<T, std::less<>, A>
weighted_directed_graph<T, A>::get_adjacent_nodes_values(
    const T& node_value) const {
    auto iter{ findNode(node_value) };
    if (iter == std::end(m_nodes)) {
        return std::set<T, std::less<>, A>{ m_allocator };
    }
    return get_adjacent_nodes_values(iter->get_adjacent_nodes_indices());
}

template<typename T, typename A>
std::set<std::pair<T, double>, std::less<>,
         typename std::allocator_traits<A>::template rebind_alloc<
             std::pair<T, double>>>
weighted_directed_graph<T, A>::get_adjacent_nodes_values_and_weights(
    const T& node_value) const {
    using PairAllocator = typename std::allocator_traits<
        A>::template rebind_alloc<std::pair<T, double>>;
    auto iter{ findNode(node_value) };
    if (iter == std::end(m_nodes)) {
        return std::set<std::pair<T, double>, std::less<>, PairAllocator>{
            m_allocator
        };
    }
    return get_adjacent_nodes_values_and_weights(
        iter->get_adjacent_nodes_indices());
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::size_type
weighted_directed_graph<T, A>::size() const noexcept {
    return m_nodes.size();
}

template<typename T, typename A>
bool weighted_directed_graph<T, A>::empty() const noexcept {
    return m_nodes.empty();
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::size_type
weighted_directed_graph<T, A>::max_size() const noexcept {
    return m_nodes.max_size();
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::iterator_adjacent_nodes
weighted_directed_graph<T, A>::begin(const T& node_value) noexcept {
    auto iter{ findNode(node_value) };
    if (iter == std::end(m_nodes)) {
        // Default-construct an end iterator, and return
        return iterator_adjacent_nodes{};
    }
    return iterator_adjacent_nodes{
        std::begin(iter->get_adjacent_nodes_indices()), this
    };
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::iterator_adjacent_nodes
weighted_directed_graph<T, A>::end(const T& node_value) noexcept {
    auto iter{ findNode(node_value) };
    if (iter == std::end(m_nodes)) { return iterator_adjacent_nodes{}; }
    return iterator_adjacent_nodes{
        std::end(iter->get_adjacent_nodes_indices()), this
    };
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::const_iterator_adjacent_nodes
weighted_directed_graph<T, A>::cbegin(const T& node_value) const noexcept {
    auto iter{ findNode(node_value) };
    if (iter == std::end(m_nodes)) {
        // Default-construct an end iterator, and return
        return const_iterator_adjacent_nodes{};
    }
    return const_iterator_adjacent_nodes{
        std::begin(iter->get_adjacent_nodes_indices()), this
    };
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::const_iterator_adjacent_nodes
weighted_directed_graph<T, A>::cend(const T& node_value) const noexcept {
    auto iter{ findNode(node_value) };
    if (iter == std::end(m_nodes)) { return const_iterator_adjacent_nodes{}; }
    return const_iterator_adjacent_nodes{
        std::end(iter->get_adjacent_nodes_indices()), this
    };
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::const_iterator_adjacent_nodes
weighted_directed_graph<T, A>::begin(const T& node_value) const noexcept {
    return cbegin(node_value);
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::const_iterator_adjacent_nodes
weighted_directed_graph<T, A>::end(const T& node_value) const noexcept {
    return cend(node_value);
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::reverse_iterator_adjacent_nodes
weighted_directed_graph<T, A>::rbegin(const T& node_value) noexcept {
    return reverse_iterator_adjacent_nodes{ end(node_value) };
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::reverse_iterator_adjacent_nodes
weighted_directed_graph<T, A>::rend(const T& node_value) noexcept {
    return reverse_iterator_adjacent_nodes{ begin(node_value) };
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::const_reverse_iterator_adjacent_nodes
weighted_directed_graph<T, A>::rbegin(const T& node_value) const noexcept {
    return const_reverse_iterator_adjacent_nodes{ end(node_value) };
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::const_reverse_iterator_adjacent_nodes
weighted_directed_graph<T, A>::rend(const T& node_value) const noexcept {
    return const_reverse_iterator_adjacent_nodes{ begin(node_value) };
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::const_reverse_iterator_adjacent_nodes
weighted_directed_graph<T, A>::crbegin(const T& node_value) const noexcept {
    return rbegin(node_value);
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::const_reverse_iterator_adjacent_nodes
weighted_directed_graph<T, A>::crend(const T& node_value) const noexcept {
    return rend(node_value);
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::iterator
weighted_directed_graph<T, A>::begin() noexcept {
    return iterator{ std::begin(m_nodes), this };
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::iterator
weighted_directed_graph<T, A>::end() noexcept {
    return iterator{ std::end(m_nodes), this };
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::const_iterator
weighted_directed_graph<T, A>::begin() const noexcept {
    return const_cast<weighted_directed_graph*>(this)->begin();
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::const_iterator
weighted_directed_graph<T, A>::end() const noexcept {
    return const_cast<weighted_directed_graph*>(this)->end();
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::const_iterator
weighted_directed_graph<T, A>::cbegin() const noexcept {
    return begin();
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::const_iterator
weighted_directed_graph<T, A>::cend() const noexcept {
    return end();
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::reverse_iterator
weighted_directed_graph<T, A>::rbegin() noexcept {
    return reverse_iterator{ end() };
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::reverse_iterator
weighted_directed_graph<T, A>::rend() noexcept {
    return reverse_iterator{ begin() };
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::const_reverse_iterator
weighted_directed_graph<T, A>::rbegin() const noexcept {
    return const_reverse_iterator{ end() };
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::const_reverse_iterator
weighted_directed_graph<T, A>::rend() const noexcept {
    return const_reverse_iterator{ begin() };
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::const_reverse_iterator
weighted_directed_graph<T, A>::crbegin() const noexcept {
    return rbegin();
}

template<typename T, typename A>
typename weighted_directed_graph<T, A>::const_reverse_iterator
weighted_directed_graph<T, A>::crend() const noexcept {
    return rend();
}

template<typename T, typename A>
std::wstring to_dot(const weighted_directed_graph<T, A>& graph,
                    std::wstring_view graph_name) {
    std::wstringstream wss;
    wss << std::format(L"digraph {} {{", graph_name.data()) << std::endl;
    for (auto&& node: graph) {
        const auto b{ graph.cbegin(node) };
        const auto e{ graph.cend(node) };
        if (b == e) {
            wss << "  " << node << std::endl;
        } else {
            for (auto iter{ b }; iter != e; ++iter) {
                auto [a, b] = *iter;
                wss << std::format(L"  {} -> {}:{}", node, a, b) << std::endl;
            }
        }
    }
    wss << "}" << std::endl;
    return wss.str();
}

template<typename GraphType>
class const_graph_iterator {
public:
    using value_type = typename GraphType::value_type;
    using difference_type = ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;
    using pointer = const value_type*;
    using reference = const value_type&;
    using iterator_type =
        typename GraphType::nodes_container_type::const_iterator;

    // This constructor is a necessary feature of a bidirectional iterator,
    // so we can just default it.
    const_graph_iterator() = default;

    const_graph_iterator(iterator_type it, const GraphType* graph);

    reference operator*() const;

    pointer operator->() const;

    const_graph_iterator& operator++();

    const_graph_iterator operator++(int);

    const_graph_iterator& operator--();

    const_graph_iterator operator--(int);

    bool operator==(const const_graph_iterator&) const = default;

protected:
    friend class weighted_directed_graph<value_type>;

    iterator_type m_nodeIterator;
    const GraphType* m_graph{ nullptr };

    // Helper methods for ++ and --
    void increment();

    void decrement();
};

template<typename GraphType>
const_graph_iterator<GraphType>::const_graph_iterator(iterator_type it,
                                                      const GraphType* graph)
    : m_nodeIterator{ it }, m_graph{ graph } {}

template<typename GraphType>
typename const_graph_iterator<GraphType>::reference
const_graph_iterator<GraphType>::operator*() const {
    return m_nodeIterator->value();
}

template<typename GraphType>
typename const_graph_iterator<GraphType>::pointer
const_graph_iterator<GraphType>::operator->() const {
    return &(m_nodeIterator->value());
}

// Defer details to the increment helper
template<typename GraphType>
const_graph_iterator<GraphType>& const_graph_iterator<GraphType>::operator++() {
    increment();
    return *this;
}

template<typename GraphType>
const_graph_iterator<GraphType>
const_graph_iterator<GraphType>::operator++(int) {
    auto oldIt{ *this };
    increment();
    return oldIt;
}

template<typename GraphType>
const_graph_iterator<GraphType>& const_graph_iterator<GraphType>::operator--() {
    decrement();
    return *this;
}

template<typename GraphType>
const_graph_iterator<GraphType>
const_graph_iterator<GraphType>::operator--(int) {
    auto oldIt{ *this };
    decrement();
    return oldIt;
}

template<typename GraphType>
void const_graph_iterator<GraphType>::increment() {
    ++m_nodeIterator;
}

template<typename GraphType>
void const_graph_iterator<GraphType>::decrement() {
    --m_nodeIterator;
}

template<typename GraphType>
class graph_iterator : public const_graph_iterator<GraphType> {
public:
    using value_type = GraphType::value_type;
    using difference_type = ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_type = typename GraphType::nodes_container_type::iterator;

    graph_iterator() = default;

    graph_iterator(iterator_type it, const GraphType* graph);

    reference operator*();

    pointer operator->();

    graph_iterator& operator++();

    graph_iterator operator++(int);

    graph_iterator& operator--();

    graph_iterator operator--(int);
};

template<typename GraphType>
graph_iterator<GraphType>::graph_iterator(graph_iterator::iterator_type it,
                                          const GraphType* graph)
    : const_graph_iterator<GraphType>{ it, graph } {}

template<typename GraphType>
typename graph_iterator<GraphType>::reference
graph_iterator<GraphType>::operator*() {
    return const_cast<reference>(this->m_nodeIterator->value());
}

template<typename GraphType>
typename graph_iterator<GraphType>::pointer
graph_iterator<GraphType>::operator->() {
    return const_cast<pointer>(&(this->m_nodeIterator->value()));
}

template<typename GraphType>
graph_iterator<GraphType>& graph_iterator<GraphType>::operator++() {
    this->increment();
    return *this;
}

template<typename GraphType>
graph_iterator<GraphType> graph_iterator<GraphType>::operator++(int) {
    auto oldIt{ *this };
    this->increment();
    return oldIt;
}

template<typename GraphType>
graph_iterator<GraphType>& graph_iterator<GraphType>::operator--() {
    this->decrement();
    return *this;
}

template<typename GraphType>
graph_iterator<GraphType> graph_iterator<GraphType>::operator--(int) {
    auto oldIt{ *this };
    this->decrement();
    return oldIt;
}

template<typename GraphType>
class const_adjacent_weighted_nodes_iterator {
public:
    struct value_type {
        const typename GraphType::value_type node;
        double weight;
    };

    struct ref_value_type {
        const typename GraphType::value_type& node;
        double weight;
    };

    struct ptr_value_type {
        const typename GraphType::value_type* node;
        double weight;
    };

    using difference_type = ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;
    using pointer = const ptr_value_type;
    using reference = const ref_value_type;
    using iterator_type = std::set<details::graph_edge>::const_iterator;

    // Bidirectional iterators need to provide a default constructor
    const_adjacent_weighted_nodes_iterator() = default;

    // No transfer of ownership
    const_adjacent_weighted_nodes_iterator(iterator_type it,
                                           const GraphType* graph);

    reference operator*() const;

    // Return type must be something that -> can be applied to, so, a pointer
    pointer operator->() const;

    const_adjacent_weighted_nodes_iterator& operator++();
    const_adjacent_weighted_nodes_iterator operator++(int);

    const_adjacent_weighted_nodes_iterator& operator--();
    const_adjacent_weighted_nodes_iterator operator--(int);

    bool operator==(const const_adjacent_weighted_nodes_iterator& rhs) const;
    bool operator!=(const const_adjacent_weighted_nodes_iterator& rhs) const;

protected:
    iterator_type m_adjacentNodeIterator;
    const GraphType* m_graph{ nullptr };

    void increment();
    void decrement();
};

template<typename GraphType>
const_adjacent_weighted_nodes_iterator<
    GraphType>::const_adjacent_weighted_nodes_iterator(iterator_type it,
                                                       const GraphType* graph)
    : m_adjacentNodeIterator{ it }, m_graph{ graph } {}

// Return a reference to the node.
template<typename GraphType>
typename const_adjacent_weighted_nodes_iterator<GraphType>::reference
const_adjacent_weighted_nodes_iterator<GraphType>::operator*() const {
    // Return an reference to the actual node, not the index to the node.
    typename GraphType::const_reference node_ref =
        (*m_graph)[m_adjacentNodeIterator->index()];
    auto weight = m_adjacentNodeIterator->weight();
    return { node_ref, weight };
}

template<typename GraphType>
typename const_adjacent_weighted_nodes_iterator<GraphType>::pointer
const_adjacent_weighted_nodes_iterator<GraphType>::operator->() const {
    auto node_ref = (*m_graph)[m_adjacentNodeIterator->index()];
    auto weight = m_adjacentNodeIterator->weight();
    return { &node_ref, weight };
}

template<typename GraphType>
const_adjacent_weighted_nodes_iterator<GraphType>&
const_adjacent_weighted_nodes_iterator<GraphType>::operator++() {
    increment();
    return *this;
}

template<typename GraphType>
const_adjacent_weighted_nodes_iterator<GraphType>
const_adjacent_weighted_nodes_iterator<GraphType>::operator++(int) {
    auto oldIt{ *this };
    increment();
    return oldIt;
}

template<typename GraphType>
const_adjacent_weighted_nodes_iterator<GraphType>&
const_adjacent_weighted_nodes_iterator<GraphType>::operator--() {
    decrement();
    return *this;
}

template<typename GraphType>
const_adjacent_weighted_nodes_iterator<GraphType>
const_adjacent_weighted_nodes_iterator<GraphType>::operator--(int) {
    auto oldIt{ *this };
    decrement();
    return oldIt;
}

// Undefined behaviour if the iterator is already the past-the-end iterator
template<typename GraphType>
void const_adjacent_weighted_nodes_iterator<GraphType>::increment() {
    ++m_adjacentNodeIterator;
}

template<typename GraphType>
void const_adjacent_weighted_nodes_iterator<GraphType>::decrement() {
    --m_adjacentNodeIterator;
}

template<typename GraphType>
bool const_adjacent_weighted_nodes_iterator<GraphType>::operator==(
    const const_adjacent_weighted_nodes_iterator& rhs) const {
    if (!m_graph && !rhs.m_graph) {
        // Both are end iterators
        return true;
    }
    return (m_graph == rhs.m_graph &&
            m_adjacentNodeIterator == rhs.m_adjacentNodeIterator);
}

template<typename GraphType>
bool const_adjacent_weighted_nodes_iterator<GraphType>::operator!=(
    const const_adjacent_weighted_nodes_iterator& rhs) const {
    return !(*this == rhs);
}

// Subclass the const_adjacent_weighted_nodes_iterator to make the non-const
template<typename GraphType>
class adjacent_weighted_nodes_iterator
    : public const_adjacent_weighted_nodes_iterator<GraphType> {
public:
    using value_type =
        typename const_adjacent_weighted_nodes_iterator<GraphType>::value_type;
    using difference_type = ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_type = std::set<details::graph_edge>::iterator;

    adjacent_weighted_nodes_iterator() = default;

    adjacent_weighted_nodes_iterator(iterator_type it, const GraphType* graph);

    reference operator*();
    pointer operator->();

    adjacent_weighted_nodes_iterator& operator++();
    adjacent_weighted_nodes_iterator operator++(int);
    adjacent_weighted_nodes_iterator& operator--();
    adjacent_weighted_nodes_iterator operator--(int);
};

template<typename GraphType>
adjacent_weighted_nodes_iterator<GraphType>::adjacent_weighted_nodes_iterator(
    iterator_type it, const GraphType* graph)
    : const_adjacent_weighted_nodes_iterator<GraphType>{ it, graph } {}

template<typename GraphType>
typename adjacent_weighted_nodes_iterator<GraphType>::reference
adjacent_weighted_nodes_iterator<GraphType>::operator*() {
    return const_cast<reference>(
        this->const_adjacent_weighted_nodes_iterator<GraphType>::operator*());
}

template<typename GraphType>
typename adjacent_weighted_nodes_iterator<GraphType>::pointer
adjacent_weighted_nodes_iterator<GraphType>::operator->() {
    return const_cast<pointer>(
        this->const_adjacent_weighted_nodes_iterator<GraphType>::operator->());
}

template<typename GraphType>
adjacent_weighted_nodes_iterator<GraphType>&
adjacent_weighted_nodes_iterator<GraphType>::operator++() {
    this->increment();
    return *this;
}

template<typename GraphType>
adjacent_weighted_nodes_iterator<GraphType>
adjacent_weighted_nodes_iterator<GraphType>::operator++(int) {
    auto oldIt{ *this };
    this->increment();
    return oldIt;
}

template<typename GraphType>
adjacent_weighted_nodes_iterator<GraphType>&
adjacent_weighted_nodes_iterator<GraphType>::operator--() {
    this->decrement();
    return *this;
}

template<typename GraphType>
adjacent_weighted_nodes_iterator<GraphType>
adjacent_weighted_nodes_iterator<GraphType>::operator--(int) {
    auto oldIt{ *this };
    this->decrement();
    return oldIt;
}
