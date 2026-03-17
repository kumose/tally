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
