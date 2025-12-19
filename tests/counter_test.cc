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
