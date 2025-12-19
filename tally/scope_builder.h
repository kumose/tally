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
#include <turbo/container/flat_hash_map.h>

#include <tally/buckets.h>
#include <tally/scope.h>
#include <tally/noop_stats_reporter.h>
#include <tally/stats_reporter.h>
#include <memory>

namespace tally {

    class ScopeInstance;

    class ScopeBuilder {
    public:
        ScopeBuilder();

        ScopeBuilder &prefix(std::string_view prefix) noexcept;

        ScopeBuilder &separator(std::string_view sep) noexcept;

        ScopeBuilder &tags(
                const turbo::flat_hash_map<std::string, std::string> &tags) noexcept;

        ScopeBuilder &tags(std::string_view tag, std::string_view  value) noexcept;

        // Build constructs a Scope and begins reporting metrics if the scope's
        // reporting interval is non-zero.
        std::shared_ptr<Scope> build() noexcept;
        // do not register to ScopeInstance, for test and internal using
        std::shared_ptr<Scope> build_only() noexcept;
    private:
        std::string prefix_;
        std::string separator_;
        turbo::flat_hash_map<std::string, std::string> tags_;
    };

}  // namespace tally
