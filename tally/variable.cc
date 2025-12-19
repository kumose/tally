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

#include <tally/variable.h>
#include <tally/scope.h>
#include <turbo/log/logging.h>
#include <tally/utility/normalize_name.h>
#include <shared_mutex>
#include <tally/stats_reporter.h>

namespace tally {


    const size_t SUB_MAP_COUNT = 32;  // must be power of 2
    static_assert(!(SUB_MAP_COUNT & (SUB_MAP_COUNT - 1)), "must be power of 2");

    struct VarMaps {
        turbo::flat_hash_map<std::string, Variable *> variable_maps[SUB_MAP_COUNT];
        std::shared_mutex variable_locks[SUB_MAP_COUNT];
    };

    static VarMaps &get_variable_maps() {
        static VarMaps ins;
        return ins;
    }

    inline size_t sub_map_index(std::string_view str) {
        if (str.empty()) {
            return 0;
        }
        size_t h = 0;
        for (auto it: str) {
            h = h * 5 + it;
        }
        return h & (SUB_MAP_COUNT - 1);
    }

    Variable::~Variable() {
        // not true
        KCHECK(!hide()) << "Subclass of Variable MUST call hide() manually in their"
                          " dtors to avoid displaying a variable that is just destructing";
    }

    bool Variable::hide() {
        if (!_exposed) {
            return false;
        }
        auto inx = sub_map_index(_full_name);
        auto &var_map = get_variable_maps().variable_maps[inx];
        auto &var_lock = get_variable_maps().variable_locks[inx];
        std::unique_lock lk(var_lock);
        auto r = var_map.erase(_full_name);
        reset();
        return r > 0;
    }

    const turbo::flat_hash_map<std::string, std::string> &Variable::tags() const {
        auto scope = _scope;
        if (scope) {
            return scope->tags();
        }
        static turbo::flat_hash_map<std::string, std::string> _empty;
        return _empty;
    }

    const std::string &Variable::prefix() const {
        auto scope = _scope;
        if (scope) {
            return scope->prefix();
        }
        static std::string emp;
        return emp;
    }

    void Variable::reset() {
        _name.clear();
        _full_name.clear();
        _help.clear();
        _exposed = false;
        _scope = nullptr;
        /// do not reset this one
        ///_attr = {VariableAttr::empty_attr()};
    }

    turbo::Status Variable::expose_impl(std::string_view name, std::string_view help, turbo::Nonnull<Scope *> scope) {
        if (name.empty()) {
            return turbo::invalid_argument_error("name is empty");
        }
        //
        hide();

        to_underscored_name(&_name, name);
        _full_name = scope->fully_qualified_name(_name);

        auto inx = sub_map_index(_full_name);
        auto &var_map = get_variable_maps().variable_maps[inx];
        auto &var_lock = get_variable_maps().variable_locks[inx];
        std::unique_lock lk(var_lock);
        auto [it, suc] = var_map.insert({_full_name, this});
        if (!suc) {
            std::stringstream ss;
            ss << "\nalready expose variable:\n";
            it->second->exposed_meta(ss);
            ss << "this expose variable:\n";
            ss << "\tname: " << _name << "\n";
            ss << "\tscope: " << scope->id() << "\n";
            ss << "\tsep: " << scope->separator() << "\n";
            ss << "\tfull_name: " << _full_name << "\n\n";
            reset();
            return turbo::already_exists_error(ss.str());
        }
        _help = help;
        _scope = scope;
        _exposed = true;
        return turbo::OkStatus();
    }

    void Variable::exposed_meta(std::ostream &ss) {
        ss << "\tname: " << _name << "\n";
        ss << "\tscope: " << _scope->id() << "\n";
        ss << "\tsep: " << _scope->separator() << "\n";
        ss << "\tfull_name: " << _full_name << "\n";
    }

    turbo::Status Variable::expose(std::string_view name, std::string_view help, Scope *scope) {
        if (scope == nullptr) {
            scope = ScopeInstance::instance()->get_default().get();
        }
        return expose_impl(name, help, scope);
    }

    void Variable::set_type(const VariableType &t) {
        _attr.type = t;
    }

    void Variable::get_value(std::any *value) const {
        std::ostringstream os;
        describe(os, false);
        *value = os.str();
    }

    turbo::Status
    Variable::describe_series(nlohmann::ordered_json &result) const {
        std::stringstream ss;
        SeriesOptions opt;
        auto rs =describe_series(ss,opt);
        if(!rs.ok()) {
            return rs;
        }
        try {
            result = nlohmann::ordered_json::parse(ss.str());
        } catch (const std::exception&e) {
            return turbo::unknown_error(e.what());
        }
        return turbo::OkStatus();
    }

    std::string Variable::get_description() const {
        std::ostringstream os;
        describe(os, false);
        return os.str();
    }

    void Variable::list_exposed(std::vector<std::string> *names,
                                const VariableFilter *filter) {
        names->clear();
        for (size_t i = 0; i < SUB_MAP_COUNT; i++) {
            std::shared_lock lk(get_variable_maps().variable_locks[i]);
            auto &var_map = get_variable_maps().variable_maps[i];
            for (auto &it: var_map) {
                if (!filter || (filter && filter->is_member(it.second))) {
                    names->push_back(it.second->full_name());
                }
            }
        }
    }

    size_t Variable::count_exposed(const VariableFilter *filter) {
        size_t cnt = 0;
        for (size_t i = 0; i < SUB_MAP_COUNT; i++) {
            std::shared_lock lk(get_variable_maps().variable_locks[i]);
            auto &var_map = get_variable_maps().variable_maps[i];
            if (!filter) {
                cnt += var_map.size();
            } else {
                for (auto &it: var_map) {
                    if (filter->is_member(it.second)) {
                        cnt++;
                    }
                }
            }
        }
        return cnt;
    }

    turbo::Status Variable::describe_exposed(std::string_view name, std::ostream &os, bool quote_string) {
        auto inx = sub_map_index(name);
        auto &var_map = get_variable_maps().variable_maps[inx];
        auto &var_lock = get_variable_maps().variable_locks[inx];
        std::shared_lock lk(var_lock);
        auto it = var_map.find(name);
        if (it == var_map.end()) {
            return turbo::not_found_error("");
        }
        it->second->describe(os, quote_string);
        return turbo::OkStatus();
    }

    std::string Variable::describe_exposed(std::string_view name, bool quote_string) {
        std::stringstream ss;
        auto rs = describe_exposed(name, ss, quote_string);
        if (rs.ok()) {
            return ss.str();
        }
        return "";
    }

    turbo::Status
    Variable::describe_series_exposed(const std::string &name, std::ostream &os, const SeriesOptions &options) {
        auto inx = sub_map_index(name);
        auto &var_map = get_variable_maps().variable_maps[inx];
        auto &var_lock = get_variable_maps().variable_locks[inx];
        std::shared_lock lk(var_lock);
        auto it = var_map.find(name);
        if (it == var_map.end()) {
            return turbo::not_found_error("");
        }
        return it->second->describe_series(os,options);
    }

    turbo::Status Variable::describe_series_exposed(const std::string &name, nlohmann::ordered_json &result) {
        SeriesOptions opt;
        std::stringstream  ss;
        auto rs = describe_series_exposed(name,ss, opt);
        if(!rs.ok()) {
            return rs;
        }
        try {
            result = nlohmann::ordered_json::parse(ss.str());
        } catch (const std::exception&e) {
            return turbo::unknown_error(e.what());
        }
        return turbo::OkStatus();
    }

    void Variable::report(turbo::Nonnull<StatsReporter *> reporter, const turbo::Time &stamp) {
        for (size_t i = 0; i < SUB_MAP_COUNT; i++) {
            std::shared_lock lk(get_variable_maps().variable_locks[i]);
            auto &var_map = get_variable_maps().variable_maps[i];
            for (auto &it: var_map) {
                reporter->report_variable(it.second, stamp);
            }
        }
    }
}  // namespace tally
