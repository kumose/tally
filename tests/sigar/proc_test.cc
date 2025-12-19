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

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#include "tests.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#if defined(MSVC)
#include <WinError.h>
#endif

#include <tally/sigar/sigar.h>
#include <tally/sigar/sigar_private.h>
#include <tally/sigar/sigar_format.h>
#include <gtest/gtest.h>

#ifdef HAVE_VALGRIND_VALGRIND_H
#include <valgrind/valgrind.h>
#else
#define RUNNING_ON_VALGRIND 0
#endif

TEST(test_sigar, proc_stat_get) {
    sigar_t * t;
    ASSERT_TRUE(SIGAR_OK == sigar_open(&t));
    SigarProcStat proc_stat;

    ASSERT_TRUE(SIGAR_OK == sigar_proc_stat_get(t, &proc_stat));
    ASSERT_TRUE(proc_stat.total > 0);

    sigar_close(t);
}

TEST(test_sigar, proc_list_get) {
    sigar_t * t;
    ASSERT_TRUE(SIGAR_OK == sigar_open(&t));
    sigar_proc_list_t proclist;
    size_t i;

    ASSERT_TRUE(SIGAR_OK == sigar_proc_list_get(t, &proclist));
    ASSERT_TRUE(proclist.number > 0);

    for (i = 0; i < proclist.number; i++) {
        sigar_pid_t pid = proclist.data[i];
        SigarProcMem proc_mem;
        SigarProcTime proc_time;
        SigarProcState proc_state;
        int ret;

        if (SIGAR_OK == (ret = sigar_proc_mem_get(t, pid, &proc_mem))) {
            ASSERT_TRUE(IS_IMPL_U64(proc_mem.size));
            ASSERT_TRUE(IS_IMPL_U64(proc_mem.resident));
#if !(defined(SIGAR_TEST_OS_DARWIN) || defined(SIGAR_TEST_OS_SOLARIS) || defined(_WIN32))
            /* MacOS X, solaris nor win32 do provide them */
            ASSERT_TRUE(IS_IMPL_U64(proc_mem.share));
            ASSERT_TRUE(IS_IMPL_U64(proc_mem.minor_faults));
            ASSERT_TRUE(IS_IMPL_U64(proc_mem.major_faults));
#endif
#if !(defined(SIGAR_TEST_OS_DARWIN))
            /* freebsd */
            ASSERT_TRUE(IS_IMPL_U64(proc_mem.page_faults));
#endif
        } else {
            switch (ret) {
                case ESRCH:
                case EPERM:
                    /* track the expected error code */
                    break;
#if (defined(SIGAR_TEST_OS_DARWIN))
                    /* valgrind on macosx doesn't handle this syscall yet */
                case ENOSYS:
                    if (RUNNING_ON_VALGRIND) {
                        break;
                    }
#endif
                default:
                    fprintf(stderr, "ret = %d (%s)\n", ret, sigar_strerror(t, ret));
                    ASSERT_TRUE(ret == SIGAR_OK);
                    break;
            }
        }

        if (SIGAR_OK == (ret = sigar_proc_time_get(t, pid, &proc_time))) {
            ASSERT_TRUE(IS_IMPL_U64(proc_time.start_time));
            ASSERT_TRUE(IS_IMPL_U64(proc_time.user));
            ASSERT_TRUE(IS_IMPL_U64(proc_time.sys));
            ASSERT_TRUE(IS_IMPL_U64(proc_time.total));

#if !(defined(SIGAR_TEST_OS_DARWIN))
            /* Freebsd */
            ASSERT_TRUE(proc_time.start_time > 0);
#endif
            ASSERT_TRUE(proc_time.user >= 0);
            ASSERT_TRUE(proc_time.sys >= 0);
            ASSERT_TRUE(proc_time.total == proc_time.user + proc_time.sys);
        } else {
            switch (ret) {
                case EPERM:
                case ESRCH:
#if (defined(MSVC))
                    /* OpenProcess() may return ERROR_ACCESS_DENIED */
                    case ERROR_ACCESS_DENIED:
#endif
                    /* track the expected error code */
                    break;
#if (defined(SIGAR_TEST_OS_DARWIN))
                    /* valgrind on macosx doesn't handle this syscall yet */
                case ENOSYS:
                    if (RUNNING_ON_VALGRIND) {
                        break;
                    }
#endif
                default:
                    fprintf(stderr, "ret = %d (%s)\n", ret, sigar_strerror(t, ret));
                    ASSERT_TRUE(ret == SIGAR_OK);
                    break;
            }
        }
        if (SIGAR_OK == sigar_proc_state_get(t, pid, &proc_state)) {
            ASSERT_TRUE(proc_state.name != NULL);
#if 0
            /* all values are fine */
            (proc_state.state); /* we should check if the state is one of the announced group */
            (proc_state.ppid);
            (proc_state.tty);
            (proc_state.priority);
            (proc_state.nice);
            (proc_state.processor);
#endif
#if !(defined(SIGAR_TEST_OS_DARWIN) || defined(SIGAR_TEST_OS_LINUX))
            /* MacOS X doesn't provide them, Linux-IA64 neither */
            ASSERT_TRUE(IS_IMPL_U64(proc_state.threads));
#endif
        } else {
            switch (ret) {
                /* track the expected error code */
#if (defined(SIGAR_TEST_OS_DARWIN))
                /* valgrind on macosx doesn't handle this syscall yet */
            case ENOSYS:
                if (RUNNING_ON_VALGRIND) {
                    break;
                }
#endif
                default:
                    fprintf(stderr, "ret = %d (%s)\n", ret, sigar_strerror(t, ret));
                    ASSERT_TRUE(ret == SIGAR_OK);
                    break;
            }
        }
    }

    sigar_proc_list_destroy(t, &proclist);

    sigar_close(t);
}
