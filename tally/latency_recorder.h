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

#include <tally/gauge.h>
#include <tally/impl/reducer.h>
#include <tally/impl/percentile.h>

namespace tally {
    namespace detail {

        class Percentile;

        typedef Window<AverageGauge, SERIES_IN_SECOND> RecorderWindow;
        typedef Window<MaxerGauge<int64_t>, SERIES_IN_SECOND> MaxWindow;
        typedef Window<Percentile, SERIES_IN_SECOND> PercentileWindow;

        // NOTE: Always use int64_t in the interfaces no matter what the impl. is.

        class CDF : public Variable {
        public:
            explicit CDF(PercentileWindow *w);

            ~CDF();

            void describe(std::ostream &os, bool quote_string) const override;
            turbo::Status describe_series(std::ostream& os, const SeriesOptions& options) const override;
        private:
            PercentileWindow *_w;
        };

        // For mimic constructor inheritance.
        class LatencyRecorderBase {
        public:
            explicit LatencyRecorderBase(time_t window_size);

            time_t window_size() const { return _latency_window.window_size(); }

            // Get |ratio|-ile latency in recent |window_size| seconds
            // E.g. 0.99 means 99%-ile
            int64_t latency_percentile(double ratio) const;

        protected:
            AverageGauge _latency;
            MaxerGauge<int64_t> _max_latency;
            Percentile _latency_percentile;

            RecorderWindow _latency_window;
            MaxWindow _max_latency_window;
            FuncGauge<int64_t> _count;
            FuncGauge<int64_t> _qps;
            PercentileWindow _latency_percentile_window;
            FuncGauge<int64_t> _latency_p1;
            FuncGauge<int64_t> _latency_p2;
            FuncGauge<int64_t> _latency_p3;
            FuncGauge<int64_t> _latency_999;  // 99.9%
            FuncGauge<int64_t> _latency_9999; // 99.99%
            CDF _latency_cdf;
            PassiveStatus<Vector<int64_t, 4> > _latency_percentiles;
        };
    } // namespace detail

    // Specialized structure to record latency.
    // It's not a Variable, but it contains multiple tally inside.
    class LatencyRecorder : public detail::LatencyRecorderBase {
        typedef detail::LatencyRecorderBase Base;
    public:
        LatencyRecorder() : Base(-1) {}

        explicit LatencyRecorder(time_t window_size) : Base(window_size) {}

        explicit LatencyRecorder(const std::string_view &prefix, std::string_view help, Scope *scope = ScopeInstance::instance()->get_default().get()) : Base(-1) {
            auto rs = expose(prefix, help, scope);
            if (!rs.ok()) {
                KLOG_IF(FATAL, turbo::get_flag(FLAGS_tally_crash_on_expose_fail))
                                << "expose LatencyRecorder failed: " << prefix << "to scope" << scope->id();
                KLOG(WARNING) << "expose LatencyRecorder failed: " << prefix << "to scope" << scope->id();
            }
        }

        LatencyRecorder(const std::string_view &prefix, std::string_view help, Scope *scope,
                        time_t window_size) : Base(window_size) {
            auto rs = expose(prefix, help, scope);
            if (!rs.ok()) {
                KLOG_IF(FATAL, turbo::get_flag(FLAGS_tally_crash_on_expose_fail))
                                << "expose LatencyRecorder failed: " << prefix << "to scope" << scope->id();
                KLOG(WARNING) << "expose LatencyRecorder failed: " << prefix << "to scope" << scope->id();
            }
        }

        ~LatencyRecorder() { hide(); }

        // Record the latency.
        LatencyRecorder &operator<<(int64_t latency);

        // Expose all internal variables using `prefix' as prefix.
        // Returns 0 on success, -1 otherwise.
        // Example:
        //   LatencyRecorder rec;
        //   rec.expose("foo_bar_write");     // foo_bar_write_latency
        //                                    // foo_bar_write_max_latency
        //                                    // foo_bar_write_count
        //                                    // foo_bar_write_qps
        //   rec.expose("foo_bar", "read");   // foo_bar_read_latency
        //                                    // foo_bar_read_max_latency
        //                                    // foo_bar_read_count
        //                                    // foo_bar_read_qps

        turbo::Status expose(std::string_view prefix, std::string_view help, Scope *scope);

        // Hide all internal variables, called in dtor as well.
        void hide();

        // Get the average latency in recent |window_size| seconds
        // If |window_size| is absent, use the window_size to ctor.
        int64_t latency(time_t window_size) const { return _latency_window.get_value(window_size).get_average_int(); }

        int64_t latency() const { return _latency_window.get_value().get_average_int(); }

        // Get p1/p2/p3/99.9-ile latencies in recent window_size-to-ctor seconds.
        Vector<int64_t, 4> latency_percentiles() const;

        // Get the max latency in recent window_size-to-ctor seconds.
        int64_t max_latency() const { return _max_latency_window.get_value(); }

        // Get the total number of recorded latencies.
        int64_t count() const { return _latency.get_value().num; }

        // Get qps in recent |window_size| seconds. The `q' means latencies
        // recorded by operator<<().
        // If |window_size| is absent, use the window_size to ctor.
        int64_t qps(time_t window_size) const;

        int64_t qps() const { return _qps.get_value(); }

        // Get name of a sub-tally.
        const std::string &latency_name() const { return _latency_window.name(); }

        const std::string &latency_percentiles_name() const { return _latency_percentiles.name(); }

        const std::string &latency_cdf_name() const { return _latency_cdf.name(); }

        const std::string &max_latency_name() const { return _max_latency_window.name(); }

        const std::string &count_name() const { return _count.name(); }

        const std::string &qps_name() const { return _qps.name(); }
    };

    std::ostream &operator<<(std::ostream &os, const LatencyRecorder &);

}  // namespace tally
