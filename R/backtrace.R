
backtrace <- function() {
  result <- .Call(r_mortem_backtrace)
  invisible(result)
}
