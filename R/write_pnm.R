

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Write a numeric matrix or array to a NETPBM PNM file
#'
#' @param data numeric 2d matrix or 3d array (with 3 planes)
#' @param filename output filename e.g. "example.ppm"
#' @param convert_to_row_major Convert to row-major order before output. R stores matrix
#'        and array data in column-major order. In order to output row-major order (as
#'        expected by PGM/PPM image format) data ordering must be converted. If this argument
#'        is set to FALSE, then image output will be faster (due to fewer data-ordering operations, and
#'        better cache coherency) but the image will be transposed. Default: TRUE
#' @param flipy By default, the position [0, 0] is considered the top-left corner of the output image. Set flipy = TRUE
#'        for [0, 0] to represent the bottom-left corner. Default: flipy = FALSE
#' @param intensity_factor Multiplication factor applied to all values in image
#'        (note: no checking is performed to ensure values remain in range [0, 1]).
#'        If intensity_factor <= 0, then automatically determine (and apply) a multiplication factor
#'        to set the maximum value to 1.0. Default: intensity_factor = 1.0
#' @param pal integer matrix of size 256x3 with values in the range [0, 255]. Each
#'        row represents the r, g, b colour for a given grey index value. Only used
#'        if \code{data} is a matrix
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
write_pnm <- function(data, filename, convert_to_row_major = TRUE, flipy = FALSE, intensity_factor = 1, pal = NULL) {
    invisible(.Call(`_foist_write_pnm_core`, data, dim(data), filename, convert_to_row_major, flipy, intensity_factor, pal))
}
