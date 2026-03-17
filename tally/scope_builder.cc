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

#include <tally/noop_stats_reporter.h>
#include <tally/scope_builder.h>
#include <tally/scope.h>
#include <tally/config.h>
#include <tally/utility/normalize_name.h>

namespace tally {

    namespace {
        const turbo::flat_hash_map<std::string, std::string> DEFAULT_TAGS =
                turbo::flat_hash_map<std::string, std::string>{};
    }  // namespace

    ScopeBuilder::ScopeBuilder()
            : prefix_(),
              separator_(turbo::get_flag(FLAGS_tally_scope_separator)),
              tags_(DEFAULT_TAGS) {}


    ScopeBuilder &ScopeBuilder::prefix(std::string_view prefix) noexcept {
        prefix_ = prefix;
        return *this;
    }

    ScopeBuilder &ScopeBuilder::separator(std::string_view sep) noexcept {
        separator_ = sep;
        return *this;
    }

    ScopeBuilder &ScopeBuilder::tags(
            const turbo::flat_hash_map<std::string, std::string> &tags) noexcept {
        tags_ = tags;
        for(auto & it : tags) {
            tags_[it.first] = it.second;
        }
        return *this;
    }

    ScopeBuilder &ScopeBuilder::tags(std::string_view tag, std::string_view value) noexcept {
        tags_[tag] = value;
        return *this;
    }

    std::shared_ptr<Scope> ScopeBuilder::build() noexcept {
        if (this->prefix_.empty()) {
            return nullptr;
        }

        std::string normed;
        to_underscored_name(&normed, prefix_);
        auto id = Scope::scope_id(normed, tags_);
        auto ptr = ScopeInstance::instance()->get_scope(id);
        if (ptr) {
            return ptr;
        }


        ptr = std::shared_ptr<Scope>(new Scope(normed, id, this->separator_, this->tags_));
        auto ret = ScopeInstance::instance()->add_scope(ptr);
        return ret;
    }

    std::shared_ptr<Scope> ScopeBuilder::build_only() noexcept {
        if (this->prefix_.empty()) {
            return nullptr;
        }
        std::string normed;
        to_underscored_name(&normed, prefix_);
        auto id = Scope::scope_id(normed, tags_);

        auto ptr = std::shared_ptr<Scope>(new Scope(normed, id, this->separator_, this->tags_));
        return ptr;
    }

}  // namespace tally
