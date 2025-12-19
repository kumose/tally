//
// Copyright (C) 2024 EA group inc.
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
//


#include <cstdlib>
#include <turbo/flags/reflection.h>
#include <tally/flag.h>
#include <tally/scope.h>

namespace tally {

    FlagStatus::FlagStatus(std::string_view name, std::string_view help, Scope *scope) : Variable(
            VariableAttr::flag_attr()) {
        if (scope == nullptr) {
            scope = ScopeInstance::instance()->get_flag_scope().get();
        }
        _st = expose(name, help, scope);
        if (_st.ok()) {
            _flag_name = name;
        }
    }

    turbo::Status
    FlagStatus::expose_impl(std::string_view name, std::string_view help, turbo::Nonnull<Scope *> scope) {
        auto flag_info = turbo::find_command_line_flag(name);
        if (!flag_info) {
            return turbo::not_found_error("not found flag: %s", name);
        }
        auto rc = Variable::expose_impl(name, help, scope);
        if (!rc.ok()) {
            return rc;
        }
        if (flag_info->is_of_type<int32_t>() ||
            flag_info->is_of_type<uint32_t>() ||
            flag_info->is_of_type<int64_t>() ||
            flag_info->is_of_type<uint64_t>() ||
            flag_info->is_of_type<float>() ||
            flag_info->is_of_type<double>()
                ) {
            auto t = VariableType::flag_type();
            t |= VariableType::gauge_type();
            set_type(t);
        }
        _flag_name = name;
        return turbo::OkStatus();
    }

    bool FlagStatus::support_update() const {
        if (flag_name().empty()) {
            return false;
        }
        auto flag_info = turbo::find_command_line_flag(name());
        if (!flag_info) {
            return false;
        }
        return flag_info->has_user_validator();
    }

    void FlagStatus::describe(std::ostream &os, bool quote_string) const {
        if (flag_name().empty()) {
            return;
        }
        auto flag_info = turbo::find_command_line_flag(flag_name());
        if (!flag_info) {
            if (quote_string) {
                os << '"';
            }
            os << "Unknown flag=" << flag_name();
            if (quote_string) {
                os << '"';
            }
        } else {
            if (quote_string && flag_info->is_of_type<std::string>()) {
                os << '"' << flag_info->current_value() << '"';
            } else {
                os << flag_info->current_value();
            }
        }
    }


    std::string FlagStatus::get_value() const {
        if (flag_name().empty()) {
            return "";
        }
        std::string str;
        auto flag_info = turbo::find_command_line_flag(flag_name());
        if (!flag_info) {
            return "Unknown flag=" + flag_name();
        }
        return flag_info->current_value();
    }

    void FlagStatus::get_value(std::any *value) const {
        FlagSample smp;
        value->reset();
        if (flag_name().empty()) {
            return;
        }
        std::string str;
        auto flag_info = turbo::find_command_line_flag(flag_name());
        if (!flag_info) {
            return;
        }
        smp.name = _flag_name;
        smp.help = flag_info->help();
        smp.current_value = flag_info->current_value();
        smp.default_value = flag_info->default_value();
        smp.support_update = flag_info->has_user_validator();
        *value = smp;
    }

    std::string FlagStatus::get_default_value() const {
        if (flag_name().empty()) {
            return "";
        }
        std::string str;
        auto flag_info = turbo::find_command_line_flag(flag_name());
        if (!flag_info) {
            return "Unknown flag=" + flag_name();
        }
        return flag_info->default_value();
    }

    bool FlagStatus::set_value(std::string_view value) {
        if (flag_name().empty()) {
            return false;
        }
        auto flag_info = turbo::find_command_line_flag(flag_name());
        if (!flag_info) {
            return false;
        }
        if (!flag_info->has_user_validator()) {
            return false;
        }
        std::string err;
        if (!flag_info->user_validate(value, &err)) {
            return false;
        }
        return flag_info->parse_from(value, &err);
    }

}  // namespace tally
