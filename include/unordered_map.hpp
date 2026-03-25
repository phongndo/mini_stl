#ifndef UNORDERED_MAP_HPP
#define UNORDERED_MAP_HPP

#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <utility>

template <typename Key, typename T, typename Hash = std::hash<Key>,
          typename KeyEqual = std::equal_to<Key>>
class UnorderedMap {
public:
  using key_type = Key;
  using mapped_type = T;
  using value_type = std::pair<const Key, T>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using hasher = Hash;
  using key_equal = KeyEqual;
  using reference = value_type &;
  using const_reference = const value_type &;
  using pointer = value_type *;
  using const_pointer = const value_type *;

private:
  struct Node {
    value_type value;
    Node *next;

    template <typename KeyArg, typename MappedArg>
    Node(KeyArg &&key, MappedArg &&mapped)
        : value(std::forward<KeyArg>(key), std::forward<MappedArg>(mapped)),
          next(nullptr) {}
  };

public:
  class const_iterator;

  class iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = UnorderedMap::value_type;
    using difference_type = UnorderedMap::difference_type;
    using pointer = UnorderedMap::pointer;
    using reference = UnorderedMap::reference;

    iterator() : _map(nullptr), _node(nullptr) {}

    reference operator*() const { return _node->value; }
    pointer operator->() const { return std::addressof(_node->value); }

    iterator &operator++() {
      advance();
      return *this;
    }
    iterator operator++(int) {
      iterator tmp(*this);
      advance();
      return tmp;
    }

    bool operator==(const iterator &other) const {
      return _map == other._map && _node == other._node;
    }
    bool operator!=(const iterator &other) const { return !(*this == other); }

  private:
    friend class UnorderedMap;
    friend class const_iterator;

    iterator(UnorderedMap *map, Node *node) : _map(map), _node(node) {}

    void advance() {
      if (_map == nullptr) {
        return;
      }
      _node = _map->next_node(_node);
    }

    UnorderedMap *_map;
    Node *_node;
  };

  class const_iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = UnorderedMap::value_type;
    using difference_type = UnorderedMap::difference_type;
    using pointer = UnorderedMap::const_pointer;
    using reference = UnorderedMap::const_reference;

    const_iterator() : _map(nullptr), _node(nullptr) {}
    const_iterator(const iterator &other) : _map(other._map), _node(other._node) {}

    reference operator*() const { return _node->value; }
    pointer operator->() const { return std::addressof(_node->value); }

    const_iterator &operator++() {
      advance();
      return *this;
    }
    const_iterator operator++(int) {
      const_iterator tmp(*this);
      advance();
      return tmp;
    }

    bool operator==(const const_iterator &other) const {
      return _map == other._map && _node == other._node;
    }
    bool operator!=(const const_iterator &other) const {
      return !(*this == other);
    }

  private:
    friend class UnorderedMap;
    friend class iterator;

    const_iterator(const UnorderedMap *map, const Node *node)
        : _map(map), _node(node) {}

    void advance() {
      if (_map == nullptr) {
        return;
      }
      _node = _map->next_node(_node);
    }

    const UnorderedMap *_map;
    const Node *_node;
  };

  UnorderedMap()
      : _buckets(allocate_bucket_array(kDefaultBucketCount)),
        _bucket_count(kDefaultBucketCount), _size(0), _hash(Hash()),
        _equal(KeyEqual()) {}

  explicit UnorderedMap(size_type bucket_count, const Hash &hash = Hash(),
                        const KeyEqual &equal = KeyEqual())
      : _buckets(allocate_bucket_array(normalize_bucket_count(bucket_count))),
        _bucket_count(normalize_bucket_count(bucket_count)), _size(0),
        _hash(hash), _equal(equal) {}

  UnorderedMap(const UnorderedMap &other)
      : UnorderedMap(other._bucket_count == 0 ? kDefaultBucketCount
                                              : other._bucket_count,
                     other._hash, other._equal) {
    for (const auto &entry : other) {
      insert(entry);
    }
  }

  UnorderedMap(UnorderedMap &&other)
      : _buckets(other._buckets), _bucket_count(other._bucket_count),
        _size(other._size), _hash(std::move(other._hash)),
        _equal(std::move(other._equal)) {
    other._buckets = nullptr;
    other._bucket_count = 0;
    other._size = 0;
  }

  ~UnorderedMap() {
    clear();
    delete[] _buckets;
  }

  UnorderedMap &operator=(UnorderedMap other) {
    swap(other);
    return *this;
  }

  iterator begin() { return iterator(this, first_node()); }
  iterator end() { return iterator(this, nullptr); }
  const_iterator begin() const { return cbegin(); }
  const_iterator end() const { return cend(); }
  const_iterator cbegin() const { return const_iterator(this, first_node()); }
  const_iterator cend() const { return const_iterator(this, nullptr); }

  bool empty() const { return _size == 0; }
  size_type size() const { return _size; }
  size_type bucket_count() const { return _bucket_count; }
  float load_factor() const {
    if (_bucket_count == 0) {
      return 0.0f;
    }
    return static_cast<float>(_size) / static_cast<float>(_bucket_count);
  }

  mapped_type &operator[](const key_type &key) {
    return insert_value(key, mapped_type()).first->second;
  }

  mapped_type &operator[](key_type &&key) {
    return insert_value(std::move(key), mapped_type()).first->second;
  }

  mapped_type &at(const key_type &key) {
    iterator it = find(key);
    if (it == end()) {
      throw std::out_of_range("Key not found");
    }
    return it->second;
  }

  const mapped_type &at(const key_type &key) const {
    const_iterator it = find(key);
    if (it == cend()) {
      throw std::out_of_range("Key not found");
    }
    return it->second;
  }

  std::pair<iterator, bool> insert(const value_type &value) {
    return insert_value(value.first, value.second);
  }

  std::pair<iterator, bool> insert(value_type &&value) {
    return insert_value(value.first, std::move(value.second));
  }

  iterator find(const key_type &key) {
    if (_bucket_count == 0) {
      return end();
    }

    const size_type index = bucket_index_for_key(key, _bucket_count);
    return iterator(this, find_node(key, index));
  }

  const_iterator find(const key_type &key) const {
    if (_bucket_count == 0) {
      return cend();
    }

    const size_type index = bucket_index_for_key(key, _bucket_count);
    return const_iterator(this, find_node(key, index));
  }

  bool contains(const key_type &key) const { return find(key) != cend(); }

  size_type count(const key_type &key) const {
    return contains(key) ? 1 : 0;
  }

  iterator erase(const_iterator position) {
    if (position._map != this || position._node == nullptr || _bucket_count == 0) {
      return end();
    }

    const size_type index =
        bucket_index_for_key(position._node->value.first, _bucket_count);
    Node *previous = nullptr;
    Node *current = _buckets[index];

    while (current != nullptr) {
      if (current == position._node) {
        Node *next = next_node(current);
        unlink_node(index, previous, current);
        delete current;
        --_size;
        return iterator(this, next);
      }

      previous = current;
      current = current->next;
    }

    return end();
  }

  size_type erase(const key_type &key) {
    if (_bucket_count == 0) {
      return 0;
    }

    const size_type index = bucket_index_for_key(key, _bucket_count);
    Node *previous = nullptr;
    Node *current = _buckets[index];

    while (current != nullptr) {
      if (_equal(current->value.first, key)) {
        unlink_node(index, previous, current);
        delete current;
        --_size;
        return 1;
      }

      previous = current;
      current = current->next;
    }

    return 0;
  }

  void clear() {
    for (size_type index = 0; index < _bucket_count; ++index) {
      Node *current = _buckets[index];
      while (current != nullptr) {
        Node *next = current->next;
        delete current;
        current = next;
      }
      _buckets[index] = nullptr;
    }
    _size = 0;
  }

  void rehash(size_type new_bucket_count) {
    if (new_bucket_count < _size) {
      new_bucket_count = _size;
    }
    new_bucket_count = normalize_bucket_count(new_bucket_count);

    Node **new_buckets = allocate_bucket_array(new_bucket_count);
    for (size_type index = 0; index < _bucket_count; ++index) {
      Node *current = _buckets[index];
      while (current != nullptr) {
        Node *next = current->next;
        const size_type new_index =
            bucket_index_for_key(current->value.first, new_bucket_count);
        current->next = new_buckets[new_index];
        new_buckets[new_index] = current;
        current = next;
      }
    }

    delete[] _buckets;
    _buckets = new_buckets;
    _bucket_count = new_bucket_count;
  }

  void reserve(size_type desired_size) {
    if (desired_size > _bucket_count) {
      rehash(desired_size);
    }
  }

  void swap(UnorderedMap &other) {
    std::swap(_buckets, other._buckets);
    std::swap(_bucket_count, other._bucket_count);
    std::swap(_size, other._size);
    std::swap(_hash, other._hash);
    std::swap(_equal, other._equal);
  }

private:
  static constexpr size_type kDefaultBucketCount = 8;

  Node **_buckets;
  size_type _bucket_count;
  size_type _size;
  Hash _hash;
  KeyEqual _equal;

  static size_type normalize_bucket_count(size_type bucket_count) {
    return bucket_count == 0 ? 1 : bucket_count;
  }

  static Node **allocate_bucket_array(size_type bucket_count) {
    return new Node *[bucket_count]();
  }

  size_type bucket_index_for_key(const key_type &key,
                                 size_type bucket_count) const {
    return _hash(key) % bucket_count;
  }

  Node *find_next_node(size_type start_bucket) const {
    for (size_type index = start_bucket; index < _bucket_count; ++index) {
      if (_buckets[index] != nullptr) {
        return _buckets[index];
      }
    }
    return nullptr;
  }

  Node *first_node() const { return find_next_node(0); }

  Node *next_node(const Node *node) const {
    if (node == nullptr) {
      return nullptr;
    }
    if (node->next != nullptr) {
      return node->next;
    }
    if (_bucket_count == 0) {
      return nullptr;
    }

    const size_type index = bucket_index_for_key(node->value.first, _bucket_count);
    return find_next_node(index + 1);
  }

  Node *find_node(const key_type &key, size_type bucket_index) const {
    Node *current = _buckets[bucket_index];
    while (current != nullptr) {
      if (_equal(current->value.first, key)) {
        return current;
      }
      current = current->next;
    }
    return nullptr;
  }

  void ensure_capacity_for_insert() {
    if (_bucket_count == 0) {
      rehash(kDefaultBucketCount);
      return;
    }

    if (_size >= _bucket_count) {
      rehash(_bucket_count * 2);
    }
  }

  void unlink_node(size_type bucket_index, Node *previous, Node *current) {
    if (previous == nullptr) {
      _buckets[bucket_index] = current->next;
      return;
    }
    previous->next = current->next;
  }

  template <typename KeyArg, typename MappedArg>
  std::pair<iterator, bool> insert_value(KeyArg &&key, MappedArg &&mapped) {
    if (_bucket_count != 0) {
      const size_type index = bucket_index_for_key(key, _bucket_count);
      Node *existing = find_node(key, index);
      if (existing != nullptr) {
        return std::make_pair(iterator(this, existing), false);
      }
    }

    ensure_capacity_for_insert();

    const size_type index = bucket_index_for_key(key, _bucket_count);
    Node *node = new Node(std::forward<KeyArg>(key), std::forward<MappedArg>(mapped));
    node->next = _buckets[index];
    _buckets[index] = node;
    ++_size;

    return std::make_pair(iterator(this, node), true);
  }
};

#endif
