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

#include <string>
#include <turbo/strings/str_split.h>
#include <turbo/container/flat_hash_set.h>

namespace tally {

    class WildcardMatcher {
    public:
        WildcardMatcher(std::string_view wildcards,
                        char question_mark,
                        bool on_both_empty)
                : _question_mark(question_mark), _on_both_empty(on_both_empty) {
            if (wildcards.empty()) {
                return;
            }
            std::string name;
            std::vector<std::string_view> sps = turbo::str_split(wildcards, turbo::ByAnyChar(",;"));
            const char wc_pattern[3] = {'*', question_mark, '\0'};
            for (auto sp = sps.begin();
                 sp != sps.end(); ++sp) {
                name.assign(sp->data(), sp->size());
                if (name.find_first_of(wc_pattern) != std::string::npos) {
                    if (_wcs.empty()) {
                        _wcs.reserve(8);
                    }
                    _wcs.push_back(name);
                } else {
                    _exact.insert(name);
                }
            }
        }

        bool match(std::string_view name) const {
            if (!_exact.empty()) {
                if (_exact.find(name) != _exact.end()) {
                    return true;
                }
            } else if (_wcs.empty()) {
                return _on_both_empty;
            }
            for (size_t i = 0; i < _wcs.size(); ++i) {
                if (wild_cmp(_wcs[i], name, _question_mark)) {
                    return true;
                }
            }
            return false;
        }

        const std::vector<std::string> &wildcards() const { return _wcs; }

        const turbo::flat_hash_set<std::string> &exact_names() const { return _exact; }
    private:
        static bool wild_cmp(std::string_view wild, std::string_view str, char question_mark) {
            std::string_view::const_iterator cp;
            std::string_view::const_iterator mp;
            auto sit = str.begin();
            auto wit = wild.begin();
            while (sit != str.end() && *wit != '*') {
                if (*wit != *sit && *wit != question_mark) {
                    return false;
                }
                ++wit;
                ++sit;
            }

            while (sit != str.end()) {
                if (*wit == '*') {
                    if (++wit == wild.end()) {
                        return true;
                    }
                    mp = wit;
                    cp = sit + 1;
                } else if (*wit == *sit || *wit == question_mark) {
                    ++wit;
                    ++sit;
                } else {
                    wit = mp;
                    sit = cp++;
                }
            }

            while (*wit == '*') {
                ++wit;
            }
            return wit == wild.end();
        }

    private:
        char _question_mark;
        bool _on_both_empty;
        std::vector<std::string> _wcs;
        turbo::flat_hash_set<std::string> _exact;
    };
}  // namespace tally
