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

#include <turbo/utility/status.h>

namespace tally {
    class JsonDumper {
    public:
        ~JsonDumper();
        static JsonDumper *instance() {
            static JsonDumper ins;
            return &ins;
        }

        turbo::Status start();

        void stop();

        bool running() const {
            return _created && !_stop;
        }

    private:
        JsonDumper() = default;

        void run();

        static void* sampling_thread(void* arg);

    private:
        std::atomic<bool> _created{false};
        bool _stop{false};
        int64_t _cumulated_time_us{0};
        pthread_t _tid;
    };
}