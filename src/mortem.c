
#include "mortem.h"

#ifndef MORTEM_ENABLED
# ifdef __GNUC__
#  define MORTEM_ENABLED
# endif
#endif

#ifdef MORTEM_ENABLED

#include <dlfcn.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <R.h>
#include <Rinternals.h>

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

SEXP r_mortem_backtrace() {

#ifdef MORTEM_ENABLED

  mortem_backtrace(getpid());

#endif

  return R_NilValue;

}

void mortem_backtrace(int pid) {

#ifdef MORTEM_ENABLED

  // fork this process
  int child = fork();
  if (child != 0) {
    waitpid(child, NULL, 0);
    return;
  }

  // build debugger command
  char command[128];

#ifdef __APPLE__
  snprintf(command, 128, "lldb -b -p %i -o bt 2> /dev/null", pid);
#else
  snprintf(command, 128, "gdb -batch -p %i -ex bt 2> /dev/null", pid);
#endif

  // run it
  FILE* fp = popen(command, "r");
  if (fp == NULL)
    return;

  // write output to console
  char buffer[512];
  while (fgets(buffer, 512, fp) != NULL)
    write(STDOUT_FILENO, buffer, strlen(buffer));

  // exit
  void* handle = dlopen(NULL, RTLD_NOW);
  void (*_exit)(int) = (void (*)(int)) dlsym(handle, "_exit");
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

  // print stack trace
  mortem_backtrace(getpid());

  // call original signal action if available
  struct sigaction action = s_actions[signum];
  if (action.sa_handler != NULL)
    action.sa_handler(signum);

#endif

}
