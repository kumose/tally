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

#include <memory>
#include <tally/latency_recorder.h>
#include <tally/config.h>
#include <turbo/strings/match.h>

namespace tally {

    namespace detail {

        typedef PercentileSamples<1022> CombinedPercentileSamples;

        /// Cumulative Distribution Function
        CDF::CDF(PercentileWindow *w) : Variable(VariableAttr::cdf_attr()), _w(w) {}

        CDF::~CDF() {
            hide();
        }

        void CDF::describe(std::ostream &os, bool) const {
            if (_w == nullptr) {
                return;
            }
            std::unique_ptr<CombinedPercentileSamples> cb(new CombinedPercentileSamples);
            std::vector<GlobalPercentileSamples> buckets;
            _w->get_samples(&buckets);
            for (size_t i = 0; i < buckets.size(); ++i) {
                cb->combine_of(buckets.begin(), buckets.end());
            }
            std::pair<int, int> values[20];
            size_t n = 0;
            for (int i = 1; i < 10; ++i) {
                values[n++] = std::make_pair(i * 10, cb->get_number(i * 0.1));
            }
            for (int i = 91; i < 100; ++i) {
                values[n++] = std::make_pair(i, cb->get_number(i * 0.01));
            }
            values[n++] = std::make_pair(100, cb->get_number(0.999));
            values[n++] = std::make_pair(101, cb->get_number(0.9999));
            KCHECK_EQ(n, TURBO_ARRAYSIZE(values));
            os << "{\"label\":\"cdf\",\"data\":[";
            for (size_t i = 0; i < n; ++i) {
                if (i) {
                    os << ',';
                }
                os << '[' << values[i].first << ',' << values[i].second << ']';
            }
            os << "]}";
        }

        // Return random int value with expectation = `dval'
        static int64_t double_to_random_int(double dval) {
            int64_t ival = static_cast<int64_t>(dval);
            if (dval > ival + turbo::fast_rand_double()) {
                ival += 1;
            }
            return ival;
        }


        turbo::Status CDF::describe_series(
                std::ostream& os, const SeriesOptions& options) const {
            if (_w == nullptr) {
                return turbo::unavailable_error("");
            }
            if (options.test_only) {
                return turbo::OkStatus();
            }
            std::unique_ptr<CombinedPercentileSamples> cb(new CombinedPercentileSamples);
            std::vector<GlobalPercentileSamples> buckets;
            _w->get_samples(&buckets);
            for (size_t i = 0; i < buckets.size(); ++i) {
                cb->combine_of(buckets.begin(), buckets.end());
            }
            std::pair<int, int> values[20];
            size_t n = 0;
            for (int i = 1; i < 10; ++i) {
                values[n++] = std::make_pair(i*10, cb->get_number(i * 0.1));
            }
            for (int i = 91; i < 100; ++i) {
                values[n++] = std::make_pair(i, cb->get_number(i * 0.01));
            }
            values[n++] = std::make_pair(100, cb->get_number(0.999));
            values[n++] = std::make_pair(101, cb->get_number(0.9999));
            KCHECK_EQ(n, TURBO_ARRAYSIZE(values));
            os << "{\"label\":\"cdf\",\"data\":[";
            for (size_t i = 0; i < n; ++i) {
                if (i) {
                    os << ',';
                }
                os << '[' << values[i].first << ',' << values[i].second << ']';
            }
            os << "]}";
            return turbo::OkStatus();
        }


        static int64_t get_window_recorder_qps(void *arg) {
            detail::Sample<Stat> s;
            static_cast<RecorderWindow *>(arg)->get_span(&s);
            // Use floating point to avoid overflow.
            if (s.time_us <= 0) {
                return 0;
            }
            return double_to_random_int(s.data.num * 1000000.0 / s.time_us);
        }

        static int64_t get_recorder_count(void *arg) {
            return static_cast<AverageGauge *>(arg)->get_value().num;
        }

        // Caller is responsible for deleting the return value.
        static CombinedPercentileSamples *combine(PercentileWindow *w) {
            CombinedPercentileSamples *cb = new CombinedPercentileSamples;
            std::vector<GlobalPercentileSamples> buckets;
            w->get_samples(&buckets);
            cb->combine_of(buckets.begin(), buckets.end());
            return cb;
        }

        template<int64_t numerator, int64_t denominator>
        static int64_t get_percetile(LatencyRecorderBase *arg) {
            return arg->latency_percentile(
                    (double) numerator / double(denominator));
        }

        static Vector<int64_t, 4> get_latencies(void *arg) {
            std::unique_ptr<CombinedPercentileSamples> cb(
                    combine((PercentileWindow *) arg));
            // NOTE: We don't show 99.99% since it's often significantly larger than
            // other values and make other curves on the plotted graph small and
            // hard to read.
            Vector<int64_t, 4> result;
            result[0] = cb->get_number(turbo::get_flag(FLAGS_tally_latency_p1) / 100.0);
            result[1] = cb->get_number(turbo::get_flag(FLAGS_tally_latency_p2) / 100.0);
            result[2] = cb->get_number(turbo::get_flag(FLAGS_tally_latency_p3) / 100.0);
            result[3] = cb->get_number(0.999);
            return result;
        }

        LatencyRecorderBase::LatencyRecorderBase(time_t window_size)
                : _max_latency(0), _latency_window(&_latency, window_size),
                  _max_latency_window(&_max_latency, window_size),
                  _count([this]() {
                      return this->_latency.get_value().num;
                  }),
                  _qps([this]() {
                      detail::Sample<Stat> s;
                      this->_latency_window.get_span(&s);
                      // Use floating point to avoid overflow.
                      if (s.time_us <= 0) {
                          return 0l;
                      }
                      return static_cast<int64_t>(double_to_random_int(s.data.num * 1000000.0 / s.time_us));
                  }),
                  _latency_percentile_window(&_latency_percentile, window_size),
                  _latency_p1([this]() {
                      return this->latency_percentile(turbo::get_flag(FLAGS_tally_latency_p1) / 100.0);
                  }),
                  _latency_p2([this]() {
                      return this->latency_percentile(turbo::get_flag(FLAGS_tally_latency_p2) / 100.0);
                  }), _latency_p3([this]() {
                    return this->latency_percentile(turbo::get_flag(FLAGS_tally_latency_p3) / 100.0);
                }), _latency_999([this]() {
                    return get_percetile<999, 1000>(this);
                }),
                  _latency_9999([this]() { return get_percetile<9999, 10000>(this); }),
                  _latency_cdf(&_latency_percentile_window),
                  _latency_percentiles([this]() {
                      std::unique_ptr<CombinedPercentileSamples> cb(
                              combine(&this->_latency_percentile_window));
                      // NOTE: We don't show 99.99% since it's often significantly larger than
                      // other values and make other curves on the plotted graph small and
                      // hard to read.
                      Vector<int64_t, 4> result;
                      result[0] = cb->get_number(turbo::get_flag(FLAGS_tally_latency_p1) / 100.0);
                      result[1] = cb->get_number(turbo::get_flag(FLAGS_tally_latency_p2) / 100.0);
                      result[2] = cb->get_number(turbo::get_flag(FLAGS_tally_latency_p3) / 100.0);
                      result[3] = cb->get_number(0.999);
                      return result;
                  }) {}

        int64_t LatencyRecorderBase::latency_percentile(double ratio) const {
            std::unique_ptr<detail::CombinedPercentileSamples> cb(
                    combine((detail::PercentileWindow *) &_latency_percentile_window));
            return cb->get_number(ratio);
        }

    }  // namespace detail

    Vector<int64_t, 4> LatencyRecorder::latency_percentiles() const {
        // const_cast here is just to adapt parameter type and safe.
        return detail::get_latencies(
                const_cast<detail::PercentileWindow *>(&_latency_percentile_window));
    }

    int64_t LatencyRecorder::qps(time_t window_size) const {
        detail::Sample<Stat> s;
        _latency_window.get_span(window_size, &s);
        // Use floating point to avoid overflow.
        if (s.time_us <= 0) {
            return 0;
        }
        return detail::double_to_random_int(s.data.num * 1000000.0 / s.time_us);
    }

    turbo::Status LatencyRecorder::expose(std::string_view prefix_src, std::string_view help, Scope *scope) {
        // User may add "_latency" as the suffix, remove it.
        std::string_view prefix_v = prefix_src;
        if (turbo::ends_with_ignore_case(prefix_v, "latency")) {
            prefix_v.remove_suffix(7);
            if (prefix_v.empty()) {
                KLOG(ERROR) << "Invalid prefix=" << prefix_src;
                return turbo::invalid_argument_error("Invalid prefix=%s", prefix_src);
            }
        }
        // set debug names for printing helpful error log.
        _latency.set_debug_name(prefix_v);
        _latency_percentile.set_debug_name(prefix_v);

        std::string prefix(prefix_v);
        auto rs = _latency_window.expose(prefix + "_latency", help, scope);
        if (!rs.ok()) {
            return rs;
        }
        rs = _max_latency_window.expose(prefix + "_max_latency", help, scope);
        if (!rs.ok()) {
            return rs;
        }
        rs = _count.expose(prefix + "_count", help, scope);
        if (!rs.ok()) {
            return rs;
        }

        rs = _qps.expose(prefix + "_qps", help, scope);
        if (!rs.ok()) {
            return rs;
        }

        auto p1 = turbo::str_format("%s_latency_%d", prefix.c_str(), (int) turbo::get_flag(FLAGS_tally_latency_p1));
        rs = _latency_p1.expose(p1, help, scope);
        if (!rs.ok()) {
            return rs;
        }
        auto p2 = turbo::str_format("%s_latency_%d", prefix.c_str(), (int) turbo::get_flag(FLAGS_tally_latency_p2));
        rs = _latency_p2.expose(p2, help, scope);
        if (!rs.ok()) {
            return rs;
        }
        auto p3 = turbo::str_format("%s_latency_%d", prefix.c_str(), (int) turbo::get_flag(FLAGS_tally_latency_p3));
        rs = _latency_p3.expose(p3, help, scope);
        if (!rs.ok()) {
            return rs;
        }

        rs = _latency_999.expose(prefix + "_latency_999", help, scope);
        if (!rs.ok()) {
            return rs;
        }

        rs = _latency_9999.expose(prefix + "_latency_9999", help, scope);
        if (!rs.ok()) {
            return rs;
        }
        rs = _latency_percentiles.expose(prefix + "_latency_percentiles", help, scope);
        if (!rs.ok()) {
            return rs;
        }

        rs = _latency_cdf.expose(prefix + "_latency_cdf", help, scope);
        if (!rs.ok()) {
            return rs;
        }
        auto namebuf = turbo::str_format("%d%%,%d%%,%d%%,99.9%%",
                                         (int) turbo::get_flag(FLAGS_tally_latency_p1),
                                         (int) turbo::get_flag(FLAGS_tally_latency_p2),
                                         (int) turbo::get_flag(FLAGS_tally_latency_p3));
        KCHECK_EQ(0, _latency_percentiles.set_vector_names(namebuf));
        return turbo::OkStatus();
    }

    void LatencyRecorder::hide() {
        _latency_window.hide();
        _max_latency_window.hide();
        _count.hide();
        _qps.hide();
        _latency_p1.hide();
        _latency_p2.hide();
        _latency_p3.hide();
        _latency_999.hide();
        _latency_9999.hide();
        _latency_cdf.hide();
        _latency_percentiles.hide();
    }

    LatencyRecorder &LatencyRecorder::operator<<(int64_t latency) {
        latency = latency / turbo::get_flag(FLAGS_tally_latency_scale_factor);
        _latency << latency;
        _max_latency << latency;
        _latency_percentile << latency;
        return *this;
    }

    std::ostream &operator<<(std::ostream &os, const LatencyRecorder &rec) {
        return os << "{latency=" << rec.latency()
                  << " max" << rec.window_size() << '=' << rec.max_latency()
                  << " qps=" << rec.qps()
                  << " count=" << rec.count() << '}';
    }

}  // namespace tally
