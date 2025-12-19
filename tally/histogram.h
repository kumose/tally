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

#include <algorithm>
#include <string>
#include <turbo/container/flat_hash_map.h>
#include <vector>

#include <tally/buckets.h>
#include <tally/histogram.h>
#include <tally/impl/histogram_bucket.h>
#include <tally/stats_reporter.h>
#include <tally/stopwatch.h>
#include <tally/variable.h>
#include <tally/counter.h>
#include <tally/scope.h>
#include <turbo/times/time.h>

namespace tally {

    class Histogram;

    class TimeRecorder {
    public:
        TimeRecorder(turbo::Nonnull<Histogram *> h) : _hist(h), _timer() {}

        ~TimeRecorder();

        void cancel() {
            _hist = nullptr;
        }

    private:
        Histogram *_hist{nullptr};
        turbo::TimeCost _timer;
    };

    class Histogram : public Variable {
    public:
        Histogram();

        explicit Histogram(const Buckets &buckets) noexcept;

        Histogram(const Buckets &buckets, std::string_view name, std::string_view help,
                  turbo::Nonnull<Scope *> scope = ScopeInstance::instance()->get_default().get()) noexcept;

        ~Histogram() override;

        // Record the given value.
        void record(double) noexcept;

        TimeRecorder record_timer() noexcept {
            return  TimeRecorder(this);
        }

        virtual MetricSample get_metric(const turbo::Time &stamp) const {
            return {type(), HistogramSample{get_value(), sample_sum_.get_value(),
                                            sample_count_.get_value()},
                    stamp};
        }

        void set_buckets(const Buckets &buckets) noexcept;

        std::vector<HistogramBucket> get_value() const;

        static std::vector<HistogramBucket> create_buckets(const Buckets &buckets);

    private:
        std::vector<HistogramBucket> buckets_;
        std::vector<Counter<int64_t>> buckets_value_;
        Counter<double> sample_sum_;
        Counter<int64_t> sample_count_;
    };

    inline TimeRecorder::~TimeRecorder() {
        if (_hist) {
            _hist->record(turbo::Duration::to_double_microseconds(_timer.stop()));
        }
    }
}  // namespace tally
