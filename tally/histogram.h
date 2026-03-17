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
