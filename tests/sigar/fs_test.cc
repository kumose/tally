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
#include "tests.h"

TEST(test_sigar, file_system_list_get) {
    sigar_t * t;
    ASSERT_TRUE(SIGAR_OK == sigar_open(&t));
	sigar_file_system_list_t fslist;
	size_t i;

	ASSERT_TRUE(SIGAR_OK == sigar_file_system_list_get(t, &fslist));
	ASSERT_TRUE(fslist.number > 0);

	// WARN: docker may not pass this test
	for (i = 0; i < fslist.number; i++) {
		sigar_file_system_t fs = fslist.data[i];
		SigarFileSystemUsage fsusage;
		SigarDiskUsage diskusage;
		int ret;

		ASSERT_TRUE(fs.dir_name);
		ASSERT_TRUE(fs.dev_name);
		ASSERT_TRUE(fs.type_name);
		ASSERT_TRUE(fs.sys_type_name);
		ASSERT_TRUE(fs.type);

		if (SIGAR_OK != (ret = sigar_file_system_ping(t, &fs))) {
			continue;
		}

		if (SIGAR_OK == (ret = sigar_file_system_usage_get(t, fs.dir_name, &fsusage))) {
			ASSERT_TRUE(IS_IMPL_U64(fsusage.total));
			ASSERT_TRUE(IS_IMPL_U64(fsusage.free));
			ASSERT_TRUE(IS_IMPL_U64(fsusage.used));
			ASSERT_TRUE(IS_IMPL_U64(fsusage.avail));
#if !(defined(SIGAR_TEST_OS_SOLARIS) || defined(_WIN32))
			/* solaris 8 */
			ASSERT_TRUE(IS_IMPL_U64(fsusage.files));
#endif
			ASSERT_TRUE(fsusage.use_percent >= 0);
		} else {
			switch (ret) {
				/* track the expected error code */
#if defined(MSVC)
			case ERROR_NOT_READY:
				break;
#endif
			default:
				fprintf(stderr, "sigar_file_system_usage_get(%s) ret = %d (%s)\n",
						fs.dir_name,
						ret, sigar_strerror(t, ret));
				ASSERT_TRUE(ret == SIGAR_OK); 
				break;
			}
		}
	
		if (SIGAR_OK == (ret = sigar_disk_usage_get(t, fs.dev_name, &diskusage))) {
			ASSERT_TRUE(IS_IMPL_U64(diskusage.reads));
			ASSERT_TRUE(IS_IMPL_U64(diskusage.writes));
#if !defined(SIGAR_TEST_OS_DARWIN)
			/* freebsd */
			ASSERT_TRUE(IS_IMPL_U64(diskusage.read_bytes));
			ASSERT_TRUE(IS_IMPL_U64(diskusage.write_bytes));
			ASSERT_TRUE(IS_IMPL_U64(diskusage.rtime));
			ASSERT_TRUE(IS_IMPL_U64(diskusage.wtime));
#endif
#if !(defined(SIGAR_TEST_OS_LINUX) || defined(SIGAR_TEST_OS_DARWIN) || defined(_WIN32))
			/* depending on the Linux version they might not be set */
			ASSERT_TRUE(IS_IMPL_U64(diskusage.qtime));
#endif
#if !(defined(SIGAR_TEST_OS_LINUX) || defined(SIGAR_TEST_OS_DARWIN))
			ASSERT_TRUE(IS_IMPL_U64(diskusage.time));
#endif
#if !defined(SIGAR_TEST_OS_DARWIN)
			ASSERT_TRUE(IS_IMPL_U64(diskusage.snaptime));
#endif
#if 0
			/* is -1 if undefined */
			ASSERT_TRUE(diskusage.service_time >= 0);
			ASSERT_TRUE(diskusage.queue >= 0);
#endif
		} else {
			switch (ret) { 
			case ESRCH: /* macosx */
			case ENXIO: /* solaris */
			case ENOENT: /* aix */
			case SIGAR_ENOTIMPL: /* hpux */
				/* track the expected error code */
				fprintf(stderr, "sigar_disk_usage_get(%s) ret = %d (%s)\n",
						fs.dev_name,
						ret, sigar_strerror(t, ret));
				break;
			default:
				fprintf(stderr, "sigar_disk_usage_get(%s) ret = %d (%s)\n",
						fs.dev_name,
						ret, sigar_strerror(t, ret));
				ASSERT_TRUE(ret == SIGAR_OK); 
				break;
			}
		}
	}

	sigar_file_system_list_destroy(t, &fslist);

    sigar_close(t);
}
