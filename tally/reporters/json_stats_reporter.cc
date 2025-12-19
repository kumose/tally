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

#include <tally/reporters/json_stats_reporter.h>
#include <turbo/log/logging.h>

namespace tally {

    void JsonStatsReporter::init() {
        _os_json.clear();
        _os_json["metric"] = nlohmann::ordered_json::array();
        _os_json["flag"] = nlohmann::ordered_json::array();
        _os_json["variable"] = nlohmann::ordered_json::array();
    }

    void JsonStatsReporter::flush() {
        //state = ReportState{};
    }

    void JsonStatsReporter::report_counter(
            std::string_view name,
            std::string_view help,
            const turbo::flat_hash_map<std::string, std::string> &tags, const Variable *v,  const turbo::Time &stamp) {
        try {
            MetricSample sample = v->get_metric(stamp);
            auto value = std::get<double>(sample.value);
            nlohmann::ordered_json obj;
            obj["name"] = name;
            if (!help.empty()) {
                obj["help"] = help;
            } else {
                obj["help"] = "help";
            }
            obj["type"] = "counter";
            obj["value"] = value;
            obj["timestamp_ms"] = turbo::Time::to_milliseconds(sample.timestamp);
            nlohmann::ordered_json js_tags;
            for (auto &it: tags) {
                js_tags[it.first] = it.second;
            }
            obj["tags"] = std::move(js_tags);
            _os_json["metric"].push_back(std::move(obj));
        } catch (const std::exception &e) {
            KLOG(ERROR) << "bad type: " << e.what();
        }
    }

    void JsonStatsReporter::report_gauge(
            std::string_view name,
            std::string_view help,
            const turbo::flat_hash_map<std::string, std::string> &tags, const Variable *v,  const turbo::Time &stamp) {
        try {
            MetricSample sample = v->get_metric(stamp);
            auto value = std::get<double>(sample.value);
            nlohmann::ordered_json obj;
            obj["name"] = name;
            if (!help.empty()) {
                obj["help"] = help;
            } else {
                obj["help"] = "help";
            }
            obj["type"] = "gauge";
            obj["value"] = value;
            obj["timestamp_ms"] = turbo::Time::to_milliseconds(sample.timestamp);
            nlohmann::ordered_json js_tags;
            for (auto &it: tags) {
                js_tags[it.first] = it.second;
            }
            obj["tags"] = std::move(js_tags);
            _os_json["metric"].push_back(std::move(obj));
        } catch (const std::exception &e) {
            KLOG(ERROR) << "bad type: " << e.what();
        }
    }

    void JsonStatsReporter::report_histogram(
            std::string_view name,
            std::string_view help,
            const turbo::flat_hash_map<std::string, std::string> &tags,
            const Variable* v,  const turbo::Time &stamp) {
        try {

            MetricSample sample = v->get_metric(stamp);
            auto hist = std::get<HistogramSample>(sample.value);
            nlohmann::ordered_json obj;
            obj["name"] = name;
            if (!help.empty()) {
                obj["help"] = help;
            } else {
                obj["help"] = "help";
            }
            obj["type"] = "histogram";
            obj["timestamp_ms"] = turbo::Time::to_milliseconds(sample.timestamp);
            nlohmann::ordered_json value;
            value["sum"] = hist.sample_sum;
            value["count"] = hist.sample_count;
            nlohmann::ordered_json value_bucket = nlohmann::ordered_json::array();
            double last = 0;
            for (auto &b: hist.buckets) {
                nlohmann::ordered_json cell;
                cell["le"] = b.upper_bound;
                cell["value"] = b.value;
                last = b.upper_bound;
                value_bucket.push_back(std::move(cell));
            }

            if (last != std::numeric_limits<double>::infinity() && last != std::numeric_limits<double>::max()) {
                nlohmann::ordered_json cell;
                cell["le"] = "+Inf";
                cell["value"] = hist.sample_count;
                value_bucket.push_back(std::move(cell));
            }

            value["bucket"] = std::move(value_bucket);
            obj["value"] = std::move(value);
            nlohmann::ordered_json js_tags;
            for (auto &it: tags) {
                js_tags[it.first] = it.second;
            }
            obj["tags"] = std::move(js_tags);
            _os_json["metric"].push_back(std::move(obj));
        } catch (const std::exception &e) {
            KLOG(ERROR) << "bad type: " << e.what();
        }

    }

    void JsonStatsReporter::report_flag(
            std::string_view n,
            std::string_view h,
            const turbo::flat_hash_map<std::string, std::string> &tags,
            const Variable* v, const turbo::Time &stamp) {
        std::any an_value;
        v->get_value(&an_value);
        if(!an_value.has_value()) {
            return;
        }
        auto value = std::any_cast<FlagSample>(an_value);
        nlohmann::ordered_json obj;
        obj["full_name"] = n;
        obj["full_help"] = h;
        nlohmann::ordered_json js_tags;
        for (auto &it: tags) {
            js_tags[it.first] = it.second;
        }
        obj["tags"] = std::move(js_tags);
        obj["type"] = "flag";
        obj["timestamp_ms"] = turbo::Time::to_milliseconds(stamp);
        obj["type"] = "flag";
        obj["is_gauge"] = v->type().is_gauge();

        nlohmann::ordered_json flag_json;
        flag_json["name"] = value.name;
        if (!value.help.empty()) {
            flag_json["help"] = value.help;
        } else {
            flag_json["help"] = "help";
        }
        flag_json["default_value"] = value.default_value;
        flag_json["current_value"] = value.current_value;
        flag_json["support_update"] = value.support_update;
        obj["flag"] = flag_json;
        _os_json["flag"].push_back(std::move(obj));
    }

    void JsonStatsReporter::report_variable(
            const Variable *var, const turbo::Time &stamp) {
        ++state.total;
        auto &full_name = var->full_name();
        auto &name = var->full_name();
        auto &prefix = var->prefix();
        auto &tags = var->tags();
        auto &help = var->help();
        auto t = var->type();
        if (t.is_empty()) {
            state.discard_count++;
            return;
        } else if (t.is_flag()) {
            report_flag(full_name, help, tags, var, stamp);
        } else if (t.is_counter()) {
            state.counter_count++;
            report_counter(full_name, help, tags, var, stamp);
        } else if (t.is_histogram()) {
            state.hist_count++;
            report_histogram(full_name, help, tags, var, stamp);
        } else if (t.is_gauge()) {
            state.gauge_count++;
            report_gauge(full_name, help, tags, var, stamp);
        } else {
            nlohmann::ordered_json obj;
            obj["name"] = name;
            obj["full_name"] = full_name;
            obj["prefix"] = prefix;
            if (!help.empty()) {
                obj["help"] = help;
            } else {
                obj["help"] = "help";
            }
            obj["type"] = "variable";
            obj["value"] = var->get_description();
            obj["timestamp_ms"] = turbo::Time::to_milliseconds(stamp);
            nlohmann::ordered_json js_tags;
            for (auto &it: tags) {
                js_tags[it.first] = it.second;
            }
            obj["tags"] = std::move(js_tags);
            _os_json["variable"].push_back(std::move(obj));
        }
    }

}  // namespace tally
