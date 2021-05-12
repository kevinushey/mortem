
#include "mortem.h"

#ifndef MORTEM_ENABLED
# ifdef __GNUC__
#  define MORTEM_ENABLED
# endif
#endif

#ifdef MORTEM_ENABLED

#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <R.h>
#include <Rinternals.h>

void Rprintf(const char* fmt, ...);

#define lengthof(x) (sizeof(x) / sizeof(*x))

// array of signal actions
struct sigaction s_actions[32] = {{ 0 }};

#endif

SEXP r_mortem_init(SEXP signalsSEXP) {

#ifdef MORTEM_ENABLED

  // iterate over signals of interest
  int signals[] = { SIGILL, SIGABRT, SIGBUS, SIGSEGV };
  for (int i = 0; i < lengthof(signals); i++) {

    // get signal number
    int signum = signals[i];

    // save old action
    struct sigaction action;
    sigaction(signum, NULL, &action);
    s_actions[signum] = action;

    // install our new action
    signal(signum, mortem_signal_handler);

  }

#endif

  return R_NilValue;

}

void mortem_backtrace() {

#ifdef MORTEM_ENABLED

  // get ready to fork
  int parent = getpid();
  int child = fork();

  // parent: wait for child to finish
  if (child != 0) {
    waitpid(child, NULL, 0);
    return;
  }

  // child: run debugger
  char command[128];

#ifdef __APPLE__
  snprintf(command, 128, "lldb -b -p %i -o bt 2> /dev/null", parent);
#else
  snprintf(command, 128, "gdb -batch -p %i -ex bt 2> /dev/null", parent);
#endif

  // run it
  FILE* fp = popen(command, "r");
  if (fp == NULL)
    return;

  // pipe output back through R
  char buffer[256];
  while (fgets(buffer, 256, fp) != NULL)
    Rprintf("%s", buffer);

  // exit
  _exit(0);

#endif

}

void mortem_signal_handler(int signum) {

#ifdef MORTEM_ENABLED

  // print a short header
#ifdef __APPLE__
  const char* fmt = "*** caught deadly signal [%s] ***\n";
  Rprintf(fmt, strsignal(signum));
#else
  const char* fmt = "*** caught deadly signal %i [%s] ***\n";
  Rprintf(fmt, signum, strsignal(signum));
#endif

  // print our stack trace
  mortem_backtrace();

  // call original signal action if available
  struct sigaction action = s_actions[signum];
  if (action.sa_handler != NULL)
    action.sa_handler(signum);

#endif

}
