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

