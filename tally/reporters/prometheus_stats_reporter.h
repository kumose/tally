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

namespace tally {

    class PrometheusStatsReporter : public StatsReporter {
    public:
        PrometheusStatsReporter(std::ostream &os) : _os(os) {
            set_name("prometheus");
            set_help("prometheus metric text reporter");
        }

        void flush() override {
           //state = ReportState{};
        }

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

        void report_variable(
                const Variable *var, const turbo::Time &stamp) override;

    private:



        void report_counter(std::string_view name,
                            std::string_view help,
                            const turbo::flat_hash_map<std::string, std::string> &tags,
                            const Variable *value, const turbo::Time &stamp);

        void report_gauge(std::string_view name,
                          std::string_view help,
                          const turbo::flat_hash_map<std::string, std::string> &tags,
                          const Variable *value, const turbo::Time &stamp);

        void report_histogram(
                std::string_view name,
                std::string_view help,
                const turbo::flat_hash_map<std::string, std::string> &tags,
                const Variable *value, const turbo::Time &stamp);
    private:
        std::ostream &_os;
    };

}  // namespace tally
