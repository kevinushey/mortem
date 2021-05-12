
#include "mortem.h"

#ifndef MORTEM_ENABLED
# ifdef __GNUC__
#  define MORTEM_ENABLED
# endif
#endif

#ifdef MORTEM_ENABLED

#include <signal.h>
#include <string.h>

#include <execinfo.h>

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

  const int max_depth = 128;
  void* addrs[max_depth];

  int depth = ::backtrace(addrs, max_depth);
  char** symbols = ::backtrace_symbols(addrs, depth);

  for (int i = 0; i < depth; i++)
    Rprintf("%s\n", symbols[i]);

  free(symbols);

#endif

}

void aborted(int signum) {

#ifdef MORTEM_ENABLED

  // print our stack trace
  backtrace_print();

  // call original signal action if available
  if (s_actions.count(signum))
  {
    // read handler
    auto action = s_actions[signum];
    auto handler = action.sa_handler;

    // handle aborts specially if no handler registered
    if (handler == NULL && signum == SIGABRT)
      handler = s_actions[SIGSEGV].sa_handler;

    // invoke handler, if any
    if (handler != NULL)
      handler(signum);
  }

#endif

}

} // end namespace mortem

