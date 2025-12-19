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

#include <tally/buckets_iterator.h>

#include <tally/impl/buckets_calculator.h>

namespace tally {

    BucketsIterator::BucketsIterator(BucketsCalculator calculator, uint64_t index)
            : calculator_(calculator), index_(index) {}

    BucketsIterator &BucketsIterator::operator++() {
        index_++;
        return *this;
    }

    BucketsIterator BucketsIterator::operator++(int) {
        // Note that the postfix ++ operator takes a dummy int parameter to
        // distinguish it from the prefix ++ operator.
        const auto old_value = *this;
        ++(*this);
        return old_value;
    }

    bool BucketsIterator::operator==(BucketsIterator other) const {
        return calculator_ == other.calculator_ && index_ == other.index_;
    }

    bool BucketsIterator::operator!=(BucketsIterator other) const {
        return !(*this == other);
    }

    double BucketsIterator::operator*() { return calculator_.calculate(index_); }

}  // namespace tally
