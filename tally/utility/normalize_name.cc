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
#include <tally/utility/normalize_name.h>

namespace tally {

    void to_underscored_name(std::string *name, std::string_view src) {
        name->reserve(name->size() + src.size() + 8/*just guess*/);
        for (const char *p = src.data(); p != src.data() + src.size(); ++p) {
            if (isalpha(*p)) {
                if (*p < 'a') { // upper cases
                    if (p != src.data() && !isupper(p[-1]) &&
                        back_char(*name) != '_') {
                        name->push_back('_');
                    }
                    name->push_back(*p - 'A' + 'a');
                } else {
                    name->push_back(*p);
                }
            } else if (isdigit(*p)) {
                name->push_back(*p);
            } else if (name->empty() || back_char(*name) != '_') {
                name->push_back('_');
            }
        }
    }
}  // namespace tally
