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

#ifndef SIGAR_GETLINE_H
#define SIGAR_GETLINE_H

#include <tally/sigar/sigar.h>

typedef int (*sigar_getline_completer_t)(char *, int, int *);

SIGAR_DECLARE(char *) sigar_getline(char *prompt);
SIGAR_DECLARE(void) sigar_getline_setwidth(int width);
SIGAR_DECLARE(void) sigar_getline_redraw(void);
SIGAR_DECLARE(void) sigar_getline_reset(void);
SIGAR_DECLARE(void) sigar_getline_windowchanged();
SIGAR_DECLARE(void) sigar_getline_histinit(char *file);
SIGAR_DECLARE(void) sigar_getline_histadd(char *buf);
SIGAR_DECLARE(int)  sigar_getline_eof();
SIGAR_DECLARE(void) sigar_getline_completer_set(sigar_getline_completer_t func);

#endif /* SIGAR_GETLINE_H */
