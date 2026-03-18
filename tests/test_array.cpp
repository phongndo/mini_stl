#define CATCH_CONFIG_MAIN
#include "array.hpp"
#include <catch2/catch.hpp>
#include <stdexcept>

TEST_CASE("Array basic properties", "[Array]") {
  Array<int, 4> arr{};

  REQUIRE(!arr.empty());
  REQUIRE(arr.size() == 4);
  REQUIRE(arr.data() != nullptr);
}

TEST_CASE("Array element access", "[Array]") {
  Array<int, 3> arr{};
  arr[0] = 10;
  arr[1] = 20;
  arr[2] = 30;

  REQUIRE(arr[0] == 10);
  REQUIRE(arr.at(1) == 20);
  REQUIRE(arr.front() == 10);
  REQUIRE(arr.back() == 30);
  REQUIRE_THROWS_AS(arr.at(3), std::out_of_range);
}

TEST_CASE("Array iterator support", "[Array]") {
  Array<int, 4> arr{};
  for (std::size_t i = 0; i < arr.size(); ++i) {
    arr[i] = static_cast<int>(i + 1);
  }

  int sum = 0;
  for (auto value : arr) {
    sum += value;
  }

  REQUIRE(sum == 10);
}

TEST_CASE("Array fill and const access", "[Array]") {
  Array<int, 5> arr{};
  arr.fill(7);

  const Array<int, 5> &const_arr = arr;
  REQUIRE(const_arr[0] == 7);
  REQUIRE(const_arr.at(4) == 7);
  REQUIRE(const_arr.front() == 7);
  REQUIRE(const_arr.back() == 7);
}

TEST_CASE("Zero-sized Array reports empty", "[Array]") {
  Array<int, 0> arr;

  REQUIRE(arr.empty());
  REQUIRE(arr.size() == 0);
  REQUIRE(arr.begin() == arr.end());
  REQUIRE(arr.data() == nullptr);
}
