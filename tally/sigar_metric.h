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
