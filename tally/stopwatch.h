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

#include <memory>
#include <turbo/times/time.h>
#include "tally/stopwatch_recorder.h"

namespace tally {

    class Stopwatch {
    public:
        Stopwatch(turbo::Time,
                  turbo::Nonnull<StopwatchRecorder *>recorder);
        ~Stopwatch();

        // Stop stops the Stopwatch and records the duration of time it observed.
        void stop();

    private:
        const turbo::Time start_;
        StopwatchRecorder * recorder_;
        bool stop_{false};
    };

}  // namespace tally
