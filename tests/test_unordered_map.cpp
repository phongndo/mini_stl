#define CATCH_CONFIG_MAIN
#include "unordered_map.hpp"
#include <catch2/catch.hpp>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>

namespace {
struct ConstantHash {
  std::size_t operator()(int) const { return 0; }
};
} // namespace

TEST_CASE("UnorderedMap basic properties", "[UnorderedMap]") {
  UnorderedMap<std::string, int> map;

  REQUIRE(map.empty());
  REQUIRE(map.size() == 0);
  REQUIRE(map.begin() == map.end());
  REQUIRE(map.bucket_count() >= 1);
}

TEST_CASE("UnorderedMap insert and lookup", "[UnorderedMap]") {
  UnorderedMap<std::string, int> map;

  auto inserted = map.insert({"one", 1});
  REQUIRE(inserted.second);
  REQUIRE(inserted.first->first == "one");
  REQUIRE(inserted.first->second == 1);
  REQUIRE(map.size() == 1);

  auto duplicate = map.insert({"one", 2});
  REQUIRE_FALSE(duplicate.second);
  REQUIRE(duplicate.first->second == 1);
  REQUIRE(map.size() == 1);

  REQUIRE(map.find("one") != map.end());
  REQUIRE(map.count("one") == 1);
  REQUIRE(map.count("missing") == 0);
  REQUIRE(map.at("one") == 1);
}

TEST_CASE("UnorderedMap operator[] default inserts and updates", "[UnorderedMap]") {
  UnorderedMap<std::string, int> map;

  REQUIRE(map["alpha"] == 0);
  REQUIRE(map.size() == 1);

  map["alpha"] = 4;
  map[std::string("beta")] += 2;

  REQUIRE(map.at("alpha") == 4);
  REQUIRE(map.at("beta") == 2);
  REQUIRE_THROWS_AS(map.at("missing"), std::out_of_range);
}

TEST_CASE("UnorderedMap erase and clear", "[UnorderedMap]") {
  UnorderedMap<std::string, int> map;
  map.insert({"one", 1});
  map.insert({"two", 2});
  map.insert({"three", 3});

  REQUIRE(map.erase("two") == 1);
  REQUIRE(map.erase("two") == 0);
  REQUIRE(map.count("two") == 0);
  REQUIRE(map.size() == 2);

  auto it = map.find("one");
  REQUIRE(it != map.end());
  auto next = map.erase(it);
  REQUIRE(map.count("one") == 0);
  REQUIRE(map.size() == 1);
  REQUIRE(next != map.end());

  map.clear();
  REQUIRE(map.empty());
  REQUIRE(map.size() == 0);
  REQUIRE(map.begin() == map.end());
}

TEST_CASE("UnorderedMap supports iteration copy and move", "[UnorderedMap]") {
  UnorderedMap<int, int> map;
  for (int i = 1; i <= 5; ++i) {
    map.insert({i, i * 10});
  }

  int key_sum = 0;
  int value_sum = 0;
  for (const auto &entry : map) {
    key_sum += entry.first;
    value_sum += entry.second;
  }

  REQUIRE(key_sum == 15);
  REQUIRE(value_sum == 150);

  UnorderedMap<int, int> copy(map);
  REQUIRE(copy.size() == map.size());
  copy[1] = 99;
  REQUIRE(map.at(1) == 10);
  REQUIRE(copy.at(1) == 99);

  UnorderedMap<int, int> moved(std::move(copy));
  REQUIRE(moved.size() == 5);
  REQUIRE(copy.empty());

  UnorderedMap<int, int> assigned;
  assigned = map;
  REQUIRE(assigned.size() == map.size());
  REQUIRE(assigned.at(2) == 20);

  UnorderedMap<int, int> move_assigned;
  move_assigned = std::move(moved);
  REQUIRE(move_assigned.size() == 5);
  REQUIRE(move_assigned.at(1) == 99);
  REQUIRE(moved.empty());
}

TEST_CASE("UnorderedMap handles collisions and rehashing", "[UnorderedMap]") {
  UnorderedMap<int, int, ConstantHash> map(1);

  for (int i = 0; i < 32; ++i) {
    auto result = map.insert({i, i * 2});
    REQUIRE(result.second);
  }

  REQUIRE(map.size() == 32);
  REQUIRE(map.bucket_count() >= 32);

  for (int i = 0; i < 32; ++i) {
    REQUIRE(map.at(i) == i * 2);
  }

  auto middle = map.find(16);
  REQUIRE(middle != map.end());
  auto next = map.erase(middle);
  REQUIRE(map.count(16) == 0);
  REQUIRE(map.size() == 31);
  REQUIRE(next != map.end());
}
