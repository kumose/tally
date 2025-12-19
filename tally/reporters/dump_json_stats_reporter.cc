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

#include <tally/reporters/dump_json_stats_reporter.h>
#include <turbo/log/logging.h>
#include <tally/config.h>

namespace tally {

    void DumpJsonStatsReporter::flush() {
        //state = ReportState{};
    }

    void DumpJsonStatsReporter::report_counter(const Variable *v,  const turbo::Time &stamp, nlohmann::ordered_json &out) {
        try {
            MetricSample sample = v->get_metric(stamp);
            auto value = std::get<double>(sample.value);
            out["value"] = value;
        } catch (const std::exception &e) {
            KLOG(ERROR) << "bad type: " << e.what();
        }
    }

    void DumpJsonStatsReporter::report_gauge(const Variable *v,  const turbo::Time &stamp, nlohmann::ordered_json &out) {
        try {
            MetricSample sample = v->get_metric(stamp);
            auto value = std::get<double>(sample.value);
            out["value"] = value;
        } catch (const std::exception &e) {
            KLOG(ERROR) << "bad type: " << e.what();
        }
    }

    void DumpJsonStatsReporter::report_histogram(const Variable *v,  const turbo::Time &stamp, nlohmann::ordered_json &out) {
        try {

            MetricSample sample = v->get_metric(stamp);
            auto hist = std::get<HistogramSample>(sample.value);
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
            out["value"] = std::move(value);
        } catch (const std::exception &e) {
            KLOG(ERROR) << "bad type: " << e.what();
        }

    }

    void DumpJsonStatsReporter::report_flag(const Variable *v,  const turbo::Time &stamp, nlohmann::ordered_json &out) {
        std::any an_value;
        v->get_value(&an_value);
        if(!an_value.has_value()) {
            return;
        }
        auto value = std::any_cast<FlagSample>(an_value);
        nlohmann::ordered_json flag_json;
        flag_json["default_value"] = value.default_value;
        flag_json["current_value"] = value.current_value;
        flag_json["support_update"] = value.support_update;
        out["value"] = flag_json;
    }

    void DumpJsonStatsReporter::report_variable(
            const Variable *var, const turbo::Time &stamp) {
        ++state.total;
        nlohmann::ordered_json obj;
        auto &full_name = var->full_name();
        auto &name = var->full_name();
        auto &prefix = var->prefix();
        auto &tags = var->tags();
        auto &help = var->help();
        auto t = var->type();
        std::string vtype;
        if (t.is_empty()) {
            state.discard_count++;
            return;
        } else if(t.is_flag()) {
            vtype = "flag";
            report_flag(var, stamp, obj);
        } else if (t.is_counter()) {
            state.counter_count++;
            vtype = "counter";
            report_counter(var, stamp, obj);
        } else if (t.is_histogram()) {
            state.hist_count++;
            vtype = "is_histogram";
            report_histogram(var, stamp, obj);
        } else if (t.is_gauge()) {
            state.gauge_count++;
            vtype = "gauge";
            report_gauge(var, stamp, obj);
        } else {
            vtype = "variable";
            state.no_metric_count++;
            obj["value"] = var->get_description();
        }
        obj["name"] = name;
        obj["full_name"] = full_name;
        obj["prefix"] = prefix;
        if (!help.empty()) {
            obj["help"] = help;
        } else {
            obj["help"] = "help";
        }
        obj["type"] = vtype;
        obj["timestamp_ms"] = turbo::Time::to_milliseconds(stamp);
        auto dt = turbo::Time::format(stamp, turbo::get_flag(FLAGS_tally_dump_local) ? turbo::TimeZone::local() : turbo::TimeZone::utc());
        obj["date"] = dt;
        nlohmann::ordered_json js_tags;
        for (auto &it: tags) {
            js_tags[it.first] = it.second;
        }
        obj["tags"] = std::move(js_tags);
        _dumped.push_back(obj.dump());
    }

}  // namespace tally
