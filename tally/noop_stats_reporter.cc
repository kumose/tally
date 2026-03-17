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
