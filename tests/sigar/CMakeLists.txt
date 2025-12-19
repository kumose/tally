#
# Copyright 2023 The titan-search Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#



kmcmake_cc_test(
        NAME uptime_test
        MODULE sigar
        SOURCES uptime_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        tally::tally_static
        turbo::turbo_static
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
)

kmcmake_cc_test(
        NAME sysinfo_test
        MODULE sigar
        SOURCES sysinfo_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        tally::tally_static
        turbo::turbo_static
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
)

kmcmake_cc_test(
        NAME swap_test
        MODULE sigar
        SOURCES swap_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        tally::tally_static
        turbo::turbo_static
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
)

kmcmake_cc_test(
        NAME reslimit_test
        MODULE sigar
        SOURCES reslimit_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        tally::tally_static
        turbo::turbo_static
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
)

kmcmake_cc_test(
        NAME proc_test
        MODULE sigar
        SOURCES proc_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        tally::tally_static
        turbo::turbo_static
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
)

kmcmake_cc_test(
        NAME pid_test
        MODULE sigar
        SOURCES pid_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        tally::tally_static
        turbo::turbo_static
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
)

kmcmake_cc_test(
        NAME netif_test
        MODULE sigar
        SOURCES netif_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        tally::tally_static
        turbo::turbo_static
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
)

kmcmake_cc_test(
        NAME netconn_test
        MODULE sigar
        SOURCES netconn_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        tally::tally_static
        turbo::turbo_static
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
)

kmcmake_cc_test(
        NAME mem_test
        MODULE sigar
        SOURCES mem_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        tally::tally_static
        turbo::turbo_static
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
)

kmcmake_cc_test(
        NAME loadavg_test
        MODULE sigar
        SOURCES loadavg_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        tally::tally_static
        turbo::turbo_static
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
)

kmcmake_cc_test(
        NAME fs_test
        MODULE sigar
        SOURCES fs_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        tally::tally_static
        turbo::turbo_static
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
)
kmcmake_cc_test(
        NAME cpu_test
        MODULE sigar
        SOURCES cpu_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        tally::tally_static
        turbo::turbo_static
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
)
