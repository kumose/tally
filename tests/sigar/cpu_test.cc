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
#include <sys/types.h>
#include <turbo/log/logging.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <tally/sigar/sigar.h>
#include <tally/sigar/sigar_private.h>
#include <tally/sigar/sigar_format.h>
#include <gtest/gtest.h>
#include "tests.h"

TEST(test_sigar, cpu_get) {
    sigar_t * t;
    ASSERT_TRUE(SIGAR_OK == sigar_open(&t));
    SigarCpu cpu;
    int ret;

    if (SIGAR_OK == (ret = sigar_cpu_get(t, &cpu))) {
        ASSERT_TRUE(IS_IMPL_U64(cpu.user));
        ASSERT_TRUE(IS_IMPL_U64(cpu.sys));
#if !(defined(SIGAR_TEST_OS_AIX))
        ASSERT_TRUE(IS_IMPL_U64(cpu.nice));
#endif
        ASSERT_TRUE(IS_IMPL_U64(cpu.idle));
        ASSERT_TRUE(IS_IMPL_U64(cpu.wait));
        ASSERT_TRUE(IS_IMPL_U64(cpu.total));
    } else {
        switch (ret) {
            /* track the expected error code */
            default:
                fprintf(stderr, "ret = %d (%s)\n", ret, sigar_strerror(t, ret));
                ASSERT_TRUE(ret == SIGAR_OK);
                break;
        }
    }

    sigar_close(t);
}

TEST(test_sigar, cpu_list_get) {
    sigar_t * t;
    ASSERT_TRUE(SIGAR_OK == sigar_open(&t));
    sigar_cpu_list_t cpulist;
    size_t i;
    int ret;

    if (SIGAR_OK != (ret = sigar_cpu_list_get(t, &cpulist))) {
        switch (ret) {
            /* track the expected error code */
            default:
                fprintf(stderr, "ret = %d (%s)\n", ret, sigar_strerror(t, ret));
                ASSERT_TRUE(ret == SIGAR_OK);
                break;
        }
    }
    KLOG(INFO)<<"cpulist.number: "<<cpulist.number;
    for (i = 0; i < cpulist.number; i++) {
        SigarCpu cpu = cpulist.data[i];

        ASSERT_TRUE(IS_IMPL_U64(cpu.user));
        ASSERT_TRUE(IS_IMPL_U64(cpu.user));
        ASSERT_TRUE(IS_IMPL_U64(cpu.sys));
#if !(defined(SIGAR_TEST_OS_AIX))
        ASSERT_TRUE(IS_IMPL_U64(cpu.nice));
#endif
        ASSERT_TRUE(IS_IMPL_U64(cpu.idle));
        ASSERT_TRUE(IS_IMPL_U64(cpu.wait));
        ASSERT_TRUE(IS_IMPL_U64(cpu.total));
    }

    sigar_cpu_list_destroy(t, &cpulist);

    sigar_close(t);
}

TEST(test_sigar, cpu_info_get) {
    sigar_t * t;
    ASSERT_TRUE(SIGAR_OK == sigar_open(&t));
    sigar_cpu_info_list_t cpuinfo;
    size_t i;

    ASSERT_TRUE(SIGAR_OK == sigar_cpu_info_list_get(t, &cpuinfo));

    for (i = 0; i < cpuinfo.number; i++) {
        SigarCpuInfo info = cpuinfo.data[i];

        ASSERT_TRUE(info.vendor);
        ASSERT_TRUE(info.model);
#if !(defined(SIGAR_TEST_OS_DARWIN))
        /* freebsd doesn't always expose it */
        ASSERT_TRUE(IS_IMPL_INT(info.mhz));
#endif
#if !(defined(SIGAR_TEST_OS_DARWIN) || defined(SIGAR_TEST_OS_SOLARIS) || defined(SIGAR_TEST_OS_HPUX) || defined(_WIN32))
        /* freebsd, solaris, hpux nor win32 do expose it */
        ASSERT_TRUE(IS_IMPL_U64(info.cache_size));
#endif
    }

    sigar_cpu_info_list_destroy(t, &cpuinfo);

    sigar_close(t);
}
