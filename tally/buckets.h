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

#include <turbo/times/time.h>
#include <vector>
#include <tally/buckets_iterator.h>
#include <tally/impl/buckets_calculator.h>

namespace tally {

    class Buckets {
    public:
        // linear_values constructs a linear sequence of `num` Value buckets beginning
        // at `start` and incrementing by `width` each time.
        static Buckets linear_values(double start, double width, uint64_t num);

        // exponential_values constructs an expoential sequence of `num` Value buckets
        // beginning at `start` and multiplying by `factor` each time.
        static Buckets exponential_values(double start, double factor, uint64_t num);

        // linear_durations constructs a linear sequence of `num` Duration buckets
        // beginning at `start` and incrementing by `width` each time.
        static Buckets linear_durations(turbo::Duration start,
                                       turbo::Duration width, uint64_t num);

        // exponential_durations constructs an expoential sequence of `num` Duration
        // buckets beginning at `start` and multiplying by `factor` each time.
        static Buckets exponential_durations(turbo::Duration start,
                                            uint64_t factor, uint64_t num);

        // Kind is an enum representing the type of a sequence of buckets.
        enum class Kind {
            Values,
            Durations,
        };

        BucketsIterator begin() const;

        BucketsIterator end() const;

        uint64_t size() const { return num_; }

        Kind kind() const { return kind_; }

    private:
        Buckets(Kind kind, BucketsCalculator calculator, uint64_t num);

        const Kind kind_;
        const BucketsCalculator calculator_;
        const uint64_t num_;
    };

}  // namespace tally
