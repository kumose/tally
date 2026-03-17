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
