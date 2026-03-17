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

#include <turbo/flags/flag.h>
#include <turbo/flags/declare.h>
#include <turbo/flags/app.h>
#include <turbo/container/flat_hash_map.h>

TURBO_DECLARE_FLAG(int32_t, tally_sampler_thread_start_delay_us);

TURBO_DECLARE_FLAG(bool, tally_enable_sampling);

TURBO_DECLARE_FLAG(bool, tally_quote_vector);

TURBO_DECLARE_FLAG(bool, tally_crash_on_expose_fail);

TURBO_DECLARE_FLAG(int32_t, tally_dump_interval);

TURBO_DECLARE_FLAG(bool, tally_save_series);

TURBO_DECLARE_FLAG(int32_t, tally_default_report_interval_ms);

TURBO_DECLARE_FLAG(int32_t, tally_min_report_interval_ms);

TURBO_DECLARE_FLAG(std::string, prometheus_scope_name);
TURBO_DECLARE_FLAG(int32_t, prometheus_collect_interval_s);

TURBO_DECLARE_FLAG(std::string, tally_root_scope_name);
TURBO_DECLARE_FLAG(std::string, tally_root_scope_tags);

TURBO_DECLARE_FLAG(std::string, tally_flag_scope_name);
TURBO_DECLARE_FLAG(std::string, tally_flag_scope_tags);

TURBO_DECLARE_FLAG(std::string, tally_sys_scope_name);
TURBO_DECLARE_FLAG(std::string, tally_sys_scope_tags);

TURBO_DECLARE_FLAG(std::string, tally_scope_separator);

TURBO_DECLARE_FLAG(uint64_t, tally_latency_scale_factor);

TURBO_DECLARE_FLAG(int32_t, tally_latency_p1);
TURBO_DECLARE_FLAG(int32_t, tally_latency_p2);
TURBO_DECLARE_FLAG(int32_t, tally_latency_p3);

TURBO_DECLARE_FLAG(int32_t, tally_collector_max_pending_samples);

TURBO_DECLARE_FLAG(int32_t, tally_collector_expected_per_second);

TURBO_DECLARE_FLAG(bool, tally_log_sigar_metric_expose);

TURBO_DECLARE_FLAG(std::string, tally_dump_file);
TURBO_DECLARE_FLAG(bool, tally_dump_local);
TURBO_DECLARE_FLAG(int32_t, tally_dump_interval_s);
TURBO_DECLARE_FLAG(std::string, tally_dump_white);
TURBO_DECLARE_FLAG(std::string, tally_dump_black);

namespace tally {
    void setup_tally_flags(turbo::cli::App *app);
}  // namespace tally