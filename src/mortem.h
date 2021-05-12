
#ifndef R_MORTEM_H
#define R_MORTEM_H

namespace mortem {

void init();
void print_backtrace();
void aborted(int signum);

}

#endif /* R_MORTEM_H */
