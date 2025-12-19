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
#ifdef WIN32
/* performance data helpers */

#define PdhFirstObject(block) \
    ((PERF_OBJECT_TYPE *)((BYTE *) block + block->HeaderLength))

#define PdhNextObject(object) \
    ((PERF_OBJECT_TYPE *)((BYTE *) object + object->TotalByteLength))

#define PdhFirstCounter(object) \
    ((PERF_COUNTER_DEFINITION *)((BYTE *) object + object->HeaderLength))

#define PdhNextCounter(counter) \
    ((PERF_COUNTER_DEFINITION *)((BYTE *) counter + counter->ByteLength))

#define PdhGetCounterBlock(inst) \
    ((PERF_COUNTER_BLOCK *)((BYTE *) inst + inst->ByteLength))

#define PdhFirstInstance(object) \
    ((PERF_INSTANCE_DEFINITION *)((BYTE *) object + object->DefinitionLength))

#define PdhNextInstance(inst) \
    ((PERF_INSTANCE_DEFINITION *)((BYTE *)inst + inst->ByteLength + \
                                  PdhGetCounterBlock(inst)->ByteLength))

#define PdhInstanceName(inst) \
    ((wchar_t *)((BYTE *)inst + inst->NameOffset))

#endif
