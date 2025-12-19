//
// Copyright (C) 2024 Kumo group inc.
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

#pragma once

#include <string>                       // std::string
#include <tally/variable.h>
#include <tally/stats_reporter.h>
#include <turbo/strings/numbers.h>
#include <turbo/flags/flag.h>
#include <turbo/log/logging.h>

namespace tally {

    // Expose important flags as var so that they're monitored.
    class FlagStatus : public Variable {
    public:
        FlagStatus() : Variable(VariableAttr::flag_attr()) {}

        FlagStatus(std::string_view name, std::string_view help, Scope *scope = nullptr);

        // Calling hide() in dtor manually is a MUST required by Variable.
        ~FlagStatus() { hide(); }

        void describe(std::ostream &os, bool quote_string) const override;

        // Get value of the turbo flag.
        // We don't bother making the return type generic. This function
        // is just for consistency with other classes.
        std::string get_value() const;

        void get_value(std::any *value) const;

        std::string get_default_value() const;

        // Set the turbo with a new value.
        // Returns true on success.
        bool set_value(std::string_view value);

        bool support_update() const;

        // name of the turbo flag.
        const std::string &flag_name() const {
            return _flag_name.empty() ? name() : _flag_name;
        }

        bool valid() const { return _st.ok(); }

    private:
        turbo::Status
        expose_impl(std::string_view name, std::string_view help, turbo::Nonnull<Scope *> scope) override;

    private:
        std::string _flag_name;
        turbo::Status _st;
    };

}  // namespace tally
