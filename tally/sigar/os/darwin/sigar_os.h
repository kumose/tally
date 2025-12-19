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

#pragma once
#ifdef __MACH__
#ifdef DARWIN
#include <mach/port.h>
#include <mach/host_info.h>
#ifdef DARWIN_HAS_LIBPROC_H
#include <mach-o/dyld.h>
#include <libproc.h>
typedef int (*proc_pidinfo_func_t)(int, int, uint64_t,  void *, int);
typedef int (*proc_pidfdinfo_func_t)(int, int, int, void *, int); 
#endif
#else
#include <kvm.h>
#endif

#ifdef __NetBSD__
#include <sys/param.h>
#endif
#include <sys/sysctl.h>

enum {
    KOFFSET_CPUINFO,
    KOFFSET_VMMETER,
#if defined(__OpenBSD__) || defined(__NetBSD__)
    KOFFSET_TCPSTAT,
    KOFFSET_TCBTABLE,
#endif
    KOFFSET_MAX
};

#if defined(__OpenBSD__) || defined(__NetBSD__)
typedef struct kinfo_proc2 bsd_pinfo_t;
#else
typedef struct kinfo_proc bsd_pinfo_t;
#endif

struct sigar_t {
    SIGAR_T_BASE;
    int pagesize;
    time_t last_getprocs;
    sigar_pid_t last_pid;
    bsd_pinfo_t *pinfo;
    int lcpu;
    size_t argmax;
#ifdef DARWIN
    mach_port_t mach_port;
#  ifdef DARWIN_HAS_LIBPROC_H
    void *libproc;
    proc_pidinfo_func_t proc_pidinfo;
    proc_pidfdinfo_func_t proc_pidfdinfo;
#  endif
#else
    kvm_t *kmem;
    /* offsets for seeking on kmem */
    unsigned long koffsets[KOFFSET_MAX];
    int proc_mounted;
#endif
};

#define SIGAR_EPERM_KMEM (SIGAR_OS_START_ERROR+EACCES)
#define SIGAR_EPROC_NOENT (SIGAR_OS_START_ERROR+2)
#endif