// Copyright (C) 2024 Kumo inc.
// Author: Jeff.li lijippy@163.com
// All rights reserved.
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#pragma once

#include <tally/utility/type_traits.h>

namespace tally::detail {
    template<class T>
    struct is_atomical
            : std::integral_constant<bool, (std::is_integral<T>::value ||
                                              std::is_floating_point<T>::value)
            > {
    };
    template<class T>
    struct is_atomical<const T> : is_atomical<T> {
    };
    template<class T>
    struct is_atomical<volatile T> : is_atomical<T> {
    };
    template<class T>
    struct is_atomical<const volatile T> : is_atomical<T> {
    };

}  // namespace tally::detail
