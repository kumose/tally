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

    class JsonStatsReporter : public StatsReporter {
    public:
        JsonStatsReporter(nlohmann::ordered_json &json) : _os_json(json) {
            init();
            set_name("json");
            set_help("json variable text reporter");
        }

        void flush() override;

        void report_variable(
                const Variable* var, const turbo::Time &stamp) override;

        void describe(std::ostream &os) const override {
            os << "name: " << _name << "\n";
            os << "help: " << _help << "\n";
            os<<"collect:\n";
            os<<"total: "<<state.total<<"\n";
            os<<"gauge: "<<state.gauge_count<<"\n";
            os<<"counter: "<<state.counter_count<<"\n";
            os<<"histogram: "<<state.hist_count<<"\n";
            os<<"not metric: "<<state.no_metric_count<<"\n";
            os<<"filter off: "<<state.discard_count<<"\n";
        }

        using StatsReporter::describe;
    private:
        void init();
        void report_counter(std::string_view name,
                            std::string_view help,
                            const turbo::flat_hash_map<std::string, std::string> &tags,
                            const Variable* value, const turbo::Time &stamp);

        void report_gauge(std::string_view name,
                          std::string_view help,
                          const turbo::flat_hash_map<std::string, std::string> &tags,
                          const Variable* value, const turbo::Time &stamp);

        void report_histogram(
                std::string_view name,
                std::string_view help,
                const turbo::flat_hash_map<std::string, std::string> &tags,
                const Variable* value, const turbo::Time &stamp);

        void report_flag(
                std::string_view name,
                std::string_view help,
                const turbo::flat_hash_map<std::string, std::string> &tags,
                const Variable* value, const turbo::Time &stamp);
    private:
        nlohmann::ordered_json &_os_json;
    };

}  // namespace tally
