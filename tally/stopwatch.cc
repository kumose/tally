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

#include <tally/stopwatch.h>

namespace tally {

    Stopwatch::Stopwatch(turbo::Time start, turbo::Nonnull<StopwatchRecorder *>recorder)
            : start_(start), recorder_(recorder) {}

    Stopwatch::~Stopwatch() {
        if(!stop_) {
            stop();
        }
    }

    void Stopwatch::stop() {
        if (recorder_ != nullptr) {
            recorder_->record_stopwatch(start_);
        }
        stop_ = true;
    }

}  // namespace tally
