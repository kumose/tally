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

// Includes
#include <tally/utility/task_timer.h>
#include <chrono>
#include <thread>
#include <gtest/gtest.h>
#include <turbo/log/logging.h>

using namespace std::chrono;

TEST(timer, stop_start) {
    {
        tally::TaskTimer t;
    }
}

TEST(timer, add) {
    tally::TaskTimer t;

    {
        int i = 0;
        t.run_after(100000, [&](tally::timer_id) { i = 42; });
        std::this_thread::sleep_for(milliseconds(120));
        ASSERT_TRUE(i == 42);
    }

    {
        int i = 0;
        t.run_after(turbo::Duration::microseconds(100), [&](tally::timer_id) { i = 43; });
        std::this_thread::sleep_for(milliseconds(120));
        ASSERT_TRUE(i == 43);
    }


    {
        int i = 0;
        t.run_at(turbo::Time::current_time() + turbo::Duration::milliseconds(100), [&](tally::timer_id) { i = 44; });
        std::this_thread::sleep_for(milliseconds(120));
        ASSERT_TRUE(i == 44);
    }
}

TEST(timer, three_add) {
    tally::TaskTimer t;

    {
        size_t count = 0;
        auto id = t.run_every(
                100000, [&](tally::timer_id) { ++count; }, 10000);
        std::this_thread::sleep_for(milliseconds(125));
        t.remove(id);
        ASSERT_TRUE(count == 3);
    }

    {
        size_t count = 0;
        auto id = t.run_every(
                turbo::Duration::milliseconds(100), [&](tally::timer_id) { ++count; },turbo::Duration::microseconds(10000));
        std::this_thread::sleep_for(milliseconds(135));
        t.remove(id);
        ASSERT_TRUE(count == 4);
    }
}

TEST(timer, callback) {
    tally::TaskTimer t;

    {
        size_t count = 0;
        t.run_every(
                turbo::Duration::milliseconds(10),
                [&](tally::timer_id id) {
                    ++count;
                    t.remove(id);
                },
                turbo::Duration::milliseconds(10));
        std::this_thread::sleep_for(milliseconds(50));
        ASSERT_TRUE(count == 1);
    }

    {
        auto id1 = t.run_after(turbo::Duration::milliseconds(40), [](tally::timer_id) {});
        auto id2 = t.run_after(turbo::Duration::milliseconds(10), [&](tally::timer_id id) { t.remove(id); });
        std::this_thread::sleep_for(milliseconds(30));
        auto id3 = t.run_after(turbo::Duration::microseconds(100), [](tally::timer_id) {});
        auto id4 = t.run_after(turbo::Duration::microseconds(100), [](tally::timer_id) {});
        ASSERT_TRUE(id3 == id2);
        ASSERT_TRUE(id4 != id1);
        ASSERT_TRUE(id4 != id2);
        std::this_thread::sleep_for(milliseconds(20));
    }

    {
        auto id1 = t.run_after(turbo::Duration::milliseconds(10), [&](tally::timer_id id) { t.remove(id); });
        auto id2 = t.run_after(turbo::Duration::milliseconds(40), [](tally::timer_id) {});
        std::this_thread::sleep_for(milliseconds(30));
        auto id3 = t.run_after(turbo::Duration::microseconds(100), [](tally::timer_id) {});
        auto id4 = t.run_after(turbo::Duration::microseconds(100), [](tally::timer_id) {});
        ASSERT_TRUE(id3 == id1);
        ASSERT_TRUE(id4 != id1);
        ASSERT_TRUE(id4 != id2);
        std::this_thread::sleep_for(milliseconds(20));
    }
}

TEST(timer, timeouts) {
    int i = 0;
    int j = 0;
    tally::TaskTimer t;
    auto ts = turbo::Time::current_time() + turbo::Duration::milliseconds(40);
    t.run_at(ts, [&](tally::timer_id) { i = 42; });
    t.run_at(ts, [&](tally::timer_id) { j = 43; });
    std::this_thread::sleep_for(milliseconds(50));
    ASSERT_TRUE(i == 42);
    ASSERT_TRUE(j == 43);
}

TEST(timer, timeouts_from_passt) {
    tally::TaskTimer t;

    {
        int i = 0;
        int j = 0;
        auto ts1 = turbo::Time::current_time() - turbo::Duration::milliseconds(10);
        auto ts2 = turbo::Time::current_time() - turbo::Duration::milliseconds(20);
        t.run_at(ts1, [&](tally::timer_id) { i = 42; });
        t.run_at(ts2, [&](tally::timer_id) { j = 43; });
        std::this_thread::sleep_for(microseconds(20));
        ASSERT_EQ(i, 42);
        ASSERT_EQ(j , 43);
    }

    {
        int i = 0;
        auto ts1 = turbo::Time::current_time() + turbo::Duration::milliseconds(10);
        auto ts2 = turbo::Time::current_time() + turbo::Duration::milliseconds(20);
        t.run_at(ts1, [&](tally::timer_id) { std::this_thread::sleep_for(milliseconds(20)); });
        t.run_at(ts2, [&](tally::timer_id) { i = 42; });
        std::this_thread::sleep_for(milliseconds(50));
        ASSERT_TRUE(i == 42);
    }
}

TEST(timer, multiple_timeouts) {
    int i = 0;
    tally::TaskTimer t;
    t.run_after(10000, [&](tally::timer_id) { i = 42; });
    t.run_after(20000, [&](tally::timer_id) { i = 43; });
    t.run_after(30000, [&](tally::timer_id) { i = 44; });
    t.run_after(40000, [&](tally::timer_id) { i = 45; });
    std::this_thread::sleep_for(milliseconds(50));
    ASSERT_TRUE(i == 45);
}

TEST(timer, multiple) {
    int i = 0;
    tally::TaskTimer t1;
    tally::TaskTimer t2;

    {
        t1.run_after(turbo::Duration::milliseconds(20), [&](tally::timer_id) { i = 42; });
        t1.run_after(turbo::Duration::milliseconds(40), [&](tally::timer_id) { i = 43; });
        std::this_thread::sleep_for(milliseconds(30));
        ASSERT_TRUE(i == 42);
        std::this_thread::sleep_for(milliseconds(20));
        ASSERT_TRUE(i == 43);
    }

    {
        i = 0;
        auto id1 = t1.run_after(turbo::Duration::milliseconds(20), [&](tally::timer_id) {
            i = 42;
        });
        t1.run_after(turbo::Duration::milliseconds(40), [&](tally::timer_id) { i = 43; });
        std::this_thread::sleep_for(milliseconds(10));
        t1.remove(id1);
        std::this_thread::sleep_for(milliseconds(20));
        ASSERT_EQ(i, 0);
        std::this_thread::sleep_for(milliseconds(20));
        ASSERT_TRUE(i == 43);
    }
}

TEST(timer, remove_timer_id) {
    tally::TaskTimer t;


    {
        auto id = t.run_after(turbo::Duration::milliseconds(20), [](tally::timer_id) {});
        std::this_thread::sleep_for(microseconds(10));
        auto res = t.remove(id + 1);
        ASSERT_TRUE(res == false);
    }


    {
        auto shared = std::make_shared<int>(10);
        tally::handler_t func = [=](tally::timer_id) { auto shared2 = shared; };
        auto id = t.run_after(turbo::Duration::milliseconds(20), std::move(func));
        ASSERT_TRUE(shared.use_count() == 2); // shared is copied
        std::this_thread::sleep_for(microseconds(10));
        auto res = t.remove(id);
        ASSERT_TRUE(res == true);
        ASSERT_TRUE(shared.use_count() == 1); // shared in the lambda is cleaned.
    }


    {
        auto shared = std::make_shared<int>(10);
        tally::handler_t func = [=](tally::timer_id) { auto shared2 = shared; };
        t.run_after(turbo::Duration::milliseconds(20), std::move(func));
        ASSERT_TRUE(shared.use_count() == 2); // shared is copied
        std::this_thread::sleep_for(milliseconds(30));
        ASSERT_TRUE(shared.use_count() == 1); // shared in the lambda is cleaned.
    }
}

TEST(timer, action) {
    struct PushMe {
        int i{0};
    };
    auto push_me = std::make_shared<PushMe>();
    push_me->i = 41;

    tally::TaskTimer t;
    int res = 0;

    // Share the shared_ptr with the lambda
    t.run_after(turbo::Duration::milliseconds(20), [&res, push_me](tally::timer_id) { res = push_me->i + 1; });

    ASSERT_TRUE(res == 0);
    std::this_thread::sleep_for(milliseconds(30));
    ASSERT_TRUE(res == 42);
}