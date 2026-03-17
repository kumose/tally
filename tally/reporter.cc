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


#include <tally/reportor.h>
#include <memory>
#include <shared_mutex>
#include <tally/config.h>
#include <tally/scope.h>
#include <tally/reporters/prometheus_stats_reporter.h>

namespace tally {

    turbo::Status Reporter::register_reporter(const std::shared_ptr<StatsReporter> &r) {
        if (r->name().empty()) {
            return turbo::data_loss_error("StatsReporter need set name before schedule");
        }
        auto &ins = instance();
        std::unique_lock lk(ins._mutex);
        auto it = ins._reporters.find(r->name());
        if (it != ins._reporters.end()) {
            return turbo::already_exists_error("already scheduled %s", r->name().c_str());
        }
        // verify
        ins._reporters.insert({r->name(), r});
        return turbo::OkStatus();
    }

    bool Reporter::has_reporter(std::string_view name) {
        if (name.empty()) {
            return false;
        }
        auto &ins = instance();
        std::shared_lock lk(ins._mutex);
        auto it = ins._reporters.find(name);
        return it != ins._reporters.end();
    }

    void Reporter::remove_reporter(std::string_view name) {
        if (name.empty()) {
            return;
        }
        auto &ins = instance();
        std::unique_lock lk(ins._mutex);
        auto it = ins._reporters.find(name);
        if (it == ins._reporters.end()) {
            return;
        }
        ins._reporters.erase(it);
    }

    std::shared_ptr<StatsReporter> Reporter::get_reporter(std::string_view name) {
        if (name.empty()) {
            return nullptr;
        }
        auto &ins = instance();
        std::unique_lock lk(ins._mutex);
        auto it = ins._reporters.find(name);
        if (it == ins._reporters.end()) {
            return nullptr;
        }
        return it->second;
    }


    size_t Reporter::reporter_size(bool exclude_builtin) {
        auto &ins = instance();
        std::shared_lock lk(ins._mutex);
        if(!exclude_builtin) {
            return ins._reporters.size() + 2;
        } else {
            return ins._reporters.size();
        }
    }

    void Reporter::list_reporter(std::vector<std::string> &reporter, bool exclude_builtin) {
        auto &ins = instance();
        reporter.clear();
        std::shared_lock lk(ins._mutex);
        reporter.reserve(ins._reporters.size());
        for (auto &it: ins._reporters) {
            reporter.emplace_back(it.first);
        }
        if (!exclude_builtin) {
            reporter.emplace_back("json");
            reporter.emplace_back("prometheus");
        }
    }
    void Reporter::list_reporter(std::vector<std::shared_ptr<StatsReporter>> &reporter, bool exclude_builtin) {
        auto &ins = instance();
        reporter.clear();
        std::shared_lock lk(ins._mutex);
        reporter.reserve(ins._reporters.size());
        for (auto &it: ins._reporters) {
            reporter.push_back(it.second);
        }
    }

    std::string Reporter::get_prometheus_reporting(ReportOptions *options) {
        std::stringstream ss;
        get_prometheus_reporting(ss, options);
        return ss.str();
    }

    void Reporter::get_prometheus_reporting(std::ostream &os, ReportOptions *options) {
        PrometheusStatsReporter reporter(os);
        if(options) {
            reporter.set_option(*options);
        }
        Variable::report(&reporter,turbo::Time::current_time());
    }

    std::string Reporter::get_json_reporting() {
        std::stringstream ss;
        get_json_reporting(ss);
        return ss.str();
    }

    void Reporter::get_json_reporting(std::ostream &os) {
        auto r = get_json_reporting_json_format();
        os<<r.dump();
    }

    nlohmann::ordered_json Reporter::get_json_reporting_json_format() {
        nlohmann::ordered_json result;
        JsonStatsReporter reporter(result);
        Variable::report(&reporter,turbo::Time::current_time());
        return result;
    }

    void Reporter::run_reporter(const std::shared_ptr<StatsReporter> &r) {
        auto scopes = ScopeInstance::instance()->list_scopes();
        auto ct = turbo::Time::current_time();
        Variable::report(r.get(), ct);
        r->flush();
    }

    void Reporter::run_all_reporter(bool exclude_builtin) {
        auto &ins = instance();
        std::shared_lock lk(ins._mutex);
        for (auto &it: ins._reporters) {
            run_reporter(it.second);
        }
    }

    Reporter::Reporter() {
    }


}  // namespace tally
