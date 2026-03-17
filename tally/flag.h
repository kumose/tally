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
