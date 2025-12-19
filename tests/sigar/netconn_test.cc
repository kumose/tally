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

TEST(test_sigar, net_connections_get) {
    sigar_t * t;
    ASSERT_TRUE(SIGAR_OK == sigar_open(&t));
    sigar_net_connection_list_t connlist;
    size_t i;
    int ret;

    if (SIGAR_OK == (ret = sigar_net_connection_list_get(t, &connlist,
                                                         SigarNetConnType::SIGAR_NETCONN_SERVER | SigarNetConnType::SIGAR_NETCONN_CLIENT |
                                                                 SigarNetConnType::SIGAR_NETCONN_TCP | SigarNetConnType::SIGAR_NETCONN_UDP))) {
        ASSERT_TRUE(connlist.number > 0);

        for (i = 0; i < connlist.number; i++) {
            sigar_net_connection_t con = connlist.data[i];

            ASSERT_TRUE(con.local_port < 65536);
            ASSERT_TRUE(con.local_port < 65536);
            ASSERT_TRUE(con.uid >= 0);
            ASSERT_TRUE(con.inode >= 0);
            ASSERT_TRUE(static_cast<int>(con.type) >= 0);
            ASSERT_TRUE(con.state >= 0);
            ASSERT_TRUE(con.send_queue >= 0);
            ASSERT_TRUE(con.receive_queue >= 0);
        }

        ASSERT_TRUE(SIGAR_OK == sigar_net_connection_list_destroy(t, &connlist));
    } else {
        switch (ret) {
            case 40013:  /* AIX: SIGAR_EPERM_KMEM */
                /* track the expected error code */
                break;
            default:
                fprintf(stderr, "ret = %d (%s)\n", ret, sigar_strerror(t, ret));
                ASSERT_TRUE(ret == SIGAR_OK);
                break;
        }
    }

    sigar_close(t);
}
