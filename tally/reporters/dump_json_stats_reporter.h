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
#include <sstream>
#include <tally/stats_reporter.h>
#include <nlohmann/json.hpp>

namespace tally {

    class DumpJsonStatsReporter : public StatsReporter {
    public:
        DumpJsonStatsReporter() {
            set_name("dump_json");
            set_help("dump_json variable text reporter");
        }

        void flush() override;

        void report_variable(
                const Variable *var, const turbo::Time &stamp) override;

        void describe(std::ostream &os) const override {
            os << "name: " << _name << "\n";
            os << "help: " << _help << "\n";
            os << "collect:\n";
            os << "total: " << state.total << "\n";
            os << "gauge: " << state.gauge_count << "\n";
            os << "counter: " << state.counter_count << "\n";
            os << "histogram: " << state.hist_count << "\n";
            os << "not metric: " << state.no_metric_count << "\n";
            os << "filter off: " << state.discard_count << "\n";
        }

        const std::vector<std::string> & data() const {
            return _dumped;
        }

        using StatsReporter::describe;
    private:
        static void report_counter(const Variable *value,  const turbo::Time &stamp, nlohmann::ordered_json &out);

        static void report_gauge(const Variable *v,  const turbo::Time &stamp, nlohmann::ordered_json &out);

        static void report_histogram(const Variable *v,  const turbo::Time &stamp, nlohmann::ordered_json &out);

        static void report_flag(const Variable *v,  const turbo::Time &stamp, nlohmann::ordered_json &out);

    private:
        std::vector<std::string> _dumped;
    };

}  // namespace tally
