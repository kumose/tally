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
