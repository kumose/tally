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
