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

#include <tally/sigar.h>
#include <tally/gauge.h>

namespace tally {

    class Scope;

    class SigarMetric {
    public:
        static SigarMetric *instance() {
            static SigarMetric ins;
            return &ins;
        }

        void expose(Scope *scope = nullptr);

        void hide();

    private:
        SigarMetric();

    public:
        static std::atomic<bool> is_exposed;
        Sigar _ar;
        // memory
        // static
        FuncGauge<int64_t> mem_ram;
        FuncGauge<int64_t> mem_total;
        // dynamic
        FuncGauge<int64_t> mem_used;
        FuncGauge<int64_t> mem_free;
        FuncGauge<int64_t> mem_actual_used;
        FuncGauge<int64_t> mem_actual_free;

        // swap
        FuncGauge<int64_t> swap_total;
        FuncGauge<int64_t> swap_used;
        FuncGauge<int64_t> swap_free;

        // sys cpu
        FuncGauge<int64_t> cpu_user;
        FuncGauge<int64_t> cpu_sys;
        FuncGauge<int64_t> cpu_nice;
        FuncGauge<int64_t> cpu_idle;
        FuncGauge<int64_t> cpu_wait;
        FuncGauge<int64_t> cpu_irq;
        FuncGauge<int64_t> cpu_soft_irq;
        FuncGauge<int64_t> cpu_stolen;
        FuncGauge<int64_t> cpu_total;

        FuncGauge<double> uptime;

        FuncGauge<double> loadavg_1m;
        FuncGauge<double> loadavg_5m;
        FuncGauge<double> loadavg_15m;

        FuncGauge<double> disk_io_read;
        FuncGauge<double> disk_io_write;
        FuncGauge<double> disk_io_total;

    };

    inline void init_sigar_metric() {
        SigarMetric::instance()->expose();
    }

    /// no need just for test using.
    inline void finish_sigar_metric() {
        SigarMetric::instance()->hide();
    }

    struct MetricInitialize {
        MetricInitialize() {
            KLOG(INFO) << "MetricInitialize expose";
            SigarMetric::instance()->expose();
        }

        ~MetricInitialize() {
            SigarMetric::instance()->hide();
        }
    };

}  // namespace tally
