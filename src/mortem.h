
#ifndef R_MORTEM_H
#define R_MORTEM_H

#define lengthof(x) (sizeof(x) / sizeof(*x))

void mortem_init();
void mortem_backtrace(int pid);
void mortem_signal_handler(int signum);

#endif /* R_MORTEM_H */
