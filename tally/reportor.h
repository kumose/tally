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
