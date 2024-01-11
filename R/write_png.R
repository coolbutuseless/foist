
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Write a numeric matrix or array to a PNG file
#'
#' Write a numeric matrix or array to a PNG file
#'
#'
#' Write a numeric matrix or array to a PNG file as fast as possible - meaning
#' that corners are cut to make it happen quickly:
#'
#' \itemize{
#' \item{Data is not compressed.}
#' \item{Matrix or array must be of type \code{numeric}}
#' }
#'
#' @param data Numeric 2d matrix or 3d array (with 3 planes)
#' @param filename Output filename e.g. "example.ppm"
#' @param convert_to_row_major Convert to row-major order before output. R stores matrix
#'        and array data in column-major order. In order to output row-major order (as
#'        expected by normal PNG output) data ordering must be converted. If this argument
#'        is set to FALSE, then image output will be faster (due to fewer data-ordering operations, and
#'        better cache coherency) but the image will be transposed. Default: TRUE
#' @param flipy By default, the position [0, 0] is considered the top-left corner of the output image.
#'        Set flipy = TRUE for [0, 0] to represent the bottom-left corner.  This operation
#'        is very fast and has negligible impact on overall write speed.
#'        Default: flipy = FALSE.
#' @param invert Invert all the pixel brightness values - as if the image were
#'        converted into a negative. Dark areas become bright and bright areas become dark.
#'        Default: FALSE
#' @param intensity_factor Multiplication factor applied to all values in image
#'        (note: no checking is performed to ensure values remain in range [0, 1]).
#'        If intensity_factor <= 0, then automatically determine (and apply) a multiplication factor
#'        to set the maximum value to 1.0. Default: intensity_factor = 1.0
#' @param pal Integer matrix of size 256x3 with values in the range [0, 255]. Each
#'        row represents the r, g, b colour for a given grey index value. Only used
#'        if \code{data} is a matrix
#' @param alpha If \code{data} is a 3d array, \code{alpha} should be an integer vector of length 3 giving the RGB colour to make fully transparent. If \code{data} is a matrix and \code{pal} has been provided, \code{alpha} should be a positive integer and the first \code{alpha} colours in the palette will be fully transparent
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
write_png <- function(data, filename,
                      convert_to_row_major = TRUE,
                      flipy                = FALSE,
                      invert               = FALSE,
                      intensity_factor     = 1,
                      pal                  = NULL,
                      alpha) {
    with_alpha <- FALSE
    alpha_rgb <- c(0, 0, 0)
    n_alpha <- 0
    if (!missing(alpha) && length(alpha) > 0) {
        if (length(dim(data)) == 3 && length(alpha) == 3) {
            with_alpha <- TRUE
            alpha_rgb <- alpha
        }
        if (!missing(pal) && length(dim(data)) == 2 && isTRUE(alpha > 0)) {
            with_alpha <- TRUE
            n_alpha <- alpha
        }
    }
    invisible(.Call(`_foist_write_png_core`, data, dim(data), filename,
                    convert_to_row_major, flipy, invert, intensity_factor, pal, with_alpha, alpha_rgb, n_alpha))
}



