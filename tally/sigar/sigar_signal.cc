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

#include <tally/sigar/sigar.h>
#include <tally/sigar/sigar_private.h>
#include <tally/sigar/sigar_util.h>
#include <turbo/strings/match.h>

#ifdef WIN32
#include <windows.h>
#endif

#include <signal.h>
#include <errno.h>

SIGAR_DECLARE(int) sigar_proc_kill(sigar_pid_t pid, int signum)
{
#ifdef WIN32
    int status = -1;
    HANDLE proc =
        OpenProcess(PROCESS_ALL_ACCESS,
                    TRUE, (DWORD)pid);

    if (proc) {
        switch (signum) {
          case 0:
            status = SIGAR_OK;
            break;
          default:
            if (TerminateProcess(proc, signum)) {
                status = SIGAR_OK;
            }
            break;
        }

        CloseHandle(proc);

        if (status == SIGAR_OK) {
            return SIGAR_OK;
        }
    }
    return GetLastError();
#else
    if (kill(pid, signum) == -1) {
        return errno;
    }
    return SIGAR_OK;
#endif
}

turbo::Result<int> sigar_signum_get(std::string_view name)
{
    if (turbo::starts_with_ignore_case(name, "SIG")) {
        name.remove_prefix(3);
    }
    if(name.empty()) {
        return turbo::invalid_argument_error("name length error");
    }

    switch (*name.begin()) {
      case 'A':
#ifdef SIGABRT
        if (turbo::equals_ignore_case(name, "ABRT")) return SIGABRT;
#endif
#ifdef SIGALRM
        if (turbo::equals_ignore_case(name, "ALRM")) return SIGALRM;
#endif
        break;
      case 'B':
#ifdef SIGBUS
        if (turbo::equals_ignore_case(name, "BUS")) return SIGBUS;
#endif
        break;
      case 'C':
#ifdef SIGCONT
        if (turbo::equals_ignore_case(name, "CONT")) return SIGCONT;
#endif
#ifdef SIGCHLD
        if (turbo::equals_ignore_case(name, "CHLD")) return SIGCHLD;
#endif
#ifdef SIGCLD
        if (turbo::equals_ignore_case(name, "CLD")) return SIGCLD;
#endif
        break;
      case 'E':
#ifdef SIGEMT
        if (strEQ(name, "EMT")) return SIGEMT;
#endif
        break;
      case 'F':
#ifdef SIGFPE
        if (turbo::equals_ignore_case(name, "FPE")) return SIGFPE;
#endif
        break;
      case 'H':
#ifdef SIGHUP
        if (turbo::equals_ignore_case(name, "HUP")) return SIGHUP;
#endif
        break;
      case 'I':
#ifdef SIGINT
        if (turbo::equals_ignore_case(name, "INT")) return SIGINT;
#endif
#ifdef SIGILL
        if (turbo::equals_ignore_case(name, "ILL")) return SIGILL;
#endif
#ifdef SIGIOT
        if (turbo::equals_ignore_case(name, "IOT")) return SIGIOT;
#endif
#ifdef SIGIO
        if (turbo::equals_ignore_case(name, "IO")) return SIGIO;
#endif
#ifdef SIGINFO
        if (strEQ(name, "INFO")) return SIGINFO;
#endif
        break;
      case 'K':
#ifdef SIGKILL
        if (turbo::equals_ignore_case(name, "KILL")) return SIGKILL;
#endif
        break;
      case 'P':
#ifdef SIGPOLL
        if (turbo::equals_ignore_case(name, "POLL")) return SIGPOLL;
#endif
#ifdef SIGPIPE
        if (turbo::equals_ignore_case(name, "PIPE")) return SIGPIPE;
#endif
#ifdef SIGPROF
        if (turbo::equals_ignore_case(name, "PROF")) return SIGPROF;
#endif
#ifdef SIGPWR
        if (turbo::equals_ignore_case(name, "PWR")) return SIGPWR;
#endif
        break;
      case 'Q':
#ifdef SIGQUIT
        if (turbo::equals_ignore_case(name, "QUIT")) return SIGQUIT;
#endif
        break;
      case 'S':
#ifdef SIGSEGV
        if (turbo::equals_ignore_case(name, "SEGV")) return SIGSEGV;
#endif
#ifdef SIGSYS
        if (turbo::equals_ignore_case(name, "SYS")) return SIGSYS;
#endif
#ifdef SIGSTOP
        if (turbo::equals_ignore_case(name, "STOP")) return SIGSTOP;
#endif
#ifdef SIGSTKFLT
        if (turbo::equals_ignore_case(name, "STKFLT")) return SIGSTKFLT;
#endif
        break;
      case 'T':
#ifdef SIGTRAP
        if (turbo::equals_ignore_case(name, "TRAP")) return SIGTRAP;
#endif
#ifdef SIGTERM
        if (turbo::equals_ignore_case(name, "TERM")) return SIGTERM;
#endif
#ifdef SIGTSTP
        if (turbo::equals_ignore_case(name, "TSTP")) return SIGTSTP;
#endif
#ifdef SIGTTIN
        if (turbo::equals_ignore_case(name, "TTIN")) return SIGTTIN;
#endif
#ifdef SIGTTOU
        if (turbo::equals_ignore_case(name, "TTOU")) return SIGTTOU;
#endif
        break;
      case 'U':
#ifdef SIGURG
        if (turbo::equals_ignore_case(name, "URG")) return SIGURG;
#endif
#ifdef SIGUSR1
        if (turbo::equals_ignore_case(name, "USR1")) return SIGUSR1;
#endif
#ifdef SIGUSR2
        if (turbo::equals_ignore_case(name, "USR2")) return SIGUSR2;
#endif
        break;
      case 'V':
#ifdef SIGVTALRM
        if (turbo::equals_ignore_case(name, "VTALRM")) return SIGVTALRM;
#endif
        break;
      case 'W':
#ifdef SIGWINCH
        if (turbo::equals_ignore_case(name, "WINCH")) return SIGWINCH;
#endif
        break;
      case 'X':
#ifdef SIGXCPU
        if (turbo::equals_ignore_case(name, "XCPU")) return SIGXCPU;
#endif
#ifdef SIGXFSZ
        if (turbo::equals_ignore_case(name, "XFSZ")) return SIGXFSZ;
#endif
        break;
      default:
        break;
    }

    return turbo::not_found_error("");
}

