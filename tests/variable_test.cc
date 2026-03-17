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
#include <turbo/log/logging.h>

TURBO_FLAG(int32_t, test_flag_gauge, 5, "abvc help");

TEST(GaugeImplTest, UpdateOnce) {
    turbo::flat_hash_map<std::string, std::string> tags({{"a", "1"}});
    auto scope = tally::ScopeBuilder().prefix("t").tags(tags).build();
    std::string name("foo");
    std::string help("help");
    std::string expect_name = scope->fully_qualified_name(name);
    auto reporter = std::make_shared<MockStatsReporter>();
    turbo::Time now = turbo::Time::current_time();
    tally::CollectedSample sm15{tally::VariableType::gauge_type(), 1.5, now};
    tally::Gauge<double> gauge;
    EXPECT_CALL(*reporter.get(), report_variable(testing::_, now)).Times(1);

    auto s = gauge.expose(name, help, scope.get());
    ASSERT_TRUE(s.ok());
    gauge.update(1.5);
    tally::Variable::report(reporter.get(), now);
}

TEST(GaugeImplTest, no_call) {
    turbo::flat_hash_map<std::string, std::string> tags({{"a", "1"}});
    auto scope = tally::ScopeBuilder().prefix("t").tags(tags).build();
    std::string name("foo");
    std::string help("help");
    std::string expect_name = scope->fully_qualified_name(name);
    auto reporter = std::make_shared<MockStatsReporter>();
    turbo::Time now = turbo::Time::current_time();
    tally::CollectedSample sm15{tally::VariableType::gauge_type(), 1.5, now};
    EXPECT_CALL(*reporter.get(), report_variable(testing::_, now)).Times(0);

    // no expose
    tally::Gauge<double> gauge;
    gauge.update(1.5);
    tally::Variable::report(reporter.get(), now);
}

TEST(GaugeImplTest, re_expose) {
    turbo::flat_hash_map<std::string, std::string> tags({{"a", "1"}});
    auto scope = tally::ScopeBuilder().prefix("t").tags(tags).build();
    std::string old_name("old");
    std::string name("foo");
    std::string help("help");
    std::string expect_name = scope->fully_qualified_name(name);
    std::string expect_old_name = scope->fully_qualified_name(old_name);
    auto reporter = std::make_shared<MockStatsReporter>();
    turbo::Time now = turbo::Time::current_time();
    tally::CollectedSample sm15{tally::VariableType::gauge_type(), 1.5, now};

    EXPECT_CALL(*reporter.get(), report_variable(testing::_, now)).Times(1);

    tally::Gauge<double> gauge;
    auto s = gauge.expose(old_name, help, scope.get());
    ASSERT_EQ(expect_old_name, gauge.full_name());
    ASSERT_EQ(tags, gauge.tags());
    ASSERT_TRUE(s.ok());
    s = gauge.expose(name, help, scope.get());
    ASSERT_TRUE(s.ok());
    ASSERT_EQ(expect_name, gauge.full_name());
    gauge.update(1.5);
    tally::Variable::report(reporter.get(), now);
}
TEST(GaugeImplTest, prom) {
    turbo::flat_hash_map<std::string, std::string> tags({{"a", "1"}});
    auto scope = tally::ScopeBuilder().prefix("t").tags(tags).build();
    std::string name("foo");
    std::string help("help");
    std::stringstream ss;
    tally::PrometheusStatsReporter reporter(ss);
    nlohmann::ordered_json result;
    auto json_reporter = tally::JsonStatsReporter(result);
    turbo::Time now = turbo::Time::current_time();
    tally::CollectedSample sm15{tally::VariableType::gauge_type(), 1.5, now};

    auto buckets = tally::Buckets::linear_values(0.0, 1.0, 10);
    tally::Histogram h1(buckets);


    tally::Gauge<double> g1;
    tally::Gauge<double> g2;
    tally::Counter<int64_t> c1;
    tally::Counter<int64_t> c2;
    auto s = g1.expose(name + "_g1", help, scope.get());
    ASSERT_TRUE(s.ok());
    s = g1.expose(name + "_g2", help, scope.get());
    ASSERT_TRUE(s.ok());
    s = c1.expose(name + "_c1", help, scope.get());
    ASSERT_TRUE(s.ok());
    s = c2.expose(name + "_c2", help, scope.get());
    ASSERT_TRUE(s.ok());
    s = h1.expose(name + "_h1", help, scope.get());
    ASSERT_TRUE(s.ok());
    g1.update(1.5);
    g2.update(2.5);
    c1.increment();
    c2.increment(10);
    h1.record(15);
    h1.record(5);
    h1.record(5.5);
    h1.record(11);
    h1.record(0.1);
    h1.record(3.1);

    tally::FlagGauge flag_gauge(&FLAGS_test_flag_gauge, scope.get());
    turbo::set_flag(&FLAGS_test_flag_gauge,10);
    tally::Variable::report(&reporter, now);
    reporter.flush();
    KLOG(INFO)<<NOPREFIX<<"-------------------------------------------------------------";
    KLOG(INFO)<<NOPREFIX<<reporter.describe();
    KLOG(INFO)<<NOPREFIX<<ss.str();
    ss.clear();
    tally::Variable::report(&json_reporter, now);
    json_reporter.flush();
    KLOG(INFO)<<NOPREFIX<<"-------------------------------------------------------------";
    KLOG(INFO)<<NOPREFIX<<json_reporter.describe();
    KLOG(INFO)<<NOPREFIX<<result.dump();
    auto sys = tally::Reporter::get_prometheus_reporting();
    KLOG(INFO)<<NOPREFIX<<"-------------------------------------------------------------";
    KLOG(INFO)<<NOPREFIX<<sys;
    auto sc = tally::ScopeInstance::instance()->get_sys_scope();
    ASSERT_TRUE(sc);
    //tally::SigarMetric::expose();
    //ASSERT_TRUE(sc->count_exposed() == 2)<<sc->count_exposed();

    //auto ins = tally::SigarMetric::instance();
    //sc->report(tally::Reporter::get_prometheus_reporter().get(), now);
    //tally::Reporter::get_prometheus_reporter()->flush();
    KLOG(INFO)<<NOPREFIX<<"-------------------------------------------------------------";
    KLOG(INFO)<<NOPREFIX<<tally::Reporter::get_prometheus_reporting();
    KLOG(INFO)<<NOPREFIX<<"before enable system default metric: "<<tally::Variable::count_exposed();
    tally::SigarMetric::instance()->expose();
    KLOG(INFO)<<NOPREFIX<<"enable system default metric: "<<tally::Variable::count_exposed();
    tally::SigarMetric::instance()->hide();
    KLOG(INFO)<<NOPREFIX<<"disable system default metric: "<<tally::Variable::count_exposed();
}
