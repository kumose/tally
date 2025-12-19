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

#include <tally/sigar_metric.h>
#include <tally/config.h>
#include <tally/scope.h>
#include <mutex>

namespace tally {

    std::atomic<bool> SigarMetric::is_exposed{false};

    SigarMetric::SigarMetric()
            : mem_ram([]() -> int64_t {
        Sigar ar;
        SigarMem mem;
        auto rs = ar.get_mem(&mem);
        if (!rs.ok()) {
            return 0;
        }
        return mem.ram;
    }),
              mem_total([]() -> int64_t {
                  Sigar ar;
                  SigarMem mem;
                  auto rs = ar.get_mem(&mem);
                  if (!rs.ok()) {
                      return 0;
                  }
                  return mem.total;
              }),
              mem_used([]() -> int64_t {
                  Sigar ar;
                  SigarMem mem;
                  auto rs = ar.get_mem(&mem);
                  if (!rs.ok()) {
                      return 0;
                  }
                  return mem.used;
              }),
              mem_free([]() -> int64_t {
                  Sigar ar;
                  SigarMem mem;
                  auto rs = ar.get_mem(&mem);
                  if (!rs.ok()) {
                      return 0;
                  }
                  return mem.free;
              }),
              mem_actual_used([]() -> int64_t {
                  Sigar ar;
                  SigarMem mem;
                  auto rs = ar.get_mem(&mem);
                  if (!rs.ok()) {
                      return 0;
                  }
                  return mem.actual_used;
              }),
              mem_actual_free([]() -> int64_t {
                  Sigar ar;
                  SigarMem mem;
                  auto rs = ar.get_mem(&mem);
                  if (!rs.ok()) {
                      return 0;
                  }
                  return mem.actual_free;
              }),
              swap_total([]() -> int64_t {
                  Sigar ar;
                  SigarSwap swap;
                  auto rs = ar.get_swap(&swap);
                  if (!rs.ok()) {
                      return 0;
                  }
                  return swap.total;
              }),
              swap_used([]() -> int64_t {
                  Sigar ar;
                  SigarSwap swap;
                  auto rs = ar.get_swap(&swap);
                  if (!rs.ok()) {
                      return 0;
                  }
                  return swap.used;
              }),
              swap_free([]() -> int64_t {
                  Sigar ar;
                  SigarSwap swap;
                  auto rs = ar.get_swap(&swap);
                  if (!rs.ok()) {
                      return 0;
                  }
                  return swap.free;
              }),
              cpu_user([]() -> int64_t {
                  Sigar ar;
                  SigarCpu cpu;
                  auto rs = ar.get_cpu(&cpu);
                  if (!rs.ok()) {
                      return 0;
                  }
                  return cpu.user;
              }),
              cpu_sys([]() -> int64_t {
                  Sigar ar;
                  SigarCpu cpu;
                  auto rs = ar.get_cpu(&cpu);
                  if (!rs.ok()) {
                      return 0;
                  }
                  return cpu.sys;
              }),
              cpu_nice([]() -> int64_t {
                  Sigar ar;
                  SigarCpu cpu;
                  auto rs = ar.get_cpu(&cpu);
                  if (!rs.ok()) {
                      return 0;
                  }
                  return cpu.nice;
              }),
              cpu_idle([]() -> int64_t {
                  Sigar ar;
                  SigarCpu cpu;
                  auto rs = ar.get_cpu(&cpu);
                  if (!rs.ok()) {
                      return 0;
                  }
                  return cpu.idle;
              }),
              cpu_wait([]() -> int64_t {
                  Sigar ar;
                  SigarCpu cpu;
                  auto rs = ar.get_cpu(&cpu);
                  if (!rs.ok()) {
                      return 0;
                  }
                  return cpu.wait;
              }),
              cpu_irq([]() -> int64_t {
                  Sigar ar;
                  SigarCpu cpu;
                  auto rs = ar.get_cpu(&cpu);
                  if (!rs.ok()) {
                      return 0;
                  }
                  return cpu.irq;
              }),
              cpu_soft_irq([]() -> int64_t {
                  Sigar ar;
                  SigarCpu cpu;
                  auto rs = ar.get_cpu(&cpu);
                  if (!rs.ok()) {
                      return 0;
                  }
                  return cpu.soft_irq;
              }),
              cpu_stolen([]() -> int64_t {
                  Sigar ar;
                  SigarCpu cpu;
                  auto rs = ar.get_cpu(&cpu);
                  if (!rs.ok()) {
                      return 0;
                  }
                  return cpu.stolen;
              }),
              cpu_total([]() -> int64_t {
                  Sigar ar;
                  SigarCpu cpu;
                  auto rs = ar.get_cpu(&cpu);
                  if (!rs.ok()) {
                      return 0;
                  }
                  return cpu.total;
              }),
              uptime([]() -> double {
                  Sigar ar;
                  auto rs = ar.get_uptime();
                  if (!rs.ok()) {
                      return 0;
                  }
                  return rs.value_or_die();
              }),
              loadavg_1m([]() -> double {
                  Sigar ar;
                  auto rs = ar.get_loadavg();
                  if (!rs.ok()) {
                      return 0;
                  }
                  return rs.value_or_die().loadavg[0];
              }),
              loadavg_5m([]() -> double {
                  Sigar ar;
                  auto rs = ar.get_loadavg();
                  if (!rs.ok()) {
                      return 0;
                  }
                  return rs.value_or_die().loadavg[1];
              }),
              loadavg_15m([]() -> double {
                  Sigar ar;
                  auto rs = ar.get_loadavg();
                  if (!rs.ok()) {
                      return 0;
                  }
                  return rs.value_or_die().loadavg[2];
              }),
              disk_io_read([]() -> uint64_t {
                  Sigar ar;
                  SigarProcDiskIO dio;
                  auto rs = ar.get_proc_disk_io(&dio);
                  if (!rs.ok()) {
                      return 0;
                  }
                  return dio.bytes_read;
              }),
              disk_io_write([]() -> uint64_t {
                  Sigar ar;
                  SigarProcDiskIO dio;
                  auto rs = ar.get_proc_disk_io(&dio);
                  if (!rs.ok()) {
                      return 0;
                  }
                  return dio.bytes_written;
              }),
              disk_io_total([]() -> uint64_t {
                  Sigar ar;
                  SigarProcDiskIO dio;
                  auto rs = ar.get_proc_disk_io(&dio);
                  if (!rs.ok()) {
                      return 0;
                  }
                  return dio.bytes_total;
              }){

    }

    std::mutex expose_mutex;

    void SigarMetric::expose(Scope *scope) {
        auto e = is_exposed.load(std::memory_order_acquire);
        if (e) {
            return;
        }
        std::unique_lock lk(expose_mutex);
        if (is_exposed) {
            return;
        }
        is_exposed = true;
        if (scope == nullptr) {
            scope = ScopeInstance::instance()->get_sys_scope().get();
        }
        auto bool_log = turbo::get_flag(FLAGS_tally_log_sigar_metric_expose);
        auto rs = mem_ram.expose("memory_ram", "system memory ram", scope);
        if (!rs.ok()) {
            KLOG_IF(WARNING, bool_log) << "memory_ram expose fail reason: " << rs.to_string();
        } else {
            KLOG_IF(INFO, bool_log) << "memory_ram expose success";
        }
        rs = mem_total.expose("memory_total", "system memory total", scope);
        if (!rs.ok()) {
            KLOG_IF(WARNING, bool_log) << "memory_total expose fail reason: " << rs.to_string();
        } else {
            KLOG_IF(INFO, bool_log) << "memory_total expose success";
        }

        rs = mem_used.expose("mem_used", "system memory used", scope);
        if (!rs.ok()) {
            KLOG_IF(WARNING, bool_log) << "mem_used expose fail reason: " << rs.to_string();
        } else {
            KLOG_IF(INFO, bool_log) << "mem_used expose success";
        }

        rs = mem_free.expose("mem_free", "system memory free", scope);
        if (!rs.ok()) {
            KLOG_IF(WARNING, bool_log) << "mem_free expose fail reason: " << rs.to_string();
        } else {
            KLOG_IF(INFO, bool_log) << "mem_free expose success";
        }

        rs = mem_actual_used.expose("mem_actual_used", "system memory actual used", scope);
        if (!rs.ok()) {
            KLOG_IF(WARNING, bool_log) << "mem_actual_used expose fail reason: " << rs.to_string();
        } else {
            KLOG_IF(INFO, bool_log) << "mem_actual_used expose success";
        }

        rs = mem_actual_free.expose("mem_actual_free", "system memory actual free", scope);
        if (!rs.ok()) {
            KLOG_IF(WARNING, bool_log) << "mem_actual_free expose fail reason: " << rs.to_string();
        } else {
            KLOG_IF(INFO, bool_log) << "mem_actual_free expose success";
        }

        rs = swap_total.expose("swap_total", "system swap actual total", scope);
        if (!rs.ok()) {
            KLOG_IF(WARNING, bool_log) << "swap_total expose fail reason: " << rs.to_string();
        } else {
            KLOG_IF(INFO, bool_log) << "swap_total expose success";
        }

        rs = swap_used.expose("swap_used", "system swap actual used", scope);
        if (!rs.ok()) {
            KLOG_IF(WARNING, bool_log) << "swap_used expose fail reason: " << rs.to_string();
        } else {
            KLOG_IF(INFO, bool_log) << "swap_used expose success";
        }

        rs = swap_free.expose("swap_free", "system swap actual free", scope);
        if (!rs.ok()) {
            KLOG_IF(WARNING, bool_log) << "swap_free expose fail reason: " << rs.to_string();
        } else {
            KLOG_IF(INFO, bool_log) << "swap_free expose success";
        }

        rs = cpu_user.expose("cpu_user", "system cpu actual user", scope);
        if (!rs.ok()) {
            KLOG_IF(WARNING, bool_log) << "cpu_user expose fail reason: " << rs.to_string();
        } else {
            KLOG_IF(INFO, bool_log) << "cpu_user expose success";
        }

        rs = cpu_sys.expose("cpu_sys", "system cpu actual sys", scope);
        if (!rs.ok()) {
            KLOG_IF(WARNING, bool_log) << "cpu_sys expose fail reason: " << rs.to_string();
        } else {
            KLOG_IF(INFO, bool_log) << "cpu_sys expose success";
        }

        rs = cpu_nice.expose("cpu_nice", "system cpu actual nice", scope);
        if (!rs.ok()) {
            KLOG_IF(WARNING, bool_log) << "cpu_nice expose fail reason: " << rs.to_string();
        } else {
            KLOG_IF(INFO, bool_log) << "cpu_nice expose success";
        }

        rs = cpu_idle.expose("cpu_idle", "system cpu actual idle", scope);
        if (!rs.ok()) {
            KLOG_IF(WARNING, bool_log) << "cpu_idle expose fail reason: " << rs.to_string();
        } else {
            KLOG_IF(INFO, bool_log) << "cpu_idle expose success";
        }

        rs = cpu_wait.expose("cpu_wait", "system cpu actual wait", scope);
        if (!rs.ok()) {
            KLOG_IF(WARNING, bool_log) << "cpu_wait expose fail reason: " << rs.to_string();
        } else {
            KLOG_IF(INFO, bool_log) << "cpu_wait expose success";
        }

        rs = cpu_irq.expose("cpu_irq", "system cpu actual irq", scope);
        if (!rs.ok()) {
            KLOG_IF(WARNING, bool_log) << "cpu_irq expose fail reason: " << rs.to_string();
        } else {
            KLOG_IF(INFO, bool_log) << "cpu_irq expose success";
        }

        rs = cpu_soft_irq.expose("cpu_soft_irq", "system cpu soft irq", scope);
        if (!rs.ok()) {
            KLOG_IF(WARNING, bool_log) << "cpu_soft_irq expose fail reason: " << rs.to_string();
        } else {
            KLOG_IF(INFO, bool_log) << "cpu_soft_irq expose success";
        }

        rs = cpu_stolen.expose("cpu_stolen", "system cpu stolen", scope);
        if (!rs.ok()) {
            KLOG_IF(WARNING, bool_log) << "cpu_stolen expose fail reason: " << rs.to_string();
        } else {
            KLOG_IF(INFO, bool_log) << "cpu_stolen expose success";
        }

        rs = cpu_total.expose("cpu_total", "system cpu total", scope);
        if (!rs.ok()) {
            KLOG_IF(WARNING, bool_log) << "cpu_total expose fail reason: " << rs.to_string();
        } else {
            KLOG_IF(INFO, bool_log) << "cpu_total expose success";
        }

        rs = uptime.expose("uptime", "system uptime", scope);
        if (!rs.ok()) {
            KLOG_IF(WARNING, bool_log) << "uptime expose fail reason: " << rs.to_string();
        } else {
            KLOG_IF(INFO, bool_log) << "uptime expose success";
        }

        rs = loadavg_1m.expose("loadavg_1m", "system loadavg 1 min", scope);
        if (!rs.ok()) {
            KLOG_IF(WARNING, bool_log) << "loadavg_1m expose fail reason: " << rs.to_string();
        } else {
            KLOG_IF(INFO, bool_log) << "loadavg_1m expose success";
        }

        rs = loadavg_5m.expose("loadavg_5m", "system loadavg 5 min", scope);
        if (!rs.ok()) {
            KLOG_IF(WARNING, bool_log) << "loadavg_5m expose fail reason: " << rs.to_string();
        } else {
            KLOG_IF(INFO, bool_log) << "loadavg_5m expose success";
        }

        rs = loadavg_15m.expose("loadavg_15m", "system loadavg 5 min", scope);
        if (!rs.ok()) {
            KLOG_IF(WARNING, bool_log) << "loadavg_15m expose fail reason: " << rs.to_string();
        } else {
            KLOG_IF(INFO, bool_log) << "loadavg_15m expose success";
        }

        rs = disk_io_read.expose("disk_io_read", "system disk io read", scope);
        if (!rs.ok()) {
            KLOG_IF(WARNING, bool_log) << "disk_io_read expose fail reason: " << rs.to_string();
        } else {
            KLOG_IF(INFO, bool_log) << "disk_io_read expose success";
        }

        rs = disk_io_write.expose("disk_io_write", "system disk io write", scope);
        if (!rs.ok()) {
            KLOG_IF(WARNING, bool_log) << "disk_io_write expose fail reason: " << rs.to_string();
        } else {
            KLOG_IF(INFO, bool_log) << "disk_io_write expose success";
        }

        rs = disk_io_total.expose("disk_io_total", "system disk io total", scope);
        if (!rs.ok()) {
            KLOG_IF(WARNING, bool_log) << "disk_io_total expose fail reason: " << rs.to_string();
        } else {
            KLOG_IF(INFO, bool_log) << "disk_io_total expose success";
        }

    }

    void SigarMetric::hide() {
        mem_ram.hide();
        mem_total.hide();
        mem_used.hide();
        mem_free.hide();
        mem_actual_used.hide();
        mem_actual_free.hide();

        // swap
        swap_total.hide();
        swap_used.hide();
        swap_free.hide();

        // cpu
        cpu_user.hide();
        cpu_sys.hide();
        cpu_nice.hide();
        cpu_idle.hide();
        cpu_wait.hide();
        cpu_irq.hide();
        cpu_soft_irq.hide();
        cpu_stolen.hide();
        cpu_total.hide();
        uptime.hide();

        loadavg_1m.hide();
        loadavg_5m.hide();
        loadavg_15m.hide();

        disk_io_read.hide();
        disk_io_write.hide();
        disk_io_total.hide();;

    }
}  // namespace tally
