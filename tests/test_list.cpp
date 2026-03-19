#define CATCH_CONFIG_MAIN
#include "list.hpp"
#include <catch2/catch.hpp>
#include <cstddef>
#include <utility>

TEST_CASE("List basic properties", "[List]") {
  List<int> list;

  REQUIRE(list.empty());
  REQUIRE(list.size() == 0);
  REQUIRE(list.begin() == list.end());
}

TEST_CASE("List push and traversal", "[List]") {
  List<int> list;

  list.push_back(2);
  list.push_front(1);
  list.push_back(3);

  REQUIRE(!list.empty());
  REQUIRE(list.size() == 3);
  REQUIRE(list.front() == 1);
  REQUIRE(list.back() == 3);

  const int expected[] = {1, 2, 3};
  std::size_t index = 0;
  for (int value : list) {
    REQUIRE(value == expected[index]);
    ++index;
  }
  REQUIRE(index == 3);

  const List<int> &const_list = list;
  auto it = const_list.cend();
  --it;
  REQUIRE(*it == 3);
  --it;
  REQUIRE(*it == 2);
}

TEST_CASE("List insert and erase", "[List]") {
  List<int> list;
  list.push_back(1);
  list.push_back(3);

  auto position = list.begin();
  ++position;
  auto inserted = list.insert(position, 2);

  REQUIRE(*inserted == 2);
  REQUIRE(list.size() == 3);

  const int expected[] = {1, 2, 3};
  auto it = list.begin();
  for (int value : expected) {
    REQUIRE(*it == value);
    ++it;
  }

  auto next = list.erase(inserted);
  REQUIRE(*next == 3);
  REQUIRE(list.size() == 2);
  REQUIRE(list.back() == 3);
}

TEST_CASE("List copy and move semantics", "[List]") {
  List<int> original;
  original.push_back(10);
  original.push_back(20);

  List<int> copy(original);
  REQUIRE(copy.size() == 2);
  REQUIRE(copy.front() == 10);
  REQUIRE(copy.back() == 20);

  copy.front() = 30;
  REQUIRE(original.front() == 10);
  REQUIRE(copy.front() == 30);

  List<int> moved(std::move(copy));
  REQUIRE(moved.size() == 2);
  REQUIRE(moved.front() == 30);
  REQUIRE(moved.back() == 20);
  REQUIRE(copy.empty());

  List<int> assigned;
  assigned = original;
  REQUIRE(assigned.size() == 2);
  REQUIRE(assigned.front() == 10);

  assigned = std::move(moved);
  REQUIRE(assigned.size() == 2);
  REQUIRE(assigned.front() == 30);
  REQUIRE(assigned.back() == 20);
  REQUIRE(moved.empty());
}

