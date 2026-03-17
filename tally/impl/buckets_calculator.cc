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

#include <cmath>
#include <cstdint>
#include <tally/impl/buckets_calculator.h>

namespace tally {

    BucketsCalculator::BucketsCalculator(Growth growth, double start, double update)
            : growth_(growth), start_(start), update_(update) {}

    double BucketsCalculator::calculate(uint64_t index) const {
        if (growth_ == BucketsCalculator::Growth::Exponential) {
            return start_ * std::pow(update_, static_cast<double>(index));
        }

        return start_ + (update_ * static_cast<double>(index));
    }

    bool BucketsCalculator::operator==(BucketsCalculator other) const {
        return growth_ == other.growth_ && start_ == other.start_ &&
               update_ == other.update_;
    }

}  // namespace tally
