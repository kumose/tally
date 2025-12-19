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

#include <tally/sigar.h>
#include <thread>

namespace tally {

    Sigar::Sigar() {
        sigar_open(&_ar);
    }

    Sigar::~Sigar() {
        if(_ar) {
            sigar_close(_ar);
            _ar = nullptr;
        }
    }

    sigar_pid_t Sigar::get_pid() {
        return  sigar_pid_get(_ar);
    }

    turbo::Status Sigar::proc_kill(sigar_pid_t pid, int signum) {
        auto r = sigar_proc_kill(pid, signum);
        if(r != 0 ) {
            return turbo::errno_to_status(r, "");
        }
        return turbo::OkStatus();
    }

    turbo::Result<int> Sigar::get_signum(std::string_view sig) {
        return sigar_signum_get(sig);
    }

    turbo::Status Sigar::get_mem(SigarMem *mem) {
        return sigar_mem_get(_ar, mem);
    }

    turbo::Status Sigar::get_swap(SigarSwap *swap) {
        return sigar_swap_get(_ar, swap);
    }

    turbo::Status Sigar::get_cpu(SigarCpu *cpu) {
        auto r = sigar_cpu_get(_ar, cpu);
        if(r != 0) {
            return turbo::errno_to_status(r, "");
        }
        return turbo::OkStatus();
    }

    turbo::Status Sigar::get_cpu_list(std::vector<SigarCpu> *list) {
        sigar_cpu_list_t cpulist;
        auto ret = sigar_cpu_list_get(_ar, &cpulist);
        if(ret != 0) {
            sigar_cpu_list_destroy(_ar, &cpulist);
            return turbo::errno_to_status(ret, "");
        }
        list->clear();
        list->reserve(cpulist.number);
        for(size_t i = 0; i < cpulist.number; i++) {
            list->push_back(cpulist.data[i]);
        }
        sigar_cpu_list_destroy(_ar, &cpulist);
        return turbo::OkStatus();
    }

    turbo::Status Sigar::get_cpu_info_list(std::vector<SigarCpuInfo> *infos) {
        sigar_cpu_info_list_t infolist;
        auto ret = sigar_cpu_info_list_get(_ar, &infolist);
        if(ret != 0) {
            sigar_cpu_info_list_destroy(_ar, &infolist);
            return turbo::errno_to_status(ret, "");
        }
        infos->clear();
        infos->reserve(infolist.number);
        for(size_t i = 0; i < infolist.number; i++) {
            infos->push_back(infolist.data[i]);
        }
        sigar_cpu_info_list_destroy(_ar, &infolist);
        return turbo::OkStatus();
    }

    turbo::Result<double> Sigar::get_uptime() {
        sigar_uptime_t up;
        auto ret = sigar_uptime_get(_ar, &up);
        if(ret != 0) {
            return turbo::errno_to_status(ret, "");
        }
        return up.uptime;
    }

    turbo::Result<SigarLoadavg> Sigar::get_loadavg() {
        SigarLoadavg ld;
        auto ret = sigar_loadavg_get(_ar, &ld);
        if(ret != 0) {
            return turbo::errno_to_status(ret, "");
        }
        return ld;
    }

    turbo::Status Sigar::get_resource_limit(SigarResourceLimit *rl) {
        auto ret = sigar_resource_limit_get(_ar, rl);
        if(ret != 0) {
            return turbo::errno_to_status(ret, "");
        }
        return turbo::OkStatus();
    }

    turbo::Status Sigar::get_proc_list(std::vector<sigar_pid_t> &pids) {
        sigar_proc_list_t list;
        auto ret = sigar_proc_list_get(_ar, &list);
        if(ret != 0) {
            sigar_proc_list_destroy(_ar, &list);
            return turbo::errno_to_status(ret, "");
        }
        pids.clear();
        pids.reserve(list.number);
        for(size_t i = 0; i < list.number; i++) {
            pids.push_back(list.data[i]);
        }
        sigar_proc_list_destroy(_ar, &list);
        return turbo::OkStatus();
    }

    turbo::Status Sigar::get_proc_stat(SigarProcStat*ps) {
        auto ret = sigar_proc_stat_get(_ar, ps);
        if(ret != 0) {
            return turbo::errno_to_status(ret, "");
        }
        return turbo::OkStatus();
    }

    turbo::Status Sigar::get_proc_mem(sigar_pid_t pid,SigarProcMem *pm) {
        auto ret = sigar_proc_mem_get(_ar, pid, pm);
        if(ret != 0) {
            return turbo::errno_to_status(ret, "");
        }
        return turbo::OkStatus();
    }
    turbo::Status Sigar::get_proc_mem(SigarProcMem *pm) {
        return get_proc_mem(get_pid(), pm);
    }

    turbo::Status Sigar::get_proc_disk_io(sigar_pid_t pid,SigarProcDiskIO *pdio) {
        auto ret = sigar_proc_disk_io_get(_ar, pid, pdio);
        if(ret != 0) {
            return turbo::errno_to_status(ret, "");
        }
        return turbo::OkStatus();
    }

    turbo::Status Sigar::get_proc_disk_io(SigarProcDiskIO *pdio) {
        return get_proc_disk_io(get_pid(), pdio);
    }

    turbo::Status Sigar::get_proc_cumulative_disk_io(sigar_pid_t pid, SigarProcCumulativeDiskIO*pdio) {
        auto ret = sigar_proc_cumulative_disk_io_get(_ar, pid, pdio);
        if(ret != 0) {
            return turbo::errno_to_status(ret, "");
        }
        return turbo::OkStatus();
    }

    turbo::Status Sigar::get_proc_cumulative_disk_io(SigarProcCumulativeDiskIO*pdio) {
        return get_proc_cumulative_disk_io(get_pid(), pdio);
    }

    turbo::Result<uint64_t> Sigar::get_dump_pid_cache() {
        sigar_dump_pid_cache_t dummy{0};
        auto ret = sigar_dump_pid_cache_get(_ar, &dummy);
        if(ret != 0) {
            return turbo::errno_to_status(ret, "");
        }
        return dummy.dummy;
    }

    turbo::Status Sigar::get_proc_cred(sigar_pid_t pid, SigarProcCred *pc) {
        auto ret = sigar_proc_cred_get(_ar, pid, pc);
        if(ret != 0) {
            return turbo::errno_to_status(ret, "");
        }
        return turbo::OkStatus();
    }

    turbo::Status Sigar::get_proc_cred(SigarProcCred *pc) {
        return get_proc_cred(get_pid(), pc);
    }

    turbo::Status Sigar::get_proc_cred_name(sigar_pid_t pid, SigarProcCredName* pc) {
        sigar_proc_cred_name_t spc{};
        auto ret = sigar_proc_cred_name_get(_ar, pid, &spc);
        if(ret != 0) {
            return turbo::errno_to_status(ret, "");
        }
        pc->group = spc.group;
        pc->user = spc.user;
        return turbo::OkStatus();
    }

    turbo::Status Sigar::get_proc_cred_name(SigarProcCredName* pc) {
        return get_proc_cred_name(get_pid(), pc);
    }

    turbo::Status Sigar::get_proc_time(sigar_pid_t pid, SigarProcTime*time){
        auto ret = sigar_proc_time_get(_ar, pid, time);
        if(ret != 0) {
            return turbo::errno_to_status(ret, "");
        }
        return turbo::OkStatus();
    }

    turbo::Status Sigar::get_proc_time(SigarProcTime*time) {
        return get_proc_time(get_pid(), time);
    }

    turbo::Status Sigar::get_proc_cpu(sigar_pid_t pid, SigarProcCpu *cpu) {
        auto ret = sigar_proc_cpu_get(_ar, pid, cpu);
        if(ret != 0) {
            return turbo::errno_to_status(ret, "");
        }
        return turbo::OkStatus();
    }

    turbo::Status Sigar::get_proc_cpu(SigarProcCpu *cpu) {
        return get_proc_cpu(get_pid(), cpu);
    }

    turbo::Status Sigar::get_proc_state(sigar_pid_t pid, SigarProcState *cpu) {
        auto ret = sigar_proc_state_get(_ar, pid, cpu);
        if(ret != 0) {
            return turbo::errno_to_status(ret, "");
        }
        return turbo::OkStatus();
    }

    turbo::Status Sigar::get_proc_state(SigarProcState *cpu) {
        return get_proc_state(get_pid(), cpu);
    }

    turbo::Result<uint64_t> Sigar::get_proc_fd(sigar_pid_t pid) {
        SigarProcFD pfd;
        auto ret = sigar_proc_fd_get(_ar, pid, &pfd);
        if(ret != 0) {
            return turbo::errno_to_status(ret, "");
        }
        return pfd.total;
    }

    turbo::Result<uint64_t> Sigar::get_proc_fd() {
        return get_proc_fd(get_pid());
    }

    turbo::Status Sigar::get_thread_cpu(uint64_t id, SigarThreadCpu*tc){
        auto ret = sigar_thread_cpu_get(_ar, id, tc);
        if(ret != 0) {
            return turbo::errno_to_status(ret, "");
        }
        return turbo::OkStatus();
    }
    turbo::Status Sigar::get_thread_cpu(SigarThreadCpu*tc) {
        return get_thread_cpu(pthread_self(), tc);
    }


    turbo::Status Sigar::get_file_system_usage(const std::string &dir, SigarFileSystemUsage*usage) {
        auto ret = sigar_file_system_usage_get(_ar, dir.c_str(), usage);
        if(ret != 0) {
            return turbo::errno_to_status(ret, "");
        }
        return turbo::OkStatus();
    }

    turbo::Status Sigar::get_disk_usage(const std::string &dir, SigarDiskUsage*usage) {
        auto ret = sigar_disk_usage_get(_ar, dir.c_str(), usage);
        if(ret != 0) {
            return turbo::errno_to_status(ret, "");
        }
        return turbo::OkStatus();
    }

    turbo::Status Sigar::get_net_stat(SigarNetConnType flag, SigarNetStat*stat) {
        auto ret = sigar_net_stat_get(_ar, stat, flag);
        if(ret != 0) {
            return turbo::errno_to_status(ret, "");
        }
        return turbo::OkStatus();
    }

}  // namespace