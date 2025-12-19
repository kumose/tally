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