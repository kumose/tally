// Copyright (C) Kumo inc. and its affiliates.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
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
#include "tests.h"

TEST(test_sigar, net_iflist_get) {
    sigar_t * t;
    ASSERT_TRUE(SIGAR_OK == sigar_open(&t));
    sigar_net_interface_list_t net_iflist;
    size_t i;
    int ret;

    ASSERT_TRUE(SIGAR_OK == sigar_net_interface_list_get(t, &net_iflist));
    ASSERT_TRUE(net_iflist.number > 0);

    for (i = 0; i < net_iflist.number; i++) {
        char *ifname = net_iflist.data[i];
        sigar_net_interface_stat_t ifstat;
        sigar_net_interface_config_t config;

        if (SIGAR_OK == (ret = sigar_net_interface_stat_get(t, ifname, &ifstat))) {
#if defined(SIGAR_TEST_OS_SOLARIS)
            /* on solaris "lo" has no real stats, skip it */
            if (0 == strncmp(ifname, "lo", 2)) continue;
#endif
            ASSERT_TRUE(IS_IMPL_U64(ifstat.rx_packets));
            ASSERT_TRUE(IS_IMPL_U64(ifstat.rx_bytes));
            ASSERT_TRUE(IS_IMPL_U64(ifstat.rx_errors));
#if !(defined(SIGAR_TEST_OS_AIX))
            ASSERT_TRUE(IS_IMPL_U64(ifstat.rx_dropped));
#endif
#if !(defined(SIGAR_TEST_OS_DARWIN) || defined(SIGAR_TEST_OS_AIX) || defined(SIGAR_TEST_OS_HPUX) || defined(_WIN32))
            ASSERT_TRUE(IS_IMPL_U64(ifstat.rx_overruns));
            ASSERT_TRUE(IS_IMPL_U64(ifstat.rx_frame));
#endif
            ASSERT_TRUE(IS_IMPL_U64(ifstat.tx_packets));
            ASSERT_TRUE(IS_IMPL_U64(ifstat.tx_bytes));
            ASSERT_TRUE(IS_IMPL_U64(ifstat.tx_errors));
#if !(defined(SIGAR_TEST_OS_HPUX) || defined(_WIN32))
            ASSERT_TRUE(IS_IMPL_U64(ifstat.tx_collisions));
#endif
#if !(defined(SIGAR_TEST_OS_DARWIN) || defined(SIGAR_TEST_OS_AIX))
            ASSERT_TRUE(IS_IMPL_U64(ifstat.tx_dropped));
#endif
#if !(defined(SIGAR_TEST_OS_DARWIN) || defined(SIGAR_TEST_OS_AIX) || defined(SIGAR_TEST_OS_HPUX) || defined(_WIN32))
            ASSERT_TRUE(IS_IMPL_U64(ifstat.tx_overruns));
            ASSERT_TRUE(IS_IMPL_U64(ifstat.tx_carrier));
#endif
#ifndef __linux__
            ASSERT_TRUE(IS_IMPL_U64(ifstat.speed));
#endif
        } else {
            switch (ret) {
                /* track the expected error code */
                default:
                    fprintf(stderr, "ret = %d (%s)\n", ret, sigar_strerror(t, ret));
                    ASSERT_TRUE(ret == SIGAR_OK);
                    break;
            }
        }

        if (SIGAR_OK == (ret = sigar_net_interface_config_get(t, ifname, &config))) {
            ASSERT_TRUE(config.name);
            ASSERT_TRUE(config.type);
            ASSERT_TRUE(config.description);
            ASSERT_TRUE(IS_IMPL_U64(config.flags));
            ASSERT_TRUE(IS_IMPL_U64(config.mtu));
            ASSERT_TRUE(IS_IMPL_U64(config.metric));
        } else {
            switch (ret) {
                /* track the expected error code */
                default:
                    fprintf(stderr, "ret = %d (%s)\n", ret, sigar_strerror(t, ret));
                    ASSERT_TRUE(ret == SIGAR_OK);
                    break;
            }
        }
    }

    ASSERT_TRUE(SIGAR_OK == sigar_net_interface_list_destroy(t, &net_iflist));

    sigar_close(t);
}