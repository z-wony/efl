/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef EFL_EET_COMPOSITE_HH_
#define EFL_EET_COMPOSITE_HH_

namespace efl { namespace eet {

template <typename, typename...> struct descriptor;

namespace _detail {

struct member_desc_info
{
  const char* name;
  std::size_t offset;
};

template <std::size_t E, typename U, typename... Types>
void descriptor_register_composite_member( ::Eet_Data_Descriptor*, int
                                           , eet::descriptor<U, Types...>const*
                                           , std::integral_constant<std::size_t, E>)
{
}

template <std::size_t E, typename U, typename... Types, std::size_t I>
void descriptor_register_composite_member( ::Eet_Data_Descriptor* cls, int offset_base
                                           , eet::descriptor<U, Types...>const* descriptor
                                           , std::integral_constant<std::size_t, I>
                                           , typename std::enable_if<E != I>::type* = 0)
{
  typedef typename std::tuple_element<I, std::tuple<Types...> >::type member_type;
  eet_data_descriptor_element_add(cls, descriptor->get_member_info()[I].name
                                  , _eet_type<member_type>::value, EET_G_UNKNOWN
                                  , offset_base + descriptor->get_member_info()[I].offset
                                  , 0, nullptr, nullptr);

  _detail::descriptor_register_composite_member<E>
    (cls, offset_base, descriptor, std::integral_constant<std::size_t, I+1>());
}

template <typename U, typename...Types>
void descriptor_type_register_composite( ::Eet_Data_Descriptor* cls, member_desc_info info
                                         , eet::descriptor<U, Types...>const* descriptor)
{
  _detail::descriptor_register_composite_member<eet::descriptor<U, Types...>::members::value>
    (cls, info.offset, descriptor, std::integral_constant<std::size_t, 0u>());
}

} } }

#endif
