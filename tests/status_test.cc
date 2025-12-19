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

#include <cstddef>
#include <memory>
#include <iostream>
#include <sstream>
#include <gtest/gtest.h>
#include <tally/tally.h>

namespace {
    class StatusTest : public testing::Test {
    protected:
        void SetUp() {
            scope = tally::ScopeInstance::instance()->get_default();
        }

        void TearDown() {
            ASSERT_EQ(0UL, tally::Variable::count_exposed());
        }

        std::shared_ptr<tally::Scope> scope;
    };

    TEST_F(StatusTest, status) {
        tally::Status<std::string> st1;
        st1.set_value("hello %d", 9);
        std::any v1;
        st1.get_value(&v1);
        ASSERT_EQ("hello 9", std::any_cast<std::string>(v1));
        ASSERT_TRUE(st1.expose("var1", "h1", scope.get()).ok());
        ASSERT_EQ("hello 9", tally::Variable::describe_exposed("km_var1"));
        ASSERT_EQ("\"hello 9\"", tally::Variable::describe_exposed("km_var1", true));

        std::vector<std::string> vars;
        tally::Variable::list_exposed(&vars);
        ASSERT_EQ(1UL, vars.size());
        ASSERT_EQ("km_var1", vars[0]);
        ASSERT_EQ(1UL, tally::Variable::count_exposed());

        tally::Status<std::string> st2;
        st2.set_value("world %d", 10);
        ASSERT_FALSE(st2.expose("var1", "h", scope.get()).ok());
        ASSERT_EQ(1UL, tally::Variable::count_exposed());
        ASSERT_EQ("world 10", st2.get_description());
        ASSERT_EQ("hello 9", tally::Variable::describe_exposed("km_var1"));
        ASSERT_EQ(1UL, tally::Variable::count_exposed());

        ASSERT_TRUE(st1.hide());
        ASSERT_EQ(0UL, tally::Variable::count_exposed());
        ASSERT_EQ("", tally::Variable::describe_exposed("km_var1"));
        ASSERT_TRUE(st1.expose("var1", "h", scope.get()).ok());
        ASSERT_EQ(1UL, tally::Variable::count_exposed());
        ASSERT_EQ("hello 9",
                  tally::Variable::describe_exposed("km_var1"));

        ASSERT_TRUE(st2.expose("var2", "h", scope.get()).ok());
        ASSERT_EQ(2UL, tally::Variable::count_exposed());
        ASSERT_EQ("hello 9", tally::Variable::describe_exposed("km_var1"));
        ASSERT_EQ("world 10", tally::Variable::describe_exposed("km_var2"));
        tally::Variable::list_exposed(&vars);
        ASSERT_EQ(2UL, vars.size());
        ASSERT_EQ("km_var1", vars[0]);
        ASSERT_EQ("km_var2", vars[1]);

        ASSERT_TRUE(st2.hide());
        ASSERT_EQ(1UL, tally::Variable::count_exposed());
        ASSERT_EQ("", tally::Variable::describe_exposed("km_var2"));
        tally::Variable::list_exposed(&vars);
        ASSERT_EQ(1UL, vars.size());
        ASSERT_EQ("km_var1", vars[0]);

        ASSERT_TRUE(st2.expose("var2 again", "h", scope.get()).ok());
        ASSERT_EQ("world 10",tally::Variable::describe_exposed("km_var2_again"));
        tally::Variable::list_exposed(&vars);
        ASSERT_EQ(2UL, vars.size());
        ASSERT_EQ("km_var1", vars[0]);
        ASSERT_EQ("km_var2_again", vars[1]);
        ASSERT_EQ(2UL, tally::Variable::count_exposed());

        tally::Status<std::string> st3("var3", "h", "foobar", scope.get());
        ASSERT_EQ("var3", st3.name());
        ASSERT_EQ(3UL, tally::Variable::count_exposed());
        ASSERT_EQ("foobar", tally::Variable::describe_exposed("km_var3"));
        tally::Variable::list_exposed(&vars);
        std::sort(vars.begin(), vars.end());
        ASSERT_EQ(3UL, vars.size());
        ASSERT_EQ("km_var1", vars[0]);
        ASSERT_EQ("km_var2_again", vars[1]);
        ASSERT_EQ("km_var3", vars[2]);
        ASSERT_EQ(3UL, tally::Variable::count_exposed());

        tally::Gauge<int> st4("var4", "h", 9, scope.get());
        ASSERT_EQ("var4", st4.name());
        ASSERT_EQ(4UL, tally::Variable::count_exposed());
        std::any v4;
        st4.get_value(&v4);
        ASSERT_EQ(9, std::any_cast<int>(v4));
        ASSERT_EQ("9", tally::Variable::describe_exposed("km_var4"));
        tally::Variable::list_exposed(&vars);
        std::sort(vars.begin(), vars.end());
        ASSERT_EQ(4UL, vars.size());
        ASSERT_EQ("km_var1", vars[0]);
        ASSERT_EQ("km_var2_again", vars[1]);
        ASSERT_EQ("km_var3", vars[2]);
        ASSERT_EQ("km_var4", vars[3]);

        tally::Status<void *> st5((void *) 19UL);
        KLOG(INFO) << st5;
        std::any v5;
        st5.get_value(&v5);
        ASSERT_EQ((void*)19UL, std::any_cast<void*>(v5));
        ASSERT_EQ("0x13", st5.get_description());

    }


TEST_F(StatusTest, passive_status) {
        auto v = 9UL;
    tally::BasicPassiveStatus<std::string> st1("var11", "help", [v](std::ostream& os) {
        os << (void*)v;
    }, scope.get());
    KLOG(INFO) << st1;
    std::any v1;
    st1.get_value(&v1);
    ASSERT_EQ("0x9", std::any_cast<std::string>(v1));
    std::ostringstream ss;
        ASSERT_TRUE(tally::Variable::describe_exposed("km_var11", ss).ok());
    ASSERT_EQ("0x9", ss.str());
    std::vector<std::string> vars;
        tally::Variable::list_exposed(&vars);
    ASSERT_EQ(1UL, vars.size());
    ASSERT_EQ("km_var11", vars[0]);
    ASSERT_EQ(1UL, tally::Variable::count_exposed());

    int64_t tmp2 = 9;
    tally::FuncGauge<int64_t> st2("var12","help", [&tmp2](){
        return tmp2;
    }, scope.get());
    std::any v2;
    st2.get_value(&v2);
    try {
        std::any_cast<int32_t>(v2);
        ASSERT_TRUE(false);
    } catch (std::bad_any_cast & e) {
        KLOG(INFO) << "Casting int64_t to int32_t throws.";
    }
    ASSERT_EQ(9, std::any_cast<int64_t>(v2));
    ss.str("");
    ASSERT_TRUE(tally::Variable::describe_exposed("km_var12", ss).ok());
    ASSERT_EQ("9", ss.str());
    tally::Variable::list_exposed(&vars);
    for(auto it : vars) {
        KLOG(INFO)<<NOPREFIX<<it;
    }
    ASSERT_EQ(2UL, vars.size());
    ASSERT_EQ("km_var11", vars[0]);
    ASSERT_EQ("km_var12", vars[1]);
    ASSERT_EQ(2UL, tally::Variable::count_exposed());
}

struct Foo {
    int x;
    Foo() : x(0) {}
    explicit Foo(int x2) : x(x2) {}
    Foo operator+(const Foo& rhs) const {
        return Foo(x + rhs.x);
    }
};

std::ostream& operator<<(std::ostream& os, const Foo& f) {
    return os << "Foo{" << f.x << "}";
}

TEST_F(StatusTest, non_primitive) {
    tally::Status<Foo> st;
    ASSERT_EQ(0, st.get_value().x);
    st.set_value(Foo(1));
    ASSERT_EQ(1, st.get_value().x);
    std::any a1;
    st.get_value(&a1);
    ASSERT_EQ(1, std::any_cast<Foo>(a1).x);
}
} // namespace
