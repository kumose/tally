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

#include <chrono>
#include <thread>

#include <gtest/gtest.h>
#include "mock_stats_reporter.h"
#include <tally/tally.h>

TURBO_FLAG(int32_t, test_flag_gauge, 5, "abvc help");

TEST(ScopeImplTest, GetOrCreateSubScope) {
    auto scope = tally::ScopeBuilder().prefix("t").build();
    std::vector<std::shared_ptr<tally::Scope>> ss = tally::ScopeInstance::instance()->list_scopes();
    for(auto & s : ss) {
        KLOG(INFO)<<s->id();
    }
    ASSERT_EQ(4,ss.size());
    auto sub_scope = scope->sub_scope("foo");
    ss = tally::ScopeInstance::instance()->list_scopes();
    for(auto & s : ss) {
        KLOG(INFO)<<s->id();
    }
    ASSERT_EQ(5,ss.size());
    EXPECT_EQ(sub_scope, scope->sub_scope("foo"));
    ss = tally::ScopeInstance::instance()->list_scopes();
    for(auto & s : ss) {
        KLOG(INFO)<<s->id();
    }
    ASSERT_EQ(5,ss.size());
    EXPECT_EQ(sub_scope, tally::ScopeInstance::instance()->get_scope(sub_scope->id()));
    EXPECT_EQ(sub_scope, tally::ScopeInstance::instance()->get_scope(sub_scope->prefix(), sub_scope->tags()));
    EXPECT_NE(sub_scope, scope->sub_scope("bar"));
    ss = tally::ScopeInstance::instance()->list_scopes();
    for(auto & s : ss) {
        KLOG(INFO)<<s->id();
    }
    ASSERT_EQ(6,ss.size());
    auto root = tally::ScopeInstance::instance()->get_scope("km", {});
    ASSERT_EQ(root, tally::ScopeInstance::instance()->get_default());
    auto sys = tally::ScopeInstance::instance()->get_scope("km_sys");
    ASSERT_EQ(sys, tally::ScopeInstance::instance()->get_sys_scope());
    auto flag = tally::ScopeInstance::instance()->get_scope("km_flag");
    ASSERT_EQ(flag, tally::ScopeInstance::instance()->get_flag_scope());

    ss = tally::ScopeInstance::instance()->list_scopes();
    for(auto & s : ss) {
        KLOG(INFO)<<s->id();
    }
    ASSERT_EQ(6,ss.size());
    ss = tally::ScopeInstance::instance()->list_scopes(true);
    ASSERT_EQ(3,ss.size());
}

TEST(ScopeImplTest, GetOrCreateTagged) {
    auto scope = tally::ScopeBuilder().prefix("rt").build();
    auto sub_scope = scope->tagged({{"a", "1"}});
    EXPECT_EQ(sub_scope, scope->tagged({{"a", "1"}}));
    EXPECT_NE(sub_scope, scope->tagged({{"b", "2"}}));
}

/*
TEST(ScopeImplTest, Reporting) {
    std::string prefix("foo");
    std::string help("help");
    std::string subscope_name("bar");
    std::string gauge_name("g_baz");
    std::string counter_name("c_baz");
    std::string histogram_name("h_baz");
    turbo::flat_hash_map<std::string, std::string> scope_tags({{"a", "1"}});
    turbo::flat_hash_map<std::string, std::string> subscope_tags({{"b", "2"}});

    std::string g_expected_name = "foo_bar_g_baz";
    std::string c_expected_name = "foo_bar_c_baz";
    std::string h_expected_name = "foo_bar_h_baz";
    turbo::flat_hash_map<std::string, std::string> expected_tags = {{"a", "1"},
                                                                        {"b", "2"}};
    turbo::Time now = turbo::Time::current_time();

    auto buckets = tally::Buckets::linear_values(0.0, 1.0, 10);

    auto reporter = std::make_shared<MockStatsReporter>();

    auto b = tally::Histogram::create_buckets(buckets);
    b[3].value = 1;
    tally::CollectedSample c_sm{tally::VariableType::counter_type(), 1.0, now};
    tally::CollectedSample g_sm{tally::VariableType::gauge_type(), 1.0, now};
    tally::CollectedSample h_sm{tally::VariableType::histogram_type(), tally::HistogramSample{b,2.5, 1}, now};
    EXPECT_CALL(*reporter.get(), report_variable(testing::_, help, expected_tags, testing::_, now)).Times(3);

    auto scope = tally::ScopeBuilder()
            .prefix(prefix)
            .tags(scope_tags)
            .build();
    auto subscope = scope->sub_scope(subscope_name)->tagged(subscope_tags);

    tally::Counter counter = subscope->counter(counter_name, help).value();
    auto gauge = subscope->gauge<double>(gauge_name, help).value();

    auto histogram = subscope->histogram(histogram_name, help, buckets).value();

    counter->increment();
    gauge->update(1.0);
    histogram->record(2.5);
    subscope->report(reporter.get(), now);
}
*/
