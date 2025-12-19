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

#ifndef SIGAR_PTQL_H
#define SIGAR_PTQL_H

#define SIGAR_PTQL_MALFORMED_QUERY -1

typedef struct sigar_ptql_query_t sigar_ptql_query_t;

#define SIGAR_PTQL_ERRMSG_SIZE 1024

typedef struct {
    char message[SIGAR_PTQL_ERRMSG_SIZE];
} sigar_ptql_error_t;

typedef int (*sigar_ptql_re_impl_t)(void *, char *, char *);

SIGAR_DECLARE(void) sigar_ptql_re_impl_set(sigar_t *sigar, void *data,
                                           sigar_ptql_re_impl_t impl);

SIGAR_DECLARE(int) sigar_ptql_query_create(sigar_ptql_query_t **query,
                                           char *ptql,
                                           sigar_ptql_error_t *error);

SIGAR_DECLARE(int) sigar_ptql_query_match(sigar_t *sigar,
                                          sigar_ptql_query_t *query,
                                          sigar_pid_t pid);

SIGAR_DECLARE(int) sigar_ptql_query_destroy(sigar_ptql_query_t *query);

SIGAR_DECLARE(int) sigar_ptql_query_find_process(sigar_t *sigar,
                                                 sigar_ptql_query_t *query,
                                                 sigar_pid_t *pid);

SIGAR_DECLARE(int) sigar_ptql_query_find(sigar_t *sigar,
                                         sigar_ptql_query_t *query,
                                         sigar_proc_list_t *proclist);

#endif /*SIGAR_PTQL_H*/
