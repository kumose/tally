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

#include <tally/reporters/prometheus_stats_reporter.h>
#include <turbo/log/logging.h>

namespace tally {

    namespace {
        static turbo::Time zo;
        // Write a double as a string, with proper formatting for infinity and NaN
        void WriteValue(std::ostream &out, double value) {
            if (std::isnan(value)) {
                out << "Nan";
            } else if (std::isinf(value)) {
                out << (value < 0 ? "-Inf" : "+Inf");
            } else {
                out << value;
            }
        }

        void WriteValue(std::ostream &out, const std::string &value) {
            for (auto c: value) {
                switch (c) {
                    case '\n':
                        out << '\\' << 'n';
                        break;

                    case '\\':
                        out << '\\' << c;
                        break;

                    case '"':
                        out << '\\' << c;
                        break;

                    default:
                        out << c;
                        break;
                }
            }
        }

        // Write a line header: metric name and labels
        template<typename T = std::string>
        void
        WriteHead(std::ostream &out, std::string_view name, const turbo::flat_hash_map<std::string, std::string> &tags,
                  const std::string &suffix = "",
                  const std::string &extraLabelName = "",
                  const T &extraLabelValue = T()) {
            out << name << suffix;
            if (tags.empty() || !extraLabelName.empty()) {
                out << "{";
                const char *prefix = "";

                for (auto &lp: tags) {
                    out << prefix << lp.first << "=\"";
                    WriteValue(out, lp.second);
                    out << "\"";
                    prefix = ",";
                }
                if (!extraLabelName.empty()) {
                    out << prefix << extraLabelName << "=\"";
                    WriteValue(out, extraLabelValue);
                    out << "\"";
                }
                out << "}";
            }
            out << " ";
        }

        // Write a line trailer: timestamp
        void WriteTail(std::ostream &out, const turbo::Time &stamp) {
            auto mls = turbo::Time::to_milliseconds(stamp);
            if (mls != 0) {
                out << " " << mls;
            }
            out << "\n";
        }

    }  // namespace

    void PrometheusStatsReporter::report_counter(
            std::string_view name,
            std::string_view help,
            const turbo::flat_hash_map<std::string, std::string> &tags, const Variable* v, const turbo::Time &stamp) {
        state.counter_count++;
        MetricSample sample = v->get_metric(stamp);
        auto value = std::get<double>(sample.value);
        if (!help.empty()) {
            _os << "# HELP " << name << " " << help << "\n";
        }
        _os << "# TYPE " << name << " counter\n";
        WriteHead(_os, name, tags);
        WriteValue(_os, value);
        WriteTail(_os, sample.timestamp);
    }

    void PrometheusStatsReporter::report_gauge(
            std::string_view name,
            std::string_view help,
            const turbo::flat_hash_map<std::string, std::string> &tags, const Variable* v, const turbo::Time &stamp) {
        state.gauge_count++;
        MetricSample sample = v->get_metric(stamp);
        auto value = std::get<double>(sample.value);
        if (!help.empty()) {
            _os << "# HELP " << name << " " << help << "\n";
        }
        _os << "# TYPE " << name << " gauge\n";
        WriteHead(_os, name, tags);
        WriteValue(_os, value);
        WriteTail(_os, sample.timestamp);
    }

    void PrometheusStatsReporter::report_histogram(
            std::string_view name,
            std::string_view help,
            const turbo::flat_hash_map<std::string, std::string> &tags,
            const Variable* v, const turbo::Time &stamp) {
        state.hist_count++;
        MetricSample sample = v->get_metric(stamp);
        auto hist = std::get<HistogramSample>(sample.value);
        if (!help.empty()) {
            _os << "# HELP " << name << " " << help << "\n";
        }
        _os << "# TYPE " << name << " histogram\n";
        WriteHead(_os, name, tags, "_sum");
        WriteValue(_os, hist.sample_sum);
        WriteTail(_os, zo);

        double last = -std::numeric_limits<double>::infinity();
        for (auto &b: hist.buckets) {
            WriteHead(_os, name, tags, "_bucket", "le", b.upper_bound);
            last = b.upper_bound;
            _os << b.value;
            WriteTail(_os, zo);
        }

        if (last != std::numeric_limits<double>::infinity() && last != std::numeric_limits<double>::max()) {
            WriteHead(_os, name, tags, "_bucket", "le", "+Inf");
        }
        WriteHead(_os, name, tags, "_count");
        _os << hist.sample_count;
        WriteTail(_os, sample.timestamp);
    }

    void PrometheusStatsReporter::report_variable(
            const Variable *var, const turbo::Time &stamp) {
        state.total++;
        if (!var->type().is_metric()) {
            state.no_metric_count++;
            return;
        }
        auto &name = var->full_name();
        auto &help = var->help();
        auto &tags = var->tags();
        MetricSample ms = var->get_metric(stamp);
        if (var->type().is_gauge()) {
            report_gauge(name, help, tags, var, stamp);
        } else if (var->type().is_counter()) {
            report_counter(name, help, tags, var, stamp);
        }
        if (var->type().is_histogram()) {
            report_histogram(name, help, tags, var, stamp);
        }
    }

}  // namespace tally
