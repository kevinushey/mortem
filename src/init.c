
#include <R.h>
#include <Rinternals.h>
#include <stdlib.h> // for NULL
#include <R_ext/Rdynload.h>

/* .Call calls */
extern SEXP r_mortem_init();
extern SEXP r_mortem_backtrace();
extern SEXP r_mortem_enabled();

static const R_CallMethodDef CallEntries[] = {
    {"r_mortem_init",      (DL_FUNC) &r_mortem_init,      0},
    {"r_mortem_backtrace", (DL_FUNC) &r_mortem_backtrace, 0},
    {"r_mortem_enabled",   (DL_FUNC) &r_mortem_enabled,   0},
    {NULL, NULL, 0}
};

void R_init_mortem(DllInfo *dll) {
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}
