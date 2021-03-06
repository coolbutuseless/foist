
#' Foist provides fast output of numeric matrices and arrays to NETPBM PGM/PPM format and PNG format.
#'
#' Foist provides fast output of numeric matrices and arrays to NETPBM PGM/PPM format and PNG format.
#'
#' This package provides functions for fast output of numeric arrays and
#' matrices to lossless image formats - PNG and NETPBM.
#'
#' NETPBM is an uncompressed format by default, and PNG can be manipulated into
#' containing uncompressed data.  By avoiding this compression step, these formats
#' can then write the array/matrix data directly to the image file making them
#' faster than more traditional output methods (e.g. the \code{png} package, or
#' the \code{raster} package).
#'
#' Further speedup is gained by having the option to write the data in the
#' same order as it is stored within R (column major ordering).  Since image
#' formats are always stored in \code{row major} order, image writing usually needs to
#' perform an expensive data transposition step (from R's column-major ordering).  By
#' avoiding this data transposition, images can be written with almost no
#' intermediate data manipulation - making it super quick!
#'
#' Also, because the data is written directly from memory into a small buffer
#' for output additional otuput features are almost free in terms of time:
#'
#' \itemize{
#' \item{Flipping an image in the y direction}
#' \item{Scaling all the image data by a constant value}
#' }
#'
#' @keywords internal
"_PACKAGE"

# The following block is used by usethis to automatically manage
# roxygen namespace tags. Modify with care!
## usethis namespace: start
## usethis namespace: end
NULL
