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

#include <string>
#include <unordered_map>
#include <gmock/gmock.h>
#include <tally/stats_reporter.h>

class MockStatsReporter : public tally::StatsReporter {
    using Clock = std::chrono::steady_clock;

public:
    // Since Capabilities returns a unique_ptr, which is non-copyable, we need a
    // proxy method which can return a pointer to the necessary Capabilities
    // object.

    MOCK_METHOD0(flush, void());

    MOCK_METHOD2(report_variable,
                 void(const tally::Variable*value, const turbo::Time &stap));
};
