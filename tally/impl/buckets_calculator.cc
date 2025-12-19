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
