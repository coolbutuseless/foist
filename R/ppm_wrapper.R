

#' Write a numeric array to a PPM file
#'
#' @param arr numeric array with 3 planes.
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
#'
write_ppm <- function(arr, filename, convert_to_row_major = TRUE, flipy = FALSE, intensity_factor = 1) {
    invisible(.Call(`_foist_write_ppm_core`, arr, dim(arr), filename, convert_to_row_major, flipy, intensity_factor))
}
