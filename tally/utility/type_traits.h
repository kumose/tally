// Copyright (C) Kumo inc. and its affiliates.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#pragma once

#include <type_traits>

namespace tally {

    template <typename T> struct add_cr_non_integral;
    // Add const& for non-integral types.
    // add_cr_non_integral<int>::type      -> int
    // add_cr_non_integral<FooClass>::type -> const FooClass&
    template <typename T> struct add_cr_non_integral {
        typedef typename std::conditional<std::is_integral<T>::value, T,
                typename std::add_lvalue_reference<typename std::add_const<T>::type>::type>::type type;
    };

}  // namespace tally
