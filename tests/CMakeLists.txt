# Copyright (C) Kumo inc. and its affiliates.
# Author: Jeff.li lijippy@163.com
# All rights reserved.
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

#[[kmcmake_cc_test(
        NAME custom_comparator_test
        SOURCES custom_comparator_test.cc
        DEPS turbo::turbo ${KMCMAKE_DEPS_LINK} ${GTEST_LIB} ${GTEST_MAIN_LIB}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)]]
#[[
kmcmake_cc_test(
        NAME foo_test
        SOURCES foo_test.cc
        DEPS ${PROJECT}::foo ${KMCMAKE_DEPS_LINK} gtest gtest_main
        COPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test(
        NAME foo_doctest
        SOURCES foo_doctest.cc
        DEPS ${PROJECT}::foo ${KMCMAKE_DEPS_LINK} doctest
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)
]]

configure_file(${PROJECT_SOURCE_DIR}/tests/config.h.in ${PROJECT_SOURCE_DIR}/tests/config.h @ONLY)
find_package(GTest REQUIRED)
kmcmake_cc_test(
        NAME raw_test
        MODULE base
        SOURCES buckets_test.cc
        LINKS
        tally::tally_static
        turbo::turbo_static
        GTest::gtest
        GTest::gtest_main
)

kmcmake_cc_test(
        NAME counter_test
        MODULE base
        SOURCES counter_test.cc
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
        NAME gauge_test
        MODULE base
        SOURCES gauge_test.cc
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
        NAME histogram_test
        MODULE base
        SOURCES histogram_test.cc
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
        NAME scope_test
        MODULE base
        SOURCES scope_test.cc
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
        NAME variable_test
        MODULE base
        SOURCES variable_test.cc
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
        NAME timer_test
        MODULE base
        SOURCES timer_test.cc
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
        NAME percentile_test
        MODULE base
        SOURCES percentile_test.cc
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
        NAME reducer_test
        MODULE base
        SOURCES reducer_test.cc
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
        NAME agent_group_test
        MODULE base
        SOURCES agent_group_test.cc
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
        NAME sampler_test
        MODULE base
        SOURCES sampler_test.cc
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
        NAME window_test
        MODULE base
        SOURCES window_test.cc
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
        NAME status_test
        MODULE base
        SOURCES status_test.cc
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
        NAME recorder_test
        MODULE base
        SOURCES recorder_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        tally::tally_static
        turbo::turbo_static
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
)


find_library(PROFILE_LIB NAMES libprofiler.a)
find_path(PROFILE_HDR NAMES gperftools/profiler.h)
find_package(PkgConfig)
pkg_check_modules(libunwind REQUIRED IMPORTED_TARGET GLOBAL libunwind)
if (PROFILE_LIB AND PROFILE_HDR)
    kmcmake_cc_test(
            NAME lock_timer_test
            MODULE base
            SOURCES lock_timer_test.cc
            CXXOPTS
            -fno-access-control
            LINKS
            tally::tally_static
            turbo::turbo_static
            ${PROFILE_LIB}
            PkgConfig::libunwind
            GTest::gtest
            GTest::gmock
            GTest::gtest_main
    )
endif ()
add_subdirectory(sigar)
