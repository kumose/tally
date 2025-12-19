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
