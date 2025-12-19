// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

// Date 2014/10/13 19:47:59

#include <pthread.h>                                // pthread_*

#include <cstddef>
#include <memory>
#include <iostream>
#include <tally/tally.h>
#include <gtest/gtest.h>

namespace {
    TEST(RecorderTest, test_complement) {
        KLOG(INFO) << "sizeof(LatencyRecorder)=" << sizeof(tally::LatencyRecorder)
                  << " " << sizeof(tally::detail::Percentile)
                  << " " << sizeof(tally::MaxerGauge<int64_t>)
                  << " " << sizeof(tally::AverageGauge)
                  << " " << sizeof(tally::Window<tally::AverageGauge>)
                  << " " << sizeof(tally::Window<tally::detail::Percentile>);

        for (int a = -10000000; a < 10000000; ++a) {
            const uint64_t complement = tally::AverageGauge::_get_complement(a);
            const int64_t b = tally::AverageGauge::_extend_sign_bit(complement);
            ASSERT_EQ(a, b);
        }
    }


    TEST(RecorderTest, positive_overflow) {
        auto scope = tally::ScopeBuilder().prefix("g").build();
        tally::AverageGauge recorder1;
        ASSERT_TRUE(recorder1.valid());
        for (int i = 0; i < 5; ++i) {
            recorder1 << std::numeric_limits<int64_t>::max();
        }
        ASSERT_EQ(std::numeric_limits<int>::max(), recorder1.average());

        tally::AverageGauge recorder2;
        ASSERT_TRUE(recorder2.valid());
        recorder2.set_debug_name("recorder2");
        for (int i = 0; i < 5; ++i) {
            recorder2 << std::numeric_limits<int64_t>::max();
        }
        ASSERT_EQ(std::numeric_limits<int>::max(), recorder2.average());

        tally::AverageGauge recorder3;
        ASSERT_TRUE(recorder3.valid());
        auto rs = recorder3.expose("recorder3", "help", scope.get());
        ASSERT_TRUE(rs.ok());
        for (int i = 0; i < 5; ++i) {
            recorder3 << std::numeric_limits<int64_t>::max();
        }
        ASSERT_EQ(std::numeric_limits<int>::max(), recorder3.average());

        tally::LatencyRecorder latency1;
        rs = latency1.expose("latency1", "help", scope.get());
        ASSERT_TRUE(rs.ok());
        latency1 << std::numeric_limits<int64_t>::max();

        tally::LatencyRecorder latency2;
        latency2 << std::numeric_limits<int64_t>::max();
    }

    TEST(RecorderTest, negtive_overflow) {
        auto scope = tally::ScopeBuilder().prefix("g").build();
        tally::AverageGauge recorder1;
        ASSERT_TRUE(recorder1.valid());
        for (int i = 0; i < 5; ++i) {
            recorder1 << std::numeric_limits<int64_t>::min();
        }
        ASSERT_EQ(std::numeric_limits<int>::min(), recorder1.average());

        tally::AverageGauge recorder2;
        ASSERT_TRUE(recorder2.valid());
        recorder2.set_debug_name("recorder2");
        for (int i = 0; i < 5; ++i) {
            recorder2 << std::numeric_limits<int64_t>::min();
        }
        ASSERT_EQ(std::numeric_limits<int>::min(), recorder2.average());

        tally::AverageGauge recorder3;
        ASSERT_TRUE(recorder3.valid());
        auto rs = recorder3.expose("recorder3", "help", scope.get());
        ASSERT_TRUE(rs.ok());
        for (int i = 0; i < 5; ++i) {
            recorder3 << std::numeric_limits<int64_t>::min();
        }
        ASSERT_EQ(std::numeric_limits<int>::min(), recorder3.average());

        tally::LatencyRecorder latency1;
        rs = latency1.expose("latency1", "help", scope.get());
        ASSERT_TRUE(rs.ok());
        latency1 << std::numeric_limits<int64_t>::min();

        tally::LatencyRecorder latency2;
        latency2 << std::numeric_limits<int64_t>::min();
    }


    TEST(RecorderTest, latency_recorder_qps_accuracy) {
        tally::LatencyRecorder lr1(2); // set windows size to 2s
        tally::LatencyRecorder lr2(2);
        tally::LatencyRecorder lr3(2);
        tally::LatencyRecorder lr4(2);
        usleep(3000000); // wait sampler to sample 3 times

        auto write = [](tally::LatencyRecorder &lr, int times) {
            for (int i = 0; i < times; ++i) {
                lr << 1;
            }
        };
        write(lr1, 10);
        write(lr2, 11);
        write(lr3, 3);
        write(lr4, 1);
        usleep(1000000); // wait sampler to sample 1 time

        auto read = [](tally::LatencyRecorder &lr, double exp_qps, int window_size = 0) {
            int64_t qps_sum = 0;
            int64_t exp_qps_int = (int64_t) exp_qps;
            for (int i = 0; i < 1000; ++i) {
                int64_t qps = window_size ? lr.qps(window_size) : lr.qps();
                EXPECT_GE(qps, exp_qps_int - 1);
                EXPECT_LE(qps, exp_qps_int + 1);
                qps_sum += qps;
            }
            double err = fabs(qps_sum / 1000.0 - exp_qps);
            return err;
        };
        ASSERT_GT(0.1, read(lr1, 10 / 2.0));
        ASSERT_GT(0.1, read(lr2, 11 / 2.0));
        ASSERT_GT(0.1, read(lr3, 3 / 2.0));
        ASSERT_GT(0.1, read(lr4, 1 / 2.0));

        ASSERT_GT(0.1, read(lr1, 10 / 3.0, 3));
        ASSERT_GT(0.2, read(lr2, 11 / 3.0, 3));
        ASSERT_GT(0.1, read(lr3, 3 / 3.0, 3));
        ASSERT_GT(0.1, read(lr4, 1 / 3.0, 3));
    }

} // namespace
