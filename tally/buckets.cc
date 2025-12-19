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


#include <tally/buckets.h>
#include <stdexcept>
#include <tally/buckets_iterator.h>
#include <tally/impl/buckets_calculator.h>

namespace tally {

    Buckets::Buckets(Buckets::Kind kind, BucketsCalculator calculator, uint64_t num)
            : kind_(kind), calculator_(calculator), num_(num) {
        if (num == 0) {
            throw std::invalid_argument("Number of buckets cannot be zero");
        }
    }

    Buckets Buckets::linear_values(double start, double width, uint64_t num) {
        if (width <= 0) {
            throw std::invalid_argument("Bucket width must be positive");
        }

        auto const calculator =
                BucketsCalculator(BucketsCalculator::Growth::Linear, start, width);
        return Buckets{Buckets::Kind::Values, calculator, num};
    }

    Buckets Buckets::exponential_values(double start, double factor, uint64_t num) {
        if (factor <= 1) {
            throw std::invalid_argument("Bucket factor must be greater than 1");
        }

        auto const calculator =
                BucketsCalculator(BucketsCalculator::Growth::Exponential, start, factor);
        return Buckets{Buckets::Kind::Values, calculator, num};
    }

    Buckets Buckets::linear_durations(turbo::Duration start,
                                     turbo::Duration width, uint64_t num) {
        if (width <= turbo::Duration::zero()) {
            throw std::invalid_argument("Bucket width must be positive");
        }
        auto const calculator = BucketsCalculator(BucketsCalculator::Growth::Linear,
                                                  turbo::Duration::to_double_microseconds(start),
                                                  turbo::Duration::to_double_microseconds(width));
        return Buckets{Buckets::Kind::Durations, calculator, num};
    }

    Buckets Buckets::exponential_durations(turbo::Duration start,
                                          uint64_t factor, uint64_t num) {
        if (factor <= 1) {
            throw std::invalid_argument("Bucket factor must be greater than 1");
        }

        auto const calculator = BucketsCalculator(
                BucketsCalculator::Growth::Exponential,
                static_cast<double>(turbo::Duration::to_double_microseconds(start)), static_cast<double>(factor));
        return Buckets{Buckets::Kind::Durations, calculator, num};
    }

    BucketsIterator Buckets::begin() const {
        return BucketsIterator{calculator_, 0};
    }

    BucketsIterator Buckets::end() const {
        return BucketsIterator{calculator_, static_cast<uint64_t>(num_)};
    }

}  // namespace tally
