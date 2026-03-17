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
