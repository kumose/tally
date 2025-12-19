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

#include <tally/noop_stats_reporter.h>

namespace tally {

    std::shared_ptr<StatsReporter> NoopStatsReporter::create() {
        return std::make_shared<NoopStatsReporter>();
    }

    void NoopStatsReporter::flush() {}

    void NoopStatsReporter::report_variable(
            const Variable *var, const turbo::Time &stamp) {

    }

}  // namespace tally
