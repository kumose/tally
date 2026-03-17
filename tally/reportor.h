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

#include <turbo/container/flat_hash_map.h>
#include <tally/stats_reporter.h>
#include <memory>
#include <shared_mutex>
#include <turbo/utility/status.h>
#include <nlohmann/json.hpp>
#include <tally/reporters/json_stats_reporter.h>

namespace tally {

    class Reporter {
    public:

        static Reporter &instance() {
            static Reporter ins;
            return ins;
        }

    public:
        static turbo::Status register_reporter(const std::shared_ptr<StatsReporter> &r);

        static bool has_reporter(std::string_view name);

        static void remove_reporter(std::string_view name);

        static size_t reporter_size(bool exclude_builtin = false);

        static void list_reporter(std::vector<std::string> &reporter, bool exclude_builtin = false);

        static void list_reporter(std::vector<std::shared_ptr<StatsReporter>> &reporter, bool exclude_builtin = false);

        static std::shared_ptr<StatsReporter> get_reporter(std::string_view name);


        static std::string get_prometheus_reporting(ReportOptions *options = nullptr);

        static void get_prometheus_reporting(std::ostream &os, ReportOptions *options = nullptr);

        static std::string get_json_reporting();

        static nlohmann::ordered_json get_json_reporting_json_format();

        static void get_json_reporting(std::ostream &os);

        static void run_reporter(const std::shared_ptr<StatsReporter> &r);

        static void run_all_reporter(bool exclude_builtin = false);

    private:
        Reporter();

    private:
        std::shared_mutex _mutex;
        turbo::flat_hash_map<std::string, std::shared_ptr<StatsReporter>> _reporters;
    };
}  // namespace tally
