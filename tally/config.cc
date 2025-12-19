//
// Copyright (C) 2024 Kumo group inc.
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

#include <tally/config.h>
#include <turbo/strings/match.h>

TURBO_FLAG(int32_t, tally_sampler_thread_start_delay_us, 10000, "tally sampler thread start delay us");
TURBO_FLAG(bool, tally_enable_sampling, true, "is enable tally sampling");

TURBO_FLAG(bool, tally_crash_on_expose_fail, true, "tally crash on expose fail");

TURBO_FLAG(bool, tally_quote_vector, true,
           "Quote description of Vector<> to make it valid to noah");

TURBO_FLAG(bool, tally_save_series, true,
           "Save values of last 60 seconds, last 60 minutes,"
           " last 24 hours and last 30 days for plotting");

TURBO_FLAG(int32_t, tally_dump_interval,
           10, "Seconds between consecutive dump");

TURBO_FLAG(int32_t, tally_default_report_interval_ms,
           5000, "milliseconds between report reporter");
TURBO_FLAG(int32_t, tally_min_report_interval_ms,
           10, "milliseconds between report reporter");

TURBO_FLAG(std::string, prometheus_scope_name, "kumo_prometheus", "kumo prometheus prefix");

TURBO_FLAG(int32_t, prometheus_collect_interval_s, 5, "seconds between prometheus report reporter");

TURBO_FLAG(std::string, tally_flag_scope_name, "flag", "default flag scope prefix");

TURBO_FLAG(std::string, tally_scope_separator, "_", "default flag scope prefix");
TURBO_FLAG(std::string, tally_flag_scope_tags, "", "default system metric scope tags eg. tag1:v1;tag2:v2");

TURBO_FLAG(std::string, tally_sys_scope_name, "sys", "default system metric scope prefix");
TURBO_FLAG(std::string, tally_sys_scope_tags, "", "default flag scope tags eg. tag1:v1;tag2:v2");


TURBO_FLAG(std::string, tally_root_scope_name, "km", "default system metric scope prefix");
TURBO_FLAG(std::string, tally_root_scope_tags, "", "default flag scope tags eg. tag1:v1;tag2:v2");

TURBO_FLAG(uint64_t, tally_latency_scale_factor, 1,
           "latency scale factor, used by method status, etc., latency_us = latency * latency_scale_factor");


TURBO_FLAG(int32_t, tally_latency_p2, 90, "Second latency percentile").on_validate(
    [](std::string_view value, std::string *err) noexcept -> bool {
        int32_t val;
        if (!turbo::parse_flag(value, &val, err)) {
            return false;
        }
        if (val <= 0 || val >= 100) {
            if (err)
                *err = "Percentile must be in (0, 100)";
            return false;
        }
        if (val >= turbo::get_flag(FLAGS_tally_latency_p3)) {
            if (err)
                *err = "Percentile must be less than var_latency_p3";
            return false;
        }
        if (val <= turbo::get_flag(FLAGS_tally_latency_p1)) {
            if (err)
                *err = "Percentile must be less than var_latency_p3";
            return false;
        }
        return true;
    });
TURBO_FLAG(int32_t, tally_latency_p1, 80, "First latency percentile").on_validate(
    [](std::string_view value, std::string *err) noexcept -> bool {
        int32_t val;
        if (!turbo::parse_flag(value, &val, err)) {
            return false;
        }
        if (val <= 0 || val >= 100) {
            if (err)
                *err = "Percentile must be in (0, 100)";
            return false;
        }
        if (val >= turbo::get_flag(FLAGS_tally_latency_p2)) {
            if (err)
                *err = "Percentile must be less than var_latency_p2";
            return false;
        }
        return true;
    });
TURBO_FLAG(int32_t, tally_latency_p3, 99, "Third latency percentile").on_validate(
    [](std::string_view value, std::string *err) noexcept -> bool {
        int32_t val;
        if (!turbo::parse_flag(value, &val, err)) {
            return false;
        }
        if (val <= 0 || val >= 100) {
            if (err)
                *err = "Percentile must be in (0, 100)";
            return false;
        }
        if (val <= turbo::get_flag(FLAGS_tally_latency_p2)) {
            if (err)
                *err = "Percentile must be larger than var_latency_p2";
            return false;
        }
        return true;
    });

// TODO: Do we need to expose this flag? Dumping thread may dump different
// kind of samples, users are unlikely to make good decisions on this value.
TURBO_FLAG(int32_t, tally_collector_max_pending_samples, 1000,
           "Destroy unprocessed samples when they're too many");

TURBO_FLAG(int32_t, tally_collector_expected_per_second, 1000,
           "Expected number of samples to be collected per second");

TURBO_FLAG(bool, tally_log_sigar_metric_expose, false, "tally log sigar metric expose");

TURBO_FLAG(std::string, tally_dump_file, "tally_var.jsonl", "tally log sigar metric expose");
TURBO_FLAG(bool, tally_dump_local, true, "tally local timezone or utc");
TURBO_FLAG(int32_t, tally_dump_interval_s, 10, "tally dump interval");
TURBO_FLAG(std::string, tally_dump_white, "", "tally dump white vars");
TURBO_FLAG(std::string, tally_dump_black, "", "tally dump black vars");

namespace tally {
    void setup_tally_flags(turbo::cli::App *app) {
        auto tally_group = app->add_option_group("tally", "tally metrics group");
        tally_group->enable_flags_option(FLAGS_tally_sampler_thread_start_delay_us);

        tally_group->enable_flags_option(FLAGS_tally_enable_sampling);

        tally_group->enable_flags_option(FLAGS_tally_quote_vector);

        tally_group->enable_flags_option(FLAGS_tally_crash_on_expose_fail);

        tally_group->enable_flags_option(FLAGS_tally_dump_interval);

        tally_group->enable_flags_option(FLAGS_tally_save_series);

        tally_group->enable_flags_option(FLAGS_tally_default_report_interval_ms);

        tally_group->enable_flags_option(FLAGS_tally_min_report_interval_ms);

        tally_group->enable_flags_option(FLAGS_prometheus_scope_name);
        tally_group->enable_flags_option(FLAGS_prometheus_collect_interval_s);

        tally_group->enable_flags_option(FLAGS_tally_root_scope_name);
        tally_group->enable_flags_option(FLAGS_tally_root_scope_tags);

        tally_group->enable_flags_option(FLAGS_tally_flag_scope_name);
        tally_group->enable_flags_option(FLAGS_tally_flag_scope_tags);

        tally_group->enable_flags_option(FLAGS_tally_sys_scope_name);
        tally_group->enable_flags_option(FLAGS_tally_sys_scope_tags);

        tally_group->enable_flags_option(FLAGS_tally_scope_separator);

        tally_group->enable_flags_option(FLAGS_tally_latency_scale_factor);

        tally_group->enable_flags_option(FLAGS_tally_latency_p1);
        tally_group->enable_flags_option(FLAGS_tally_latency_p2);
        tally_group->enable_flags_option(FLAGS_tally_latency_p3);

        tally_group->enable_flags_option(FLAGS_tally_collector_max_pending_samples);

        tally_group->enable_flags_option(FLAGS_tally_collector_expected_per_second);

        tally_group->enable_flags_option(FLAGS_tally_log_sigar_metric_expose);

        tally_group->enable_flags_option(FLAGS_tally_dump_file);
        tally_group->enable_flags_option(FLAGS_tally_dump_local);
        tally_group->enable_flags_option(FLAGS_tally_dump_interval_s);
        tally_group->enable_flags_option(FLAGS_tally_dump_white);
        tally_group->enable_flags_option(FLAGS_tally_dump_black);
    }
} // namespace tally
