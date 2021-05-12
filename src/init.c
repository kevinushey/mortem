
#include <R.h>
#include <Rinternals.h>
#include <stdlib.h> // for NULL
#include <R_ext/Rdynload.h>

/* .Call calls */
extern SEXP r_mortem_init();

static const R_CallMethodDef CallEntries[] = {
    {"r_mortem_init", (DL_FUNC) &r_mortem_init, 0},
    {NULL, NULL, 0}
};

void R_init_mortem(DllInfo *dll) {
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}
