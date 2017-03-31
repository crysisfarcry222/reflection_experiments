#pragma once
#include "string_literal.hpp"

template<typename ...Types>
struct reduce_pack {
  // Find the type in the pack which is non-void

  template <typename Tn, void* ...v>
  static Tn f(decltype(v)..., Tn, ...);

  using type = decltype(f(std::declval<Types>()...));
};

template <typename... Members>
using member_pack_as_tuple = std::tuple<
    std::meta::get_reflected_type_t<std::meta::get_type_m<Members>>...>;

template<typename T>
struct n_fields {
  using MetaObj = reflexpr(T);
  template<typename ...MetaFields>
  struct n_fields_helper {
    constexpr static int value = sizeof...(MetaFields);
  };

  constexpr static std::size_t value = std::meta::unpack_sequence_t<
    std::meta::get_data_members_m<MetaObj>, n_fields_helper>::value;
};



template<size_t N, typename T>
struct decl_data_member_type {
  using Element = std::meta::get_element_m<std::meta::get_data_members_m<reflexpr(T)>, N>;
  using type = decltype(std::declval<T>().*(std::meta::get_pointer<Element>::value));
};

template<typename T, typename MemberName>
struct has_member {
  // strip away alias
  using MetaObj = std::meta::get_aliased_m<reflexpr(T)>;

  template<typename ...MetaField>
  struct has_member_pack {
    // using field = std::meta::get_base_name_v<MetaField>;
    static constexpr bool value = ((unpack_string_literal<MemberName>::equal(std::meta::get_base_name_v<MetaField>)) || ...);
  };

  static constexpr bool value = std::meta::unpack_sequence_t<
    std::meta::get_data_members_m<MetaObj>, has_member_pack>::value;
};

template<typename MetaObj, typename MemberName>
struct meta_has_member {

  template<typename ...MetaField>
  struct has_member_pack {
    static constexpr bool value = ((unpack_string_literal<MemberName>::equal(std::meta::get_base_name_v<MetaField>)) || ...);
  };

  static constexpr bool value = std::meta::unpack_sequence_t<
    std::meta::get_data_members_m<MetaObj>, has_member_pack>::value;
};

template<typename T, typename MemberName>
struct get_member {
  using MetaObj = reflexpr(T);

  template<typename ...MetaField>
  struct get_member_pack {
    // using field = std::meta::get_base_name_v<MetaField>;
    using field = typename reduce_pack<
      typename std::conditional<
        unpack_string_literal<MemberName>::equal(std::meta::get_base_name_v<MetaField>),
          MetaField, void*
      >::type...
    >::type;
  };
  // get the data member with the requested name
  using MetaField = typename std::meta::unpack_sequence_t<std::meta::get_data_members_m<MetaObj>, get_member_pack>::field;
};

template<typename T, typename MemberName>
struct index_of_member {
  using MetaObj = reflexpr(T);

  template<std::size_t ...i>
  constexpr static std::size_t index_helper(std::index_sequence<i...>) {
    return ((unpack_string_literal<MemberName>::equal(
          std::meta::get_base_name_v<std::meta::get_element_m<MetaObj, i>>) ? i : 0) + ...);
  }

  constexpr static std::size_t index = index_helper(std::make_index_sequence<n_fields<T>::value>{});
};