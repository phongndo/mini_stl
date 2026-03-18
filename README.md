# mini_stl

`mini_stl` is a small C++17, header-only project for reimplementing STL-style
containers from scratch. The focus is learning container internals,
iterator behavior, copy and move semantics, and manual storage management
without hiding the mechanics behind a large framework.

## Current Status

Implemented today:

- `DynamicArray<T>` in `include/dynamic_array.hpp`
- `Array<T, N>` in `include/array.hpp`

Scaffolded and planned:

- `deque`
- `forward_list`
- `list`
- `map`
- `priority_queue`
- `queue`
- `set`
- `stack`
- `unordered_map`

This repository does not try to fully match `std::*` yet. The goal is a small,
readable subset that can be extended incrementally.

## Why This Repo Exists

- Learn how common containers work internally
- Practice ownership, allocation, destruction, and iterator design
- Build a test-backed reference for future container implementations
- Keep the code explicit, dependency-light, and easy to debug

## Example

```cpp
#include "array.hpp"
#include "dynamic_array.hpp"

int main() {
  Array<int, 3> fixed{};
  fixed.fill(7);

  DynamicArray<int> dynamic;
  dynamic.push_back(10);
  dynamic.push_back(20);

  return fixed[0] + dynamic[1];
}
```

## Build and Test

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Notes:

- The project uses CMake and requires C++17.
- The first configure fetches Catch2 from GitHub through CMake `FetchContent`.

## Project Layout

```text
include/   header-only container implementations
tests/     Catch2-based test executables
```

## Next Steps

- Expand test coverage for edge cases and iterator correctness
- Implement the remaining container headers one by one
- Keep APIs close to STL naming where it improves familiarity
- Prefer small, verifiable changes over broad refactors
