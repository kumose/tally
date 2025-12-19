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
