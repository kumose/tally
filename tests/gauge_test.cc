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


TEST(GaugeImplTest, UpdateOnce) {
    std::string name("foo");
    std::string help("help");
    turbo::flat_hash_map<std::string, std::string> tags({});
    auto reporter = std::make_shared<MockStatsReporter>();
    turbo::Time now = turbo::Time::current_time();
    tally::Gauge<double> gauge;
    EXPECT_CALL(*reporter.get(), report_variable( &gauge, now)).Times(1);

    gauge.update(1.5);
    reporter->report_variable(&gauge, now);
}

TEST(GaugeImplTest, avager) {
    std::string name("foo");
    std::string help("help");
    turbo::flat_hash_map<std::string, std::string> tags({});
    auto reporter = std::make_shared<MockStatsReporter>();
    turbo::Time now = turbo::Time::current_time();
    tally::AverageGauge gauge;
    EXPECT_CALL(*reporter.get(), report_variable(&gauge, now)).Times(1);


    gauge.update(1);
    gauge.update(2);
    reporter->report_variable(&gauge, now);
}

TEST(GaugeImplTest, UpdateMultipleTimes) {
    std::string name("foo");
    std::string help("help");
    turbo::Time now = turbo::Time::current_time();
    tally::CollectedSample sm2{tally::VariableType::gauge_type(), 2.25, now};
    turbo::flat_hash_map<std::string, std::string> tags({});
    auto reporter = std::make_shared<MockStatsReporter>();
    tally::Gauge<double> gauge;
    EXPECT_CALL(*reporter.get(), report_variable(&gauge, now)).Times(1);

    gauge.update(1.5);
    gauge.update(2.25);
    reporter->report_variable(&gauge, now);
}

TEST(GaugeImplTest, ValueIsReset) {
    std::string name("foo");
    std::string help("help");
    turbo::Time now = turbo::Time::current_time();
    turbo::flat_hash_map<std::string, std::string> tags({});
    auto reporter = std::make_shared<MockStatsReporter>();
    tally::CollectedSample sm15{tally::VariableType::gauge_type(), 1.5, now};
    tally::CollectedSample sm2{tally::VariableType::gauge_type(), 2.25, now};
    tally::Gauge<double> gauge;
    EXPECT_CALL(*reporter.get(), report_variable(&gauge, now)).Times(2);

    gauge.update(1.5);
    reporter->report_variable(&gauge, now);

    gauge.update(2.25);
    reporter->report_variable(&gauge, now);
}

TEST(RecorderTest, test_compress) {
    const uint64_t num = 125345;
    const uint64_t sum = 26032906;
    const uint64_t compressed = tally::AverageGauge::_compress(num, sum);
    ASSERT_EQ(num, tally::AverageGauge::_get_num(compressed));
    ASSERT_EQ(sum, tally::AverageGauge::_get_sum(compressed));
}


TEST(RecorderTest, test_compress_negtive_number) {
    for (int a = -10000000; a < 10000000; ++a) {
        const uint64_t sum = tally::AverageGauge::_get_complement(a);
        const uint64_t num = 123456;
        const uint64_t compressed = tally::AverageGauge::_compress(num, sum);
        ASSERT_EQ(num, tally::AverageGauge::_get_num(compressed));
        ASSERT_EQ(a, tally::AverageGauge::_extend_sign_bit(tally::AverageGauge::_get_sum(compressed)));
    }
}


TEST(RecorderTest, sanity) {
    // default prefix = "km"
    auto scope = tally::ScopeInstance::instance()->get_default();
    {
        tally::AverageGauge recorder;
        ASSERT_TRUE(recorder.valid());
        ASSERT_TRUE(recorder.expose("var1", "h", scope.get()).ok());
        for (size_t i = 0; i < 100; ++i) {
            recorder << 2;
        }
        ASSERT_EQ(2l, (int64_t) recorder.average());
        ASSERT_EQ("2", tally::Variable::describe_exposed("km_var1"));
        std::vector<std::string> vars;
        tally::Variable::list_exposed(&vars);
        ASSERT_EQ(1UL, vars.size());
        ASSERT_EQ("km_var1", vars[0]);
        ASSERT_EQ(1UL, tally::Variable::count_exposed());
    }
    // dtor hide;
    ASSERT_EQ(0UL, tally::Variable::count_exposed());
}


TEST(RecorderTest, window) {
    tally::AverageGauge c1;
    ASSERT_TRUE(c1.valid());
    tally::Window<tally::AverageGauge> w1(&c1, 1);
    tally::Window<tally::AverageGauge> w2(&c1, 2);
    tally::Window<tally::AverageGauge> w3(&c1, 3);

    const int N = 10000;
    int64_t last_time = turbo::Time::current_microseconds();
    for (int i = 1; i <= N; ++i) {
        c1 << i;
        int64_t now = turbo::Time::current_microseconds();
        if (now - last_time >= 1000000L) {
            last_time = now;
            KLOG(INFO) << "c1=" << c1 << " w1=" << w1 << " w2=" << w2 << " w3=" << w3;
        } else {
            usleep(950);
        }
    }
}

TEST(RecorderTest, negative) {
    tally::AverageGauge recorder;
    ASSERT_TRUE(recorder.valid());
    for (size_t i = 0; i < 3; ++i) {
        recorder << -2;
    }
    ASSERT_EQ(-2, recorder.average());
}

const size_t OPS_PER_THREAD = 20000000;

static void *thread_counter(void *arg) {
    tally::AverageGauge *recorder = (tally::AverageGauge *) arg;

    auto start = turbo::Time::current_time();
    for (int i = 0; i < (int) OPS_PER_THREAD; ++i) {
        *recorder << i;
    }
    auto end = turbo::Time::current_time();
    auto n_elapsed = turbo::Duration::to_nanoseconds(end - start);
    return (void *) (n_elapsed);
}

TEST(RecorderTest, perf) {
    tally::AverageGauge recorder;
    ASSERT_TRUE(recorder.valid());
    pthread_t threads[8];
    for (size_t i = 0; i < TURBO_ARRAYSIZE(threads); ++i) {
        pthread_create(&threads[i], nullptr, &thread_counter, (void *) &recorder);
    }
    long totol_time = 0;
    for (size_t i = 0; i < TURBO_ARRAYSIZE(threads); ++i) {
        void *ret;
        pthread_join(threads[i], &ret);
        totol_time += (long) ret;
    }
    ASSERT_EQ(((int64_t) OPS_PER_THREAD - 1) / 2, recorder.average());
    KLOG(INFO) << "Recorder takes " << totol_time / (OPS_PER_THREAD * TURBO_ARRAYSIZE(threads))
              << "ns per sample with " << TURBO_ARRAYSIZE(threads)
              << " threads";
}