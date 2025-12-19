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
#ifndef SIGAR_FORMAT_H
#define SIGAR_FORMAT_H

typedef struct {
    double user;
    double sys;
    double nice;
    double idle;
    double wait;
    double irq;
    double soft_irq;
    double stolen;
    double combined;
} sigar_cpu_perc_t;

SIGAR_DECLARE(int) sigar_cpu_perc_calculate(SigarCpu *prev,
                                            SigarCpu *curr,
                                            sigar_cpu_perc_t *perc);

SIGAR_DECLARE(int) sigar_uptime_string(sigar_t *sigar, 
                                       sigar_uptime_t *uptime,
                                       char *buffer,
                                       int buflen);

SIGAR_DECLARE(char *) sigar_format_size(sigar_uint64_t size, char *buf);

SIGAR_DECLARE(int) sigar_net_address_equals(SigarNetAddress *addr1,
                                            SigarNetAddress *addr2);

SIGAR_DECLARE(int) sigar_net_address_to_string(sigar_t *sigar,
                                               SigarNetAddress *address,
                                               char *addr_str);

SIGAR_DECLARE(const char *)sigar_net_scope_to_string(int type);

SIGAR_DECLARE(sigar_uint32_t) sigar_net_address_hash(SigarNetAddress *address);

SIGAR_DECLARE(const char *)sigar_net_connection_type_get(int type);

SIGAR_DECLARE(const char *)sigar_net_connection_state_get(int state);

SIGAR_DECLARE(char *) sigar_net_interface_flags_to_string(sigar_uint64_t flags, char *buf);

SIGAR_DECLARE(char *)sigar_net_services_name_get(sigar_t *sigar,
                                                 SigarNetConnType protocol, unsigned long port);

#endif

