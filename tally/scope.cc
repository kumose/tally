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

#include <tally/scope.h>
#include <tally/scope_builder.h>
#include <tally/utility/normalize_name.h>
#include <tally/config.h>

namespace tally {

    Scope::Scope(std::string_view prefix, std::string_view id, std::string_view separator,
                 const turbo::flat_hash_map<std::string, std::string> &tags) : _prefix(prefix), _id(id),
                                                                               _separator(separator),
                                                                               _tags(tags) {
    }

    std::string Scope::fully_qualified_name(std::string_view name) const {
        if (_prefix == "") {
            return std::string(name);
        }

        std::string str;
        str.reserve(_prefix.length() + _separator.length() + name.length());
        std::ostringstream stream(str);
        stream << _prefix << _separator << name;

        return stream.str();
    }

    std::string Scope::scope_id(
            std::string_view prefix,
            const turbo::flat_hash_map<std::string, std::string> &tags) {
        std::vector<std::string> keys;
        keys.reserve(tags.size());
        std::transform(
                std::begin(tags), std::end(tags), std::back_inserter(keys),
                [](const std::pair<std::string, std::string> &tag) { return tag.first; });

        std::sort(keys.begin(), keys.end());

        std::string str;
        str.reserve(prefix.length() + keys.size() * 20);
        std::ostringstream stream(str);

        stream << prefix;
        if(!keys.empty()) {
            stream << "+";

            for (auto it = keys.begin(); it < keys.end(); it++) {
                auto key = *it;
                stream << key << "=" << tags.at(key);

                if (it != keys.end() - 1) {
                    stream << ",";
                }
            }
        }

        return stream.str();
    }

    std::shared_ptr<tally::Scope> Scope::tagged(
            const turbo::flat_hash_map<std::string, std::string> &tags) noexcept {
        return sub_cope_impl(_prefix, tags);
    }

    std::shared_ptr<tally::Scope> Scope::sub_scope(std::string_view name) const noexcept {
        return sub_cope_impl(fully_qualified_name(name),
                             turbo::flat_hash_map<std::string, std::string>{});
    }

    std::shared_ptr<tally::Scope>
    Scope::sub_scope(std::string_view name, const turbo::flat_hash_map<std::string, std::string> &tags) const noexcept {
        return sub_cope_impl(fully_qualified_name(name),
                             tags);
    }

    std::shared_ptr<tally::Scope>
    Scope::sub_scope_internal(std::string_view name,
                              const turbo::flat_hash_map<std::string, std::string> &tags) const noexcept {
        return sub_cope_impl(fully_qualified_name(name),
                             tags, false);
    }

    void Scope::describe(std::ostream &os) const {
        os << "{name: " << _prefix;
        bool not_first = false;
        os << ", tags: {";
        for (auto &it: _tags) {
            if (not_first) {
                os << ", ";
            } else {
                not_first = true;
            }
            os << "{" << it.first << ": " << it.second << "}";
        }
        os << "}";
        os << ", id: " << _id;
        os << "}";
    }

    std::shared_ptr<tally::Scope> Scope::sub_cope_impl(
            const std::string &prefix,
            const turbo::flat_hash_map<std::string, std::string> &tags, bool add_to_instance) const {
        turbo::flat_hash_map<std::string, std::string> new_tags;

        // Insert the new tags second as they take priority over the scope's tags.
        for (auto const &tag: _tags) {
            new_tags.insert(tag);
        }

        for (auto const &tag: tags) {
            new_tags.insert(tag);
        }
        if (add_to_instance) {
            std::shared_ptr<Scope> scope = ScopeBuilder()
                    .prefix(prefix)
                    .separator(_separator)
                    .tags(new_tags)
                    .build();
            return scope;
        } else {
            std::shared_ptr<Scope> scope = ScopeBuilder()
                    .prefix(prefix)
                    .separator(_separator)
                    .tags(new_tags)
                    .build_only();
            return scope;
        }
    }

    static turbo::flat_hash_map<std::string, std::string> parse_tags(std::string_view tag) {
        turbo::flat_hash_map<std::string, std::string> ret;
        std::vector<std::string_view> seg = turbo::str_split(tag, ";", turbo::SkipEmpty());
        for (auto &it: seg) {
            std::vector<std::string_view> pair = turbo::str_split(it, ":", turbo::SkipEmpty());
            if (pair.size() != 2) {
                continue;
            }
            ret[pair[0]] = pair[1];
        }
        return ret;
    }

    std::vector<std::shared_ptr<Scope>> ScopeInstance::list_scopes(bool exclude_default) const {
        std::vector<std::shared_ptr<Scope>> result;
        {
            std::shared_lock lock(_registry_mutex);
            result.reserve(_registry.size());
            for (auto &it: _registry) {
                result.push_back(it.second);
            }
        }
        if(!exclude_default) {
            result.push_back(_root_scope);
            result.push_back(_sys_scope);
            result.push_back(_flag_scope);
        }
        return result;

    }

    std::shared_ptr<Scope>
    ScopeInstance::get_scope(std::string_view prefix, const turbo::flat_hash_map<std::string, std::string> &tags) const {
        auto id = Scope::scope_id(prefix, tags);
        if(id == _root_scope->id()) {
            return _root_scope;
        } else if(id == _flag_scope->id()) {
            return _flag_scope;
        }else if(id == _sys_scope->id()) {
            return _sys_scope;
        }
        std::shared_lock lock(_registry_mutex);
        auto it = _registry.find(id);
        if (it == _registry.end()) {
            return nullptr;
        }
        return it->second;
    }

    std::shared_ptr<Scope> ScopeInstance::get_scope(std::string_view id) const {
        if(id == _root_scope->id()) {
            return _root_scope;
        } else if(id == _flag_scope->id()) {
            return _flag_scope;
        }else if(id == _sys_scope->id()) {
            return _sys_scope;
        }
        std::shared_lock lock(_registry_mutex);
        auto it = _registry.find(id);
        if (it == _registry.end()) {
            return nullptr;
        }
        return it->second;
    }

    size_t ScopeInstance::scope_size(bool exclude_default) const {
        size_t s = 0;
        {
            std::shared_lock lock(_registry_mutex);
            s = _registry.size();
        }
        if (exclude_default) {
            return s;
        }
        return s + 3;

    }

    bool ScopeInstance::has_scope(std::string_view full_name) {
        if(full_name == _root_scope->id() || full_name == _flag_scope->id() || full_name == _sys_scope->id()) {
            return true;
        }
        std::shared_lock lock(_registry_mutex);
        auto it = _registry.find(full_name);
        if (it == _registry.end()) {
            return false;
        }
        return true;
    }

    ScopeInstance *ScopeInstance::instance() {
        static ScopeInstance ins;
        ins.init_once();
        return &ins;
    }

    std::once_flag init_flag;

    void ScopeInstance::init_once() {
        std::call_once(init_flag, [this]() {
            _root_scope = ScopeBuilder()
                    .prefix(turbo::get_flag(FLAGS_tally_root_scope_name))
                    .separator(turbo::get_flag(FLAGS_tally_scope_separator))
                    .tags(parse_tags(turbo::get_flag(FLAGS_tally_root_scope_tags)))
                    .build_only();
            _sys_scope = _root_scope->sub_scope_internal(turbo::get_flag(FLAGS_tally_sys_scope_name),
                                                         parse_tags(turbo::get_flag(FLAGS_tally_sys_scope_tags)));
            _flag_scope = _root_scope->sub_scope_internal(turbo::get_flag(FLAGS_tally_flag_scope_name),
                                                          parse_tags(turbo::get_flag(FLAGS_tally_flag_scope_tags)));
        });
    }

    std::shared_ptr<Scope> ScopeInstance::add_scope(std::shared_ptr<Scope> ptr) {
        std::unique_lock lock(_registry_mutex);
        auto it = _registry.find(ptr->id());
        if (it != _registry.end()) {
            return it->second;
        }
        _registry[ptr->id()] = ptr;
        return ptr;
    }

}  // namespace tally
