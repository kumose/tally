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

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <turbo/container/flat_hash_map.h>
#include <tally/buckets.h>
#include <tally/variable.h>
#include <tally/utility/wildcard_matcher.h>
#include <tally/utility/task_timer.h>

namespace tally {

    // Options for Variable::dump_exposed().
    class ReportOptions {
    public:
        ReportOptions() = default;

        ~ReportOptions() = default;

        ReportOptions(const ReportOptions &) = default;

        ReportOptions &operator=(const ReportOptions &) = default;

        ReportOptions(ReportOptions &&) = default;

        ReportOptions &operator=(ReportOptions &&) = default;

        bool allow_report(std::string_view name) const {
            if (_w_matcher && !_w_matcher->match(name)) {
                return false;
            }
            if (_b_matcher && _b_matcher->match(name)) {
                return false;
            }
            return true;
        }

        constexpr bool quote_string() const {
            return _quote_string;
        }


        char question_mark() const {
            return _question_mark;
        }

        int32_t interval_ms() {
            return _interval_ms;
        }

        ReportOptions &quote_string(bool flag) {
            _quote_string = flag;
            return *this;
        }

        ReportOptions &interval_ms(int32_t ms) {
            _interval_ms = ms;
            return *this;
        }

        ReportOptions &build_filter(std::string_view white, std::string_view black, char question_mark = '?') {
            _question_mark = question_mark;
            _white_wildcards = white;
            _black_wildcards = black;
            if (!white.empty()) {
                _w_matcher = std::make_unique<WildcardMatcher>(_white_wildcards, _question_mark, true);
            }
            if (!black.empty()) {
                _b_matcher = std::make_unique<WildcardMatcher>(_black_wildcards, _question_mark, false);
            }
            return *this;
        }

    protected:
        // If this is true, string-type values will be quoted.
        // only effect on raw text report
        bool _quote_string{true};

        // The ? in wildcards. Wildcards in URL need to use another character
        // because ? is reserved.
        char _question_mark{'?'};

        // Name matched by these wildcards (or exact names) are kept.
        std::string _white_wildcards;

        // Name matched by these wildcards (or exact names) are skipped.
        std::string _black_wildcards;

        // schedule interval
        int32_t _interval_ms{0};

        std::shared_ptr<WildcardMatcher> _w_matcher;
        std::shared_ptr<WildcardMatcher> _b_matcher;
    };

    struct ReportState {
        size_t total{0};
        size_t gauge_count{0};
        size_t counter_count{0};
        size_t hist_count{0};
        size_t no_metric_count{0};
        size_t discard_count{0};
    };

    class StatsReporter {
    public:
        virtual ~StatsReporter() = default;

        virtual void report_variable(
                const Variable *var, const turbo::Time &stamp) = 0;


        virtual void flush() = 0;

        ReportOptions &option() {
            return _opt;
        }

        void set_option(const ReportOptions &opt) {
            _opt = opt;
        }

        const std::string &name() const {
            return _name;
        }

        void set_name(std::string_view name) {
            _name = name;
        }

        const std::string &help() const {
            return _help;
        }

        void set_help(std::string_view hstr) {
            _help = hstr;
        }

        bool allow_scope_report(std::string_view name) const {
            return _opt.allow_report(name);
        }

        virtual bool allow_display(VariableAttr attr) const {
            return true;
        }


        virtual void describe(std::ostream &os) const {
            os << "name: " << _name << "\n";
            os << "help: " << _help << "\n";
        }

        std::string describe() const {
            std::stringstream ss;
            describe(ss);
            return ss.str();
        }
    public:
        ReportState state;
    protected:
        ReportOptions _opt;
        std::string _name;
        std::string _help;
    };

}  // namespace tally
