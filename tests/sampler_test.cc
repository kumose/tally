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

namespace {

    TEST(SamplerTest, linked_list) {
        turbo::LinkNode<tally::detail::Sampler> n1, n2;
        n1.InsertBeforeAsList(&n2);
        ASSERT_EQ(n1.next(), &n2);
        ASSERT_EQ(n1.previous(), &n2);
        ASSERT_EQ(n2.next(), &n1);
        ASSERT_EQ(n2.previous(), &n1);

        turbo::LinkNode<tally::detail::Sampler> n3, n4;
        n3.InsertBeforeAsList(&n4);
        ASSERT_EQ(n3.next(), &n4);
        ASSERT_EQ(n3.previous(), &n4);
        ASSERT_EQ(n4.next(), &n3);
        ASSERT_EQ(n4.previous(), &n3);

        n1.InsertBeforeAsList(&n3);
        ASSERT_EQ(n1.next(), &n2);
        ASSERT_EQ(n2.next(), &n3);
        ASSERT_EQ(n3.next(), &n4);
        ASSERT_EQ(n4.next(), &n1);
        ASSERT_EQ(&n1, n2.previous());
        ASSERT_EQ(&n2, n3.previous());
        ASSERT_EQ(&n3, n4.previous());
        ASSERT_EQ(&n4, n1.previous());
    }

    class DebugSampler : public tally::detail::Sampler {
    public:
        DebugSampler() : _ncalled(0) {}

        ~DebugSampler() {
            ++_s_ndestroy;
        }

        void take_sample() {
            ++_ncalled;
        }

        int called_count() const { return _ncalled; }

    private:
        int _ncalled;
        static int _s_ndestroy;
    };

    int DebugSampler::_s_ndestroy = 0;

    TEST(SamplerTest, single_threaded) {
        const int N = 100;
        DebugSampler *s[N];
        for (int i = 0; i < N; ++i) {
            s[i] = new DebugSampler;
            s[i]->schedule();
        }
        usleep(1010000);
        for (int i = 0; i < N; ++i) {
            // LE: called once every second, may be called more than once
            ASSERT_LE(1, s[i]->called_count()) << "i=" << i;
        }
        EXPECT_EQ(0, DebugSampler::_s_ndestroy);
        for (int i = 0; i < N; ++i) {
            s[i]->destroy();
        }
        usleep(1010000);
        EXPECT_EQ(N, DebugSampler::_s_ndestroy);
    }

    static void *check(void *) {
        const int N = 100;
        DebugSampler *s[N];
        for (int i = 0; i < N; ++i) {
            s[i] = new DebugSampler;
            s[i]->schedule();
        }
        usleep(1010000);
        for (int i = 0; i < N; ++i) {
            EXPECT_LE(1, s[i]->called_count()) << "i=" << i;
        }
        for (int i = 0; i < N; ++i) {
            s[i]->destroy();
        }
        return nullptr;
    }

    TEST(SamplerTest, multi_threaded) {
        pthread_t th[10];
        DebugSampler::_s_ndestroy = 0;
        for (size_t i = 0; i < TURBO_ARRAYSIZE(th); ++i) {
            ASSERT_EQ(0, pthread_create(&th[i], nullptr, check, nullptr));
        }
        for (size_t i = 0; i < TURBO_ARRAYSIZE(th); ++i) {
            ASSERT_EQ(0, pthread_join(th[i], nullptr));
        }
        sleep(1);
        EXPECT_EQ(100 * TURBO_ARRAYSIZE(th), (size_t) DebugSampler::_s_ndestroy);
    }
} // namespace
