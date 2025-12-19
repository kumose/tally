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

#include <limits>                           //std::numeric_limits

#include <tally/tally.h>
#include <gtest/gtest.h>
#include <turbo/strings/str_split.h>
#include <turbo/strings/numbers.h>

namespace {
    class ReducerTest : public testing::Test {
    protected:
        void SetUp() {}

        void TearDown() {}
    };

    TEST_F(ReducerTest, atomicity) {
        ASSERT_EQ(sizeof(int32_t), sizeof(tally::detail::ElementContainer<int32_t>));
        ASSERT_EQ(sizeof(int64_t), sizeof(tally::detail::ElementContainer<int64_t>));
        ASSERT_EQ(sizeof(float), sizeof(tally::detail::ElementContainer<float>));
        ASSERT_EQ(sizeof(double), sizeof(tally::detail::ElementContainer<double>));
    }

    TEST_F(ReducerTest, adder) {
        tally::Counter<uint32_t> reducer1;
        EXPECT_TRUE(reducer1.type().is_counter());
        ASSERT_TRUE(reducer1.valid());
        reducer1 << 2 << 4;
        ASSERT_EQ(6ul, reducer1.get_value());
        std::any v1;
        reducer1.get_value(&v1);
        ASSERT_EQ(6u, std::any_cast<unsigned int>(v1));

        tally::Counter<double> reducer2;
        EXPECT_TRUE(reducer2.type().is_counter());
        ASSERT_TRUE(reducer2.valid());
        reducer2 << 2.0 << 4.0;
        ASSERT_DOUBLE_EQ(6.0, reducer2.get_value());

        tally::Counter<int> reducer3;
        ASSERT_TRUE(reducer3.valid());
        reducer3 << -9 << 1 << 0 << 3;
        ASSERT_EQ(-5, reducer3.get_value());
    }

    const size_t OPS_PER_THREAD = 500000;

    static void *thread_counter(void *arg) {
        tally::Counter<uint64_t> *reducer = (tally::Counter<uint64_t> *) arg;
        auto start = turbo::Time::current_time();
        for (size_t i = 0; i < OPS_PER_THREAD; ++i) {
            (*reducer) << 2;
        }
        auto end = turbo::Time::current_time();
        auto n_elapsed = turbo::Duration::to_nanoseconds(end - start);
        return (void *) (n_elapsed);
    }

    void *add_atomic(void *arg) {
        std::atomic<uint64_t> *counter = (std::atomic<uint64_t> *) arg;
        auto start = turbo::Time::current_time();
        for (size_t i = 0; i < OPS_PER_THREAD / 100; ++i) {
            counter->fetch_add(2, std::memory_order_relaxed);
        }
        auto end = turbo::Time::current_time();
        auto n_elapsed = turbo::Duration::to_nanoseconds(end - start);
        return (void *) (n_elapsed);
    }

    static long start_perf_test_with_atomic(size_t num_thread) {
        std::atomic<uint64_t> counter(0);
        pthread_t threads[num_thread];
        for (size_t i = 0; i < num_thread; ++i) {
            pthread_create(&threads[i], nullptr, &add_atomic, (void *) &counter);
        }
        long totol_time = 0;
        for (size_t i = 0; i < num_thread; ++i) {
            void *ret;
            pthread_join(threads[i], &ret);
            totol_time += (long) ret;
        }
        long avg_time = totol_time / (OPS_PER_THREAD / 100 * num_thread);
        EXPECT_EQ(2ul * num_thread * OPS_PER_THREAD / 100, counter.load());
        return avg_time;
    }

    static long start_perf_test_with_adder(size_t num_thread) {
        tally::Counter<uint64_t> reducer;
        EXPECT_TRUE(reducer.type().is_counter());
        EXPECT_TRUE(reducer.valid());
        pthread_t threads[num_thread];
        for (size_t i = 0; i < num_thread; ++i) {
            pthread_create(&threads[i], nullptr, &thread_counter, (void *) &reducer);
        }
        long totol_time = 0;
        for (size_t i = 0; i < num_thread; ++i) {
            void *ret = nullptr;
            pthread_join(threads[i], &ret);
            totol_time += (long) ret;
        }
        long avg_time = totol_time / (OPS_PER_THREAD * num_thread);
        EXPECT_EQ(2ul * num_thread * OPS_PER_THREAD, reducer.get_value());
        return avg_time;
    }

    TEST_F(ReducerTest, perf) {
        std::ostringstream oss;
        for (size_t i = 1; i <= 24; ++i) {
            oss << i << '\t' << start_perf_test_with_adder(i) << '\n';
        }
        KLOG(INFO) << "Adder performance:\n" << oss.str();
        oss.str("");
        for (size_t i = 1; i <= 24; ++i) {
            oss << i << '\t' << start_perf_test_with_atomic(i) << '\n';
        }
        KLOG(INFO) << "Atomic performance:\n" << oss.str();
    }

    TEST_F(ReducerTest, Min) {
        tally::MinerGauge<uint64_t> reducer;
        ASSERT_TRUE(reducer.type().is_gauge());
        ASSERT_EQ(std::numeric_limits<uint64_t>::max(), reducer.get_value());
        reducer << 10 << 20;
        ASSERT_EQ(10ul, reducer.get_value());
        reducer << 5;
        ASSERT_EQ(5ul, reducer.get_value());
        reducer << std::numeric_limits<uint64_t>::max();
        ASSERT_EQ(5ul, reducer.get_value());
        reducer << 0;
        ASSERT_EQ(0ul, reducer.get_value());

        tally::MinerGauge<int> reducer2;
        ASSERT_EQ(std::numeric_limits<int>::max(), reducer2.get_value());
        reducer2 << 10 << 20;
        ASSERT_EQ(10, reducer2.get_value());
        reducer2 << -5;
        ASSERT_EQ(-5, reducer2.get_value());
        reducer2 << std::numeric_limits<int>::max();
        ASSERT_EQ(-5, reducer2.get_value());
        reducer2 << 0;
        ASSERT_EQ(-5, reducer2.get_value());
        reducer2 << std::numeric_limits<int>::min();
        ASSERT_EQ(std::numeric_limits<int>::min(), reducer2.get_value());
    }

    TEST_F(ReducerTest, max) {
        tally::MaxerGauge<uint64_t> reducer;
        ASSERT_EQ(std::numeric_limits<uint64_t>::min(), reducer.get_value());
        ASSERT_TRUE(reducer.valid());
        reducer << 20 << 10;
        ASSERT_EQ(20ul, reducer.get_value());
        reducer << 30;
        ASSERT_EQ(30ul, reducer.get_value());
        reducer << 0;
        ASSERT_EQ(30ul, reducer.get_value());

        tally::MaxerGauge<int> reducer2;
        ASSERT_EQ(std::numeric_limits<int>::min(), reducer2.get_value());
        ASSERT_TRUE(reducer2.valid());
        reducer2 << 20 << 10;
        ASSERT_EQ(20, reducer2.get_value());
        reducer2 << 30;
        ASSERT_EQ(30, reducer2.get_value());
        reducer2 << 0;
        ASSERT_EQ(30, reducer2.get_value());
        reducer2 << std::numeric_limits<int>::max();
        ASSERT_EQ(std::numeric_limits<int>::max(), reducer2.get_value());
    }

    tally::Counter<long> g_a;

    TEST_F(ReducerTest, global) {
        ASSERT_TRUE(g_a.valid());
        g_a.get_value();
    }

    void ReducerTest_window() {
        tally::Counter<int> c1;
        tally::MaxerGauge<int> c2;
        tally::MinerGauge<int> c3;
        tally::Window<tally::Counter<int> > w1(&c1, 1);
        tally::Window<tally::Counter<int> > w2(&c1, 2);
        tally::Window<tally::Counter<int> > w3(&c1, 3);
        tally::Window<tally::MaxerGauge<int> > w4(&c2, 1);
        tally::Window<tally::MaxerGauge<int> > w5(&c2, 2);
        tally::Window<tally::MaxerGauge<int> > w6(&c2, 3);
        tally::Window<tally::MinerGauge<int> > w7(&c3, 1);
        tally::Window<tally::MinerGauge<int> > w8(&c3, 2);
        tally::Window<tally::MinerGauge<int> > w9(&c3, 3);

        const int N = 6000;
        int count = 0;
        int total_count = 0;
        int64_t last_time = turbo::Time::current_microseconds();
        for (int i = 1; i <= N; ++i) {
            c1 << 1;
            c2 << N - i;
            c3 << i;
            ++count;
            ++total_count;
            int64_t now = turbo::Time::current_microseconds();
            if (now - last_time >= 1000000L) {
                last_time = now;
                ASSERT_EQ(total_count, c1.get_value());
                KLOG(INFO) << "c1=" << total_count
                          << " count=" << count
                          << " w1=" << w1
                          << " w2=" << w2
                          << " w3=" << w3
                          << " w4=" << w4
                          << " w5=" << w5
                          << " w6=" << w6
                          << " w7=" << w7
                          << " w8=" << w8
                          << " w9=" << w9;
                count = 0;
            } else {
                usleep(950);
            }
        }
    }

    struct Foo {
        int x;

        Foo() : x(0) {}

        explicit Foo(int x2) : x(x2) {}

        void operator+=(const Foo &rhs) {
            x += rhs.x;
        }
    };

    std::ostream &operator<<(std::ostream &os, const Foo &f) {
        return os << "Foo{" << f.x << "}";
    }

    TEST_F(ReducerTest, non_primitive) {
        tally::AdderStatus<Foo> adder;
        adder << Foo(2) << Foo(3) << Foo(4);
        ASSERT_EQ(9, adder.get_value().x);
    }

    bool g_stop = false;
    struct StringAppenderResult {
        int count;
    };

    static void *string_appender(void *arg) {
        tally::AdderStatus<std::string> *cater = (tally::AdderStatus<std::string> *) arg;
        int count = 0;
        std::string id = turbo::str_format("%lld", (long long) pthread_self());
        std::string tmp = "a";
        for (count = 0; !count || !g_stop; ++count) {
            *cater << id << ":";
            for (char c = 'a'; c <= 'z'; ++c) {
                tmp[0] = c;
                *cater << tmp;
            }
            *cater << ".";
        }
        StringAppenderResult *res = new StringAppenderResult;
        res->count = count;
        KLOG(INFO) << "Appended " << count;
        return res;
    }

    TEST_F(ReducerTest, non_primitive_mt) {
        tally::AdderStatus<std::string> cater;
        ASSERT_TRUE(cater.type().is_status());
        ASSERT_TRUE(!cater.type().is_counter());
        ASSERT_TRUE(!cater.type().is_metric());
        pthread_t th[8];
        g_stop = false;
        for (size_t i = 0; i < TURBO_ARRAYSIZE(th); ++i) {
            pthread_create(&th[i], nullptr, string_appender, &cater);
        }
        usleep(50000);
        g_stop = true;
        turbo::flat_hash_map<pthread_t, int> appended_count;
        for (size_t i = 0; i < TURBO_ARRAYSIZE(th); ++i) {
            StringAppenderResult *res = nullptr;
            pthread_join(th[i], (void **) &res);
            appended_count[th[i]] = res->count;
            delete res;
        }
        turbo::flat_hash_map<pthread_t, int> got_count;
        std::string res = cater.get_value();
        std::vector<std::string_view> sps = turbo::str_split(res.c_str(), '.', turbo::SkipEmpty());
        KLOG(INFO) << "res: " << res;
        for (auto sp = sps.begin(); sp != sps.end(); ++sp) {
            std::vector<std::string_view> ps = turbo::str_split(*sp, ":");
            ASSERT_EQ(ps.size(), 2) << "*sp: " << *sp;
            uint64_t tid;
            ASSERT_TRUE(turbo::simple_atoi(ps[0], &tid));
            ++got_count[(pthread_t) tid];
            ASSERT_EQ("abcdefghijklmnopqrstuvwxyz", ps[1]);
        }
        ASSERT_EQ(appended_count.size(), got_count.size());
        for (size_t i = 0; i < TURBO_ARRAYSIZE(th); ++i) {
            ASSERT_EQ(appended_count[th[i]], got_count[th[i]]);
        }
    }

    TEST_F(ReducerTest, simple_window) {
        tally::Counter<int64_t> a;
        tally::Window<tally::Counter<int64_t> > w(&a, 10);
        a << 100;
        sleep(3);
        const int64_t v = w.get_value();
        ASSERT_EQ(100, v) << "v=" << v;
    }
} // namespace
