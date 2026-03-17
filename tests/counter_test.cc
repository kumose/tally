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

#include "mock_stats_reporter.h"
#include <tally/tally.h>

TEST(CounterImplTest, IncrementOnce) {
    std::string name("foo");
    std::string help("help");
    turbo::flat_hash_map<std::string, std::string> tags({});
    auto reporter = std::make_shared<MockStatsReporter>();
    turbo::Time now = turbo::Time::current_time();
    tally::Counter<int64_t> counter;
    EXPECT_CALL(*reporter.get(), report_variable(&counter,now)).Times(1);

    counter.increment(1);
    reporter->report_variable(&counter, now);
}

TEST(CounterImplTest, IncrementMultipleTimes) {
    std::string name("foo");
    std::string help("help");
    turbo::Time now = turbo::Time::current_time();
    turbo::flat_hash_map<std::string, std::string> tags({});
    auto reporter = std::make_shared<MockStatsReporter>();
    tally::Counter<int64_t> counter;
    EXPECT_CALL(*reporter.get(), report_variable(&counter,now)).Times(1);


    counter.increment(1);
    counter.increment(2);
    reporter->report_variable(&counter, now);
}

TEST(CounterImplTest, ValueIsReset) {
    std::string name("foo");
    std::string help("help");
    turbo::Time now = turbo::Time::current_time();
    turbo::flat_hash_map<std::string, std::string> tags({});
    auto reporter = std::make_shared<MockStatsReporter>();
    tally::CollectedSample sm{tally::VariableType::counter_type(), 1.0, now};
    tally::CollectedSample sm3{tally::VariableType::counter_type(), 3.0, now};
    tally::Counter<int64_t> counter;
    EXPECT_CALL(*reporter.get(), report_variable(&counter,now)).Times(2);

    counter.increment(1);
    reporter->report_variable(&counter, now);

    counter.increment(2);
    reporter->report_variable(&counter, now);
}
