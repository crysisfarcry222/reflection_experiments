#pragma once

#include <experimental/type_traits>
#include <string>
#include <type_traits>
#include <tuple>

namespace jk {
namespace metaprogramming {

// from http://stackoverflow.com/questions/16337610/how-to-know-if-a-type-is-a-specialization-of-stdvector
template<typename Test, template<typename...> class Ref>
struct is_specialization : std::false_type {};

template<template<typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref>: std::true_type {};

template<typename T>
using stringable = decltype(std::to_string(std::declval<T>()));

template<typename T>
using iterable = std::void_t<
  decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>;

template<typename T>
using resizable = decltype(std::declval<T>().resize(std::declval<size_t>()));
// ???
// using resizable = decltype(std::declval<T>().resize(std::declval<T::size_type>()));

template<typename T>
using has_tuple_size = decltype(std::tuple_size<T>::value);
// TODO I want this to be like this but I think it might be a bug
// using has_tuple_size = std::tuple_size<T>;

template<typename T>
using equality_comparable = decltype(std::declval<T>() == std::declval<T>());

template<template<typename ...> typename Op, typename... Args>
using is_detected = std::experimental::is_detected<Op, Args...>;

static constexpr unsigned cstrlen(const char* str) {
  return *str ? 1 + cstrlen(str + 1) : 0;
}

// thx superv
#define CONSTEXPR_ERROR(Message) \
  [](auto x) { \
    static_assert(decltype(x){}, Message); \
  }(std::false_type{}); \

}  // namespace metaprogramming
}  // namespace jk
