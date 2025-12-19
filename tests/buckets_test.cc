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

#include <gtest/gtest.h>
#include <tally/tally.h>

TEST(BucketsTest, LinearIteratorBuckets) {
    double start = 4.0;
    double width = 6.0;
    uint64_t num = 3.0;
    auto buckets = tally::Buckets::linear_values(start, width, num);

    double expected = start;
    for (auto it = buckets.begin(); it != buckets.end(); it++) {
        EXPECT_EQ(expected, *it);
        expected += width;
    }
}

TEST(BucketsTest, LinearIteratorEquality) {
    auto buckets = tally::Buckets::linear_values(1.0, 2.0, 10);

    auto other_it = buckets.begin();
    for (auto it = buckets.begin(); it != buckets.end(); it++) {
        EXPECT_EQ(other_it, it);
        other_it++;
    }
}

TEST(BucketsTest, ExponentialIteratorBuckets) {
    double start = 4.0;
    double factor = 2.0;
    uint64_t num = 3.0;
    auto buckets = tally::Buckets::exponential_values(start, factor, num);

    double expected = start;
    for (auto it = buckets.begin(); it != buckets.end(); it++) {
        EXPECT_EQ(expected, *it);
        expected *= factor;
    }
}

TEST(BucketsTest, ExponentialIteratorEquality) {
    auto buckets = tally::Buckets::exponential_values(1.0, 2.0, 10);

    auto other_it = buckets.begin();
    for (auto it = buckets.begin(); it != buckets.end(); it++) {
        EXPECT_EQ(other_it, it);
        other_it++;
    }
}

TEST(BucketsTest, Size) {
    auto linear_buckets = tally::Buckets::exponential_values(1.0, 2.0, 4);
    EXPECT_EQ(4, linear_buckets.size());

    auto exponential_buckets = tally::Buckets::exponential_values(1.0, 2.0, 10);
    EXPECT_EQ(10, exponential_buckets.size());
}
