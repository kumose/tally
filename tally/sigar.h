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
