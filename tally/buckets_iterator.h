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

#include <iterator>

#include <tally/impl/buckets_calculator.h>

namespace tally {

    class BucketsIterator : public std::iterator<std::input_iterator_tag, double> {
        friend class Buckets;

    public:
        BucketsIterator &operator++();

        BucketsIterator operator++(int);

        bool operator==(BucketsIterator other) const;

        bool operator!=(BucketsIterator other) const;

        double operator*();

    private:
        BucketsIterator(BucketsCalculator calculator, uint64_t index);

        const BucketsCalculator calculator_;
        uint64_t index_;
    };

}  // namespace tally
