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
#include <shared_mutex>
#include <tally/variable.h>

namespace tally {

    class ScopeBuilder;
    class ScopeInstance;

    class Scope {
    public:
        ~Scope() = default;

        // Tagged creates a new child scope with the same name as the parent and with
        // the tags of the parent and those provided. The provided tags take
        // precedence over the parent's tags.
        std::shared_ptr<Scope> tagged(
                const turbo::flat_hash_map<std::string, std::string> &tags) noexcept;

        std::string fully_qualified_name(std::string_view name) const;

        // SubScope creates a new child scope with the same tags as the parent but
        // with the additional name.
        std::shared_ptr<tally::Scope> sub_scope(std::string_view name) const noexcept;

        std::shared_ptr<tally::Scope>
        sub_scope(std::string_view name, const turbo::flat_hash_map<std::string, std::string> &tags) const noexcept;

        void describe(std::ostream &) const;

        const std::string &prefix() const {
            return _prefix;
        }

        const std::string &separator() const {
            return _separator;
        }

        const turbo::flat_hash_map<std::string, std::string> &tags() const {
            return _tags;
        }

        const std::string id() const {
            return _id;
        }

        static std::string scope_id(
                std::string_view prefix,
                const turbo::flat_hash_map<std::string, std::string> &tags);


    private:

        friend class Variable;

        friend class ScopeBuilder;
        friend class ScopeInstance;

        std::shared_ptr<tally::Scope>
        sub_scope_internal(std::string_view name, const turbo::flat_hash_map<std::string, std::string> &tags) const noexcept;

        Scope(std::string_view prefix, std::string_view id, std::string_view separator,
              const turbo::flat_hash_map<std::string, std::string> &tags);

        // SubScope constructs a subscope with the provided prefix and tags.
        std::shared_ptr<tally::Scope> sub_cope_impl(
                const std::string &prefix,
                const turbo::flat_hash_map<std::string, std::string> &tags, bool add_to_instance = true) const;

    private:
        const std::string _prefix;
        const std::string _id;
        const std::string _separator;
        const turbo::flat_hash_map<std::string, std::string> _tags;

    };

    class ScopeInstance {
    public:
        ~ScopeInstance() = default;

        static ScopeInstance *instance();

        // include_default if set true,
        // the result include scope default cope flag and sys
        std::vector<std::shared_ptr<Scope>> list_scopes(bool exclude_default = false) const;

        std::shared_ptr<Scope>
        get_scope(std::string_view prefix, const turbo::flat_hash_map<std::string, std::string> &tags) const;

        // exclude_default set true exclude root flag sys scopes
        size_t scope_size(bool exclude_default = false) const;

        std::shared_ptr<Scope> get_scope(std::string_view id) const;

        std::shared_ptr<Scope> get_default() {
            return _root_scope;
        }

        std::shared_ptr<Scope> get_flag_scope() {
            return _flag_scope;
        }

        std::shared_ptr<Scope> get_sys_scope() {
            return _sys_scope;
        }

        // full name
        bool has_scope(std::string_view full_name);

        bool has_scope(std::string_view name, const turbo::flat_hash_map<std::string, std::string> &tags) {
            return has_scope(Scope::scope_id(name, tags));
        }
    private:
        friend class ScopeBuilder;

        ScopeInstance() = default;

        void init_once();

        std::shared_ptr<Scope> add_scope(std::shared_ptr<Scope> ptr);

    private:
        mutable std::shared_mutex _registry_mutex;
        turbo::flat_hash_map<std::string, std::shared_ptr<Scope>> _registry;

        std::shared_ptr<Scope> _flag_scope;

        std::shared_ptr<Scope> _sys_scope;
        std::shared_ptr<Scope> _root_scope;
    };

}  // namespace tally

// Make variables printable.
namespace std {

    inline ostream &operator<<(ostream &os, const ::tally::Scope &s) {
        s.describe(os);
        return os;
    }

}  // namespace std
