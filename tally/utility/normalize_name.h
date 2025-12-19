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

#include <string_view>
#include <string>

namespace tally {

    // Make name only use lowercased alphabets / digits / underscores, and append
    // the result to `out'.
    // Examples:
    //   foo-inl.h       -> foo_inl_h
    //   foo::bar::Apple -> foo_bar_apple
    //   Car_Rot         -> car_rot
    //   FooBar          -> foo_bar
    //   RPCTest         -> rpctest
    //   HELLO           -> hello
    void to_underscored_name(std::string* out, std::string_view name);

    inline char front_char(const std::string &s) { return s[0]; }
    inline char back_char(const std::string &s) { return s[s.size() - 1]; }
}  // namespace tally

