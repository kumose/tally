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
