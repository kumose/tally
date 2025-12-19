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
