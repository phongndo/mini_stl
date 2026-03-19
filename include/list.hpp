#ifndef LIST_HPP
#define LIST_HPP

#include <cstddef>
#include <iterator>
#include <memory>
#include <utility>

template <typename T> class List {
private:
  struct NodeBase {
    NodeBase *prev;
    NodeBase *next;

    NodeBase() : prev(nullptr), next(nullptr) {}
  };

  struct Node : NodeBase {
    T value;

    template <typename U>
    explicit Node(U &&data) : NodeBase(), value(std::forward<U>(data)) {}
  };

public:
  using value_type = T;
  using pointer = T *;
  using const_pointer = const T *;
  using reference = T &;
  using const_reference = const T &;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  class const_iterator;

  class iterator {
  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = List::value_type;
    using difference_type = List::difference_type;
    using pointer = List::pointer;
    using reference = List::reference;

    iterator() : _node(nullptr) {}
    explicit iterator(NodeBase *node) : _node(node) {}

    reference operator*() const { return static_cast<Node *>(_node)->value; }
    pointer operator->() const {
      return std::addressof(static_cast<Node *>(_node)->value);
    }

    iterator &operator++() {
      _node = _node->next;
      return *this;
    }
    iterator operator++(int) {
      iterator tmp(*this);
      ++(*this);
      return tmp;
    }

    iterator &operator--() {
      _node = _node->prev;
      return *this;
    }
    iterator operator--(int) {
      iterator tmp(*this);
      --(*this);
      return tmp;
    }

    bool operator==(const iterator &other) const {
      return _node == other._node;
    }
    bool operator!=(const iterator &other) const {
      return _node != other._node;
    }

  private:
    friend class List;
    friend class const_iterator;

    NodeBase *_node;
  };

  class const_iterator {
  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = List::value_type;
    using difference_type = List::difference_type;
    using pointer = List::const_pointer;
    using reference = List::const_reference;

    const_iterator() : _node(nullptr) {}
    explicit const_iterator(const NodeBase *node) : _node(node) {}
    const_iterator(const iterator &other) : _node(other._node) {}

    reference operator*() const {
      return static_cast<const Node *>(_node)->value;
    }
    pointer operator->() const {
      return std::addressof(static_cast<const Node *>(_node)->value);
    }

    const_iterator &operator++() {
      _node = _node->next;
      return *this;
    }
    const_iterator operator++(int) {
      const_iterator tmp(*this);
      ++(*this);
      return tmp;
    }

    const_iterator &operator--() {
      _node = _node->prev;
      return *this;
    }
    const_iterator operator--(int) {
      const_iterator tmp(*this);
      --(*this);
      return tmp;
    }

    bool operator==(const const_iterator &other) const {
      return _node == other._node;
    }
    bool operator!=(const const_iterator &other) const {
      return _node != other._node;
    }

  private:
    friend class List;
    friend class iterator;

    const NodeBase *_node;
  };

  List() { reset_root(); }

  List(const List &other) {
    reset_root();
    for (const auto &value : other) {
      push_back(value);
    }
  }

  List(List &&other) noexcept {
    reset_root();
    steal_from(other);
  }

  ~List() { clear(); }

  List &operator=(const List &other) {
    if (this != &other) {
      List tmp(other);
      swap(tmp);
    }
    return *this;
  }

  List &operator=(List &&other) noexcept {
    if (this != &other) {
      List tmp(std::move(other));
      swap(tmp);
    }
    return *this;
  }

  iterator begin() { return iterator(_root.next); }
  iterator end() { return iterator(&_root); }
  const_iterator begin() const { return const_iterator(_root.next); }
  const_iterator end() const { return const_iterator(&_root); }
  const_iterator cbegin() const { return const_iterator(_root.next); }
  const_iterator cend() const { return const_iterator(&_root); }

  bool empty() const { return _size == 0; }
  size_type size() const { return _size; }

  reference front() { return static_cast<Node *>(_root.next)->value; }
  const_reference front() const {
    return static_cast<const Node *>(_root.next)->value;
  }

  reference back() { return static_cast<Node *>(_root.prev)->value; }
  const_reference back() const {
    return static_cast<const Node *>(_root.prev)->value;
  }

  iterator insert(const_iterator position, const T &value) {
    return insert_value(position, value);
  }

  iterator insert(const_iterator position, T &&value) {
    return insert_value(position, std::move(value));
  }

  iterator erase(const_iterator position) {
    if (position._node == &_root) {
      return end();
    }

    NodeBase *node = const_cast<NodeBase *>(position._node);
    iterator next(node->next);
    unlink(node);
    delete static_cast<Node *>(node);
    --_size;
    return next;
  }

  void push_front(const T &value) { insert(cbegin(), value); }
  void push_front(T &&value) { insert(cbegin(), std::move(value)); }

  void push_back(const T &value) { insert(cend(), value); }
  void push_back(T &&value) { insert(cend(), std::move(value)); }

  void pop_front() {
    if (!empty()) {
      erase(cbegin());
    }
  }

  void pop_back() {
    if (!empty()) {
      erase(const_iterator(_root.prev));
    }
  }

  void clear() {
    NodeBase *node = _root.next;
    while (node != &_root) {
      NodeBase *next = node->next;
      delete static_cast<Node *>(node);
      node = next;
    }
    reset_root();
    _size = 0;
  }

  void swap(List &other) noexcept {
    if (this == &other) {
      return;
    }

    List tmp;
    tmp.steal_from(*this);
    steal_from(other);
    other.steal_from(tmp);
  }

  friend void swap(List &lhs, List &rhs) noexcept { lhs.swap(rhs); }

private:
  template <typename U> iterator insert_value(const_iterator position, U &&value) {
    Node *node = new Node(std::forward<U>(value));
    link_before(const_cast<NodeBase *>(position._node), node);
    ++_size;
    return iterator(node);
  }

  void reset_root() {
    _root.next = &_root;
    _root.prev = &_root;
  }

  void link_before(NodeBase *position, NodeBase *node) {
    node->next = position;
    node->prev = position->prev;
    position->prev->next = node;
    position->prev = node;
  }

  void unlink(NodeBase *node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
  }

  void steal_from(List &other) noexcept {
    if (other._size == 0) {
      reset_root();
      _size = 0;
      return;
    }

    _root.next = other._root.next;
    _root.prev = other._root.prev;
    _root.next->prev = &_root;
    _root.prev->next = &_root;
    _size = other._size;

    other.reset_root();
    other._size = 0;
  }

  NodeBase _root;
  size_type _size = 0;
};

#endif
