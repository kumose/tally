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
