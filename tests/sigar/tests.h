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

#ifndef __SIGAR_TESTS_H__
#define __SIGAR_TESTS_H__

#ifdef WIN32
#define EOL "\r\n"
#else
#define EOL "\n"
#endif

/**
 * pick the right format for a unsigned 64bit int */
#ifdef WIN32
# define F_U64 "%I64u"
# define F_SIZE_T "%lu"
#else
# ifdef SIGAR_64BIT
#  define F_U64 "%lu"
#  define F_SIZE_T "%lu"
# else
#  define F_U64 "%llu"
#  define F_SIZE_T "%lu"
# endif
#endif

#ifndef RLIM_INFINITY
# define RLIM_INFINITY -1
#endif

#define IS_IMPL_U64(x) \
		(x != (sigar_uint64_t)SIGAR_FIELD_NOTIMPL)
#define IS_IMPL_INT(x) \
		(x != (int)SIGAR_FIELD_NOTIMPL)
#define IS_RLIM_INF(x) \
		(x == (sigar_uint64_t)RLIM_INFINITY)

#define F_IF_VALID_U64 \
		"%s" F_U64

#define NETADDR_IF_IMPL(x, value) \
	if (x.family != SIGAR_AF_UNSPEC) { \
		value = calloc(1, SIGAR_INET6_ADDRSTRLEN + 1); \
		assert(SIGAR_OK == sigar_net_address_to_string(t, &x, value));\
	}



#endif
