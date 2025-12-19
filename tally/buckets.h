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
