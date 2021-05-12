
#ifndef R_MORTEM_H
#define R_MORTEM_H

void mortem_init();
void mortem_backtrace();
void mortem_signal_handler(int signum);

#endif /* R_MORTEM_H */
