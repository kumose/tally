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

#pragma once

#include <tally/sigar/sigar.h>
#include <turbo/utility/status.h>

namespace tally {

    class Sigar {
    public:
        Sigar();

        ~Sigar();

        sigar_pid_t get_pid();

        turbo::Status proc_kill(sigar_pid_t pid, int signum);

        turbo::Result<int> get_signum(std::string_view sig);

        turbo::Status get_mem(SigarMem *mem);

        turbo::Status get_swap(SigarSwap *swap);

        turbo::Status get_cpu(SigarCpu *cpu);

        turbo::Status get_cpu_list(std::vector<SigarCpu> *cpu);

        turbo::Status get_cpu_info_list(std::vector<SigarCpuInfo> *infos);

        turbo::Result<double> get_uptime();

        turbo::Result<SigarLoadavg> get_loadavg();

        turbo::Status get_resource_limit(SigarResourceLimit *rl);

        turbo::Status get_proc_list(std::vector<sigar_pid_t> &pids);

        turbo::Status get_proc_stat(SigarProcStat *ps);

        turbo::Status get_proc_mem(sigar_pid_t pid, SigarProcMem *pm);

        turbo::Status get_proc_mem(SigarProcMem *pm);

        turbo::Status get_proc_disk_io(sigar_pid_t pid, SigarProcDiskIO *pdio);

        turbo::Status get_proc_disk_io(SigarProcDiskIO *pdio);

        turbo::Status get_proc_cumulative_disk_io(sigar_pid_t pid, SigarProcCumulativeDiskIO *pdio);

        turbo::Status get_proc_cumulative_disk_io(SigarProcCumulativeDiskIO *pdio);

        turbo::Result<uint64_t> get_dump_pid_cache();

        turbo::Status get_proc_cred(sigar_pid_t pid, SigarProcCred *pc);

        turbo::Status get_proc_cred(SigarProcCred *pc);

        turbo::Status get_proc_cred_name(sigar_pid_t pid, SigarProcCredName *pc);

        turbo::Status get_proc_cred_name(SigarProcCredName *pc);

        turbo::Status get_proc_time(sigar_pid_t pid, SigarProcTime *time);

        turbo::Status get_proc_time(SigarProcTime *time);

        turbo::Status get_proc_cpu(sigar_pid_t pid, SigarProcCpu *cpu);

        turbo::Status get_proc_cpu(SigarProcCpu *cpu);

        turbo::Status get_proc_state(sigar_pid_t pid, SigarProcState *state);

        turbo::Status get_proc_state(SigarProcState *state);

        turbo::Result<uint64_t> get_proc_fd(sigar_pid_t pid);

        turbo::Result<uint64_t> get_proc_fd();

        turbo::Status get_thread_cpu(uint64_t id, SigarThreadCpu *tc);

        turbo::Status get_thread_cpu(SigarThreadCpu *tc);

        turbo::Status get_file_system_usage(const std::string &dir, SigarFileSystemUsage*usage);

        turbo::Status get_disk_usage(const std::string &dir, SigarDiskUsage*usage);

        turbo::Status get_net_stat(SigarNetConnType flag, SigarNetStat*stat);

    private:
        sigar_t *_ar{nullptr};
    };
}  // namespace tally
