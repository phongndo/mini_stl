#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <cstddef>
#include <stdexcept>

template <typename T, std::size_t N> struct ArrayStorage {
  T data[N];

  T *ptr() { return data; }
  const T *ptr() const { return data; }
};

template <typename T> struct ArrayStorage<T, 0> {
  T *ptr() { return nullptr; }
  const T *ptr() const { return nullptr; }
};

template <typename T, std::size_t N> class Array {
public:
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = value_type &;
  using const_reference = const value_type &;
  using pointer = value_type *;
  using const_pointer = const value_type *;
  using iterator = pointer;
  using const_iterator = const_pointer;

  iterator begin() { return _storage.ptr(); }
  iterator end() { return N == 0 ? _storage.ptr() : _storage.ptr() + N; }
  const_iterator begin() const { return _storage.ptr(); }
  const_iterator end() const {
    return N == 0 ? _storage.ptr() : _storage.ptr() + N;
  }
  const_iterator cbegin() const { return _storage.ptr(); }
  const_iterator cend() const {
    return N == 0 ? _storage.ptr() : _storage.ptr() + N;
  }

  bool empty() const { return N == 0; }
  size_type size() const { return N; }

  reference operator[](size_type index) { return _storage.ptr()[index]; }
  const_reference operator[](size_type index) const {
    return _storage.ptr()[index];
  }

  reference at(size_type index) {
    if (index >= N) {
      throw std::out_of_range("Out of bounds");
    }
    return _storage.ptr()[index];
  }

  const_reference at(size_type index) const {
    if (index >= N) {
      throw std::out_of_range("Out of bounds");
    }
    return _storage.ptr()[index];
  }

  reference front() { return _storage.ptr()[0]; }
  const_reference front() const { return _storage.ptr()[0]; }

  reference back() { return _storage.ptr()[N - 1]; }
  const_reference back() const { return _storage.ptr()[N - 1]; }

  pointer data() { return _storage.ptr(); }
  const_pointer data() const { return _storage.ptr(); }

  void fill(const_reference value) {
    for (size_type i = 0; i < N; ++i) {
      _storage.ptr()[i] = value;
    }
  }

private:
  ArrayStorage<T, N> _storage;
};

#endif
