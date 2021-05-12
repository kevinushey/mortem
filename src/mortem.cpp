
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


#include <initializer_list>
#include <map>

extern "C" void Rprintf(const char* fmt, ...);

// array of signal actions
std::map<int, struct sigaction> s_actions;

#endif

namespace mortem {

void init() {

#ifdef MORTEM_ENABLED

  for (int signum : { SIGILL, SIGABRT, SIGSEGV }) {

    // save old action
    struct sigaction action;
    sigaction(signum, NULL, &action);
    s_actions[signum] = action;

    // install our new action
    signal(signum, aborted);

  }

#endif

}

void backtrace_print() {

#ifdef MORTEM_ENABLED

// get read to fork
int parent = getpid();
int child = fork();

if (child == 0) {

  // create debugger command
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

} else {

  // wait for child
  waitpid(child, NULL, 0);

}

#endif

}

void aborted(int signum) {

#ifdef MORTEM_ENABLED

  Rprintf("[!] Caught deadly signal %i [%s]\n", signum, strsignal(signum));

  // print our stack trace
  backtrace_print();

  // call original signal action if available
  if (s_actions.count(signum))
  {
    // read handler
    auto action = s_actions[signum];
    auto handler = action.sa_handler;

    // invoke handler, if any
    if (handler != NULL)
      handler(signum);
  }

#endif

}

} // end namespace mortem

