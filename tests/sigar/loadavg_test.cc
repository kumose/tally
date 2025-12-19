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

#include <tally/sigar/sigar.h>
#include <tally/sigar/sigar_private.h>
#include <tally/sigar/sigar_format.h>
#include <gtest/gtest.h>

TEST(test_sigar, loadavg_get) {
    sigar_t * t;
    ASSERT_TRUE(SIGAR_OK == sigar_open(&t));
	SigarLoadavg loadavg;
	int ret;

	if (SIGAR_OK == (ret = sigar_loadavg_get(t, &loadavg))) {
		ASSERT_TRUE(loadavg.loadavg[0] >= 0);
		ASSERT_TRUE(loadavg.loadavg[1] >= 0);
		ASSERT_TRUE(loadavg.loadavg[2] >= 0);
	} else {
		switch (ret) {
			/* track the expected error code */
		default:
#if !(defined(_WIN32))
			/* win32 has no loadavg */
			fprintf(stderr, "ret = %d (%s)\n", ret, sigar_strerror(t, ret));
			ASSERT_TRUE(ret == SIGAR_OK); 
#endif
			break;
		}
	}

    sigar_close(t);
}
