#pragma once
#include "../string_literal.hpp"
#include "generalized_fold.hpp"
#include <reflexpr>

#include <experimental/type_traits>
#include <variant>

namespace jk {
namespace refl_utilities {

namespace sl = string_literal;
namespace meta = std::meta;

template<typename MetaT>
using unreflect_type = meta::get_reflected_type_t<meta::get_type_m<MetaT>>;

template<typename ...Types>
struct reduce_pack {
  // Find the type in the pack which is non-void

  template <typename Tn, void* ...v>
  static Tn f(decltype(v)..., Tn, ...);

  using type = decltype(f(std::declval<Types>()...));
};

template <typename... Members>
using member_pack_as_tuple = std::tuple<
  meta::get_reflected_type_t<meta::get_type_m<Members>>...>;

template<typename T>
struct n_fields : meta::get_size<meta::get_data_members_m<reflexpr(T)>> {};

// generic meta-object fold
template<typename ...Object>
struct runtime_fold_helper {
  template<typename T, typename Init, typename Func>
  static inline auto apply(T&& t, Init&& init, Func&& func) {
    return fold(func, init, t.*meta::get_pointer<Object>::value ...);
  }
};

// Func is a callable of arity 2
template<typename T, typename Init, typename Func,
  typename std::enable_if_t<meta::get_size<meta::get_data_members_m<reflexpr(std::decay_t<T>)>>{} >= 1>* = nullptr>
auto fold_over_data_members(Func&& func, Init&& init, T&& t) {
  using MetaT = reflexpr(std::decay_t<T>);
	return meta::unpack_sequence_t<meta::get_data_members_m<MetaT>, runtime_fold_helper>::apply(t, init, func);
}

template<typename ...MetaField>
struct has_member_pack {
  template<typename StrT>
  static constexpr bool apply(const StrT& name) {
    return (sl::equal(name, meta::get_base_name_v<MetaField>) || ...);
  }
};

template<typename T, typename StrT>
constexpr bool has_member(const StrT& member_name) {
  return meta::unpack_sequence_t<meta::get_member_types_m<reflexpr(T)>, has_member_pack>::apply(member_name);
}

template<typename T, typename StrT, std::size_t ...i>
constexpr static auto index_helper(const StrT& name, std::index_sequence<i...>) {
  return ((sl::equal(name,
            meta::get_base_name_v<
              meta::get_element_m<
                meta::get_data_members_m<reflexpr(T)>, i>
              >) ? i : 0
          ) + ...);
}

template<typename T, typename StrT>
constexpr static auto index_of_member(const StrT& name) {
  return index_helper<T>(name, std::make_index_sequence<n_fields<T>{}>{});
}

template<typename T, typename StrT>
constexpr auto get_member_pointer(StrT&&) {
  return meta::get_pointer<
    meta::get_element_m<
      meta::get_data_members_m<reflexpr(T)>,
      index_of_member<T>(StrT{})
    >
  >::value;
}

template<typename T, typename MemberName>
struct get_member_type {
  using type = unreflect_type<
    meta::get_element_m<
      meta::get_data_members_m<reflexpr(T)>,
      index_of_member<T>(MemberName{})
    >
  >;
};

// free metafunctions for metaobjects
struct get_name {
  template<typename MetaT>
  constexpr auto operator()(MetaT&& t) {
    return meta::get_base_name<MetaT>{};
  }
};


template<typename T>
struct members_variant : std::variant<meta::get_data_members_m<reflexpr(T)> > {};

/*
// return a reference to the field in T that matches the runtime-determined string value
template<typename T>
members_variant<T> get_member(const T& x, const std::string_view& name) {
  // This is a product type-to-sum type mapping
}
*/

// TODO think this is wrong
template<typename T, typename StringT, typename Callable>
void call_for_member(const T& x, const StringT& name, Callable&& callback) {
  auto wrapped_callback = [&name, &callback](auto&& metainfo) {
    using MetaInfo = std::decay_t<decltype(metainfo)>;
    if (sl::equal(name, meta::get_base_name_v<MetaInfo>)) {
      callback(metainfo);
    }
  };
  meta::for_each<meta::get_data_members_m<reflexpr(std::decay_t<T>)>>(wrapped_callback);
}

}  // namespace refl_utilities
}  // namespace jk