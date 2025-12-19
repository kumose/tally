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

#include <chrono>

#include <gtest/gtest.h>

#include "mock_stats_reporter.h"
#include <tally/tally.h>

TEST(HistogramImplTest, RecordValueOnce) {
    std::string name("foo");
    std::string help("help");
    turbo::flat_hash_map<std::string, std::string> tags({});
    auto buckets = tally::Buckets::linear_values(0.0, 1.0, 10);
    double value = 1.5;
    std::shared_ptr<MockStatsReporter> reporter(new MockStatsReporter());

    turbo::Time now = turbo::Time::current_time();
    tally::Histogram histogram(buckets);
    EXPECT_CALL(*reporter.get(), report_variable(&histogram, now)).Times(1);


    histogram.record(value);
    reporter->report_variable(&histogram, now);
}
/*
TEST(HistogramImplTest, RecordDurationOnce) {
    std::string name("foo");
    std::string help("help");
    turbo::flat_hash_map<std::string, std::string> tags({});
    auto buckets = tally::Buckets::exponential_durations(
            std::chrono::nanoseconds(1000), 2, 10);
    std::chrono::nanoseconds duration(1500);
    std::shared_ptr<MockStatsReporter> reporter(new MockStatsReporter());

    EXPECT_CALL(*reporter.get(),
                ReportHistogramDurationSamples(
                        name, tags, 1, 10, std::chrono::nanoseconds(1000),
                        std::chrono::nanoseconds(2000), 1));

    tally::Histogram histogram(buckets);
    histogram->Record(duration);
    histogram->Report(name, tags, reporter.get());
}

TEST(HistogramImplTest, RecordValueMultipleTimes) {
    std::string name("foo");
    turbo::flat_hash_map<std::string, std::string> tags({});
    auto buckets = tally::Buckets::linear_values(0.0, 1.0, 10);
    double value = 1.5;
    std::shared_ptr<MockStatsReporter> reporter(new MockStatsReporter());

    EXPECT_CALL(*reporter.get(),
                ReportHistogramValueSamples(name, tags, 2, 10, 1.0, 2.0, 2));

    auto histogram = tally::HistogramImpl::New(buckets);
    histogram->Record(value);
    histogram->Record(value);
    histogram->Report(name, tags, reporter.get());
}

TEST(HistogramImplTest, RecordDurationMultipleTimes) {
    std::string name("foo");
    turbo::flat_hash_map<std::string, std::string> tags({});
    auto buckets = tally::Buckets::exponential_durations(
            std::chrono::nanoseconds(1000), 2, 10);
    std::chrono::nanoseconds duration(3000);
    std::shared_ptr<MockStatsReporter> reporter(new MockStatsReporter());

    EXPECT_CALL(*reporter.get(),
                ReportHistogramDurationSamples(
                        name, tags, 2, 10, std::chrono::nanoseconds(2000),
                        std::chrono::nanoseconds(4000), 2));

    auto histogram = tally::HistogramImpl::New(buckets);
    histogram->Record(duration);
    histogram->Record(duration);
    histogram->Report(name, tags, reporter.get());
}

TEST(HistogramImplTest, RecordValueMultipleTimesWithMultipleBuckets) {
    std::string name("foo");
    turbo::flat_hash_map<std::string, std::string> tags({});
    auto buckets = tally::Buckets::linear_values(0.0, 1.0, 10);
    double first_value = 0.5;
    double second_value = 2.5;
    std::shared_ptr<MockStatsReporter> reporter(new MockStatsReporter());

    EXPECT_CALL(*reporter.get(),
                ReportHistogramValueSamples(name, tags, 1, 10, 0.0, 1.0, 1));
    EXPECT_CALL(*reporter.get(),
                ReportHistogramValueSamples(name, tags, 3, 10, 2.0, 3.0, 2));

    auto histogram = tally::HistogramImpl::New(buckets);
    histogram->Record(first_value);
    histogram->Record(second_value);
    histogram->Record(second_value);
    histogram->Report(name, tags, reporter.get());
}

TEST(HistogramImplTest, RecordDurationMultipleTimesWithMultipleBuckets) {
    std::string name("foo");
    turbo::flat_hash_map<std::string, std::string> tags({});
    auto buckets = tally::Buckets::exponential_durations(
            std::chrono::nanoseconds(1000), 2, 10);
    std::chrono::nanoseconds first_duration(3000);
    std::chrono::nanoseconds second_duration(1500);
    std::shared_ptr<MockStatsReporter> reporter(new MockStatsReporter());

    EXPECT_CALL(*reporter.get(),
                ReportHistogramDurationSamples(
                        name, tags, 1, 10, std::chrono::nanoseconds(1000),
                        std::chrono::nanoseconds(2000), 2));
    EXPECT_CALL(*reporter.get(),
                ReportHistogramDurationSamples(
                        name, tags, 2, 10, std::chrono::nanoseconds(2000),
                        std::chrono::nanoseconds(4000), 1));

    auto histogram = tally::HistogramImpl::New(buckets);
    histogram->Record(first_duration);
    histogram->Record(second_duration);
    histogram->Record(second_duration);
    histogram->Report(name, tags, reporter.get());
}

TEST(HistogramImplTest, Stopwatch) {
    std::string name("foo");
    turbo::flat_hash_map<std::string, std::string> tags({});
    auto buckets = tally::Buckets::linear_durations(
            std::chrono::nanoseconds(0), std::chrono::nanoseconds(1000000), 10);
    std::shared_ptr<MockStatsReporter> reporter(new MockStatsReporter());

    EXPECT_CALL(*reporter.get(),
                ReportHistogramDurationSamples(
                        name, tags, 1, 10, std::chrono::nanoseconds(0),
                        std::chrono::nanoseconds(1000000), 1));

    auto histogram = tally::HistogramImpl::New(buckets);
    auto stopwatch = histogram->Start();
    stopwatch.Stop();
    histogram->Report(name, tags, reporter.get());
}

TEST(HistogramImplTest, record_stopwatch) {
    std::string name("foo");
    turbo::flat_hash_map<std::string, std::string> tags({});
    auto buckets = tally::Buckets::linear_durations(
            std::chrono::nanoseconds(0), std::chrono::nanoseconds(1000000), 10);
    std::shared_ptr<MockStatsReporter> reporter(new MockStatsReporter());

    EXPECT_CALL(*reporter.get(),
                ReportHistogramDurationSamples(
                        name, tags, 1, 10, std::chrono::nanoseconds(0),
                        std::chrono::nanoseconds(1000000), 1));

    auto histogram = tally::HistogramImpl::New(buckets);
    histogram->record_stopwatch(std::chrono::steady_clock::now());
    histogram->Report(name, tags, reporter.get());
}
*/