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
