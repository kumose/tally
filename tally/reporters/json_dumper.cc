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

#include <tally/reporters/json_dumper.h>
#include <turbo/memory/leaky_singleton.h>
#include <turbo/threading/platform_thread.h>
#include <tally/config.h>
#include <turbo/log/logging.h>
#include <tally/reporters/dump_json_stats_reporter.h>
#include <memory>
#include <thread>
#include <turbo/strings/str_format.h>
#include <turbo/times/time.h>
#include <turbo/files/file.h>

namespace tally {

    static std::string make_dump_filename(const turbo::Time &stamp) {
        turbo::CivilHour ch = turbo::Time::to_civil_hour(stamp, turbo::get_flag(FLAGS_tally_dump_local) ? turbo::TimeZone::local() : turbo::TimeZone::utc());
        auto d_file = turbo::get_flag(FLAGS_tally_dump_file);
        static std::string suffix = ".jsonl";
        if(turbo::ends_with(d_file, suffix)) {
            auto s = d_file.size() - suffix.size();
            d_file.resize(s);
        }
        return turbo::str_format("%s_%04d-%02d-%02d-%02d.jsonl", d_file.c_str(), ch.year(),ch.month(),ch.day(),ch.hour());
    }
    static void append_dump(const std::vector<std::string> &data, const turbo::Time &stamp) {
        if(data.empty()) {
            return;
        }
        auto fn = make_dump_filename(stamp);
        std::ofstream ofs(fn, std::ios::app | std::ios::binary);
        if(!ofs.is_open()) {
            KLOG(ERROR)<<"open to write: "<<fn<<" error: "<<errno;
        }
        for(auto &item : data) {
            ofs<<item<<"\n";
        }
        ofs.close();
    }

    const int WARN_NO_SLEEP_THRESHOLD = 2;

    JsonDumper::~JsonDumper() {
        KLOG_IF(FATAL, _created)<<"must stop this before exit";
    }
    turbo::Status JsonDumper::start() {
        if(_created) {
            return turbo::OkStatus();
        }
        const int rc = pthread_create(&_tid, nullptr, sampling_thread, this);
        if (rc != 0) {
            return turbo::unknown_error("Fail to create sampling_thread");
        } else {
            _created = true;
        }
        return turbo::OkStatus();
    }

    void JsonDumper::stop() {
        if (_created) {
            _stop = true;
            pthread_join(_tid, nullptr);
            _created = false;
        }
    }

    void* JsonDumper::sampling_thread(void* arg) {
        turbo::PlatformThread::SetName("json_dumper");
        static_cast<JsonDumper*>(arg)->run();
        return nullptr;
    }

    void JsonDumper::run() {
        ::usleep(turbo::get_flag(FLAGS_tally_sampler_thread_start_delay_us));

        int consecutive_nosleep = 0;
        while (!_stop) {
            int64_t abstime = turbo::Time::current_microseconds();
            std::unique_ptr<ReportOptions> options;
            if(!turbo::get_flag(FLAGS_tally_dump_white).empty() && !turbo::get_flag(FLAGS_tally_dump_black).empty()) {
                options = std::make_unique<ReportOptions>();
                options->build_filter(turbo::get_flag(FLAGS_tally_dump_white), turbo::get_flag(FLAGS_tally_dump_black));
            }
            DumpJsonStatsReporter reporter;
            if(options) {
                reporter.set_option(*options);
            }
            auto report_now = turbo::Time::current_time();
            Variable::report(&reporter,report_now);
            auto &data = reporter.data();
            append_dump(data,report_now);
            bool slept = false;
            int64_t now = turbo::Time::current_microseconds();
            _cumulated_time_us += now - abstime;
            abstime += 1000000L * turbo::get_flag(FLAGS_tally_dump_interval_s);
            while (abstime > now) {
                ::usleep(abstime - now);
                slept = true;
                now = turbo::Time::current_microseconds();
            }
            if (slept) {
                consecutive_nosleep = 0;
            } else {
                if (++consecutive_nosleep >= WARN_NO_SLEEP_THRESHOLD) {
                    consecutive_nosleep = 0;
                    KLOG(WARNING) << "tally is busy at sampling for "
                                 << WARN_NO_SLEEP_THRESHOLD << " seconds!";
                }
            }
        }
    }
}  // namespace tally
