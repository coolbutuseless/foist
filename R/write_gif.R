

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Write a numeric matrix to an uncompressed GIF file
#'
#' @param data numeric 2d
#' @param filename output filename e.g. "example.ppm"
#' @param convert_to_row_major Convert to row-major order before output. R stores matrix
#'        and array data in column-major order. In order to output row-major order (as
#'        expected by normal GIF output) data ordering must be converted. If this argument
#'        is set to FALSE, then image output will be faster (due to fewer data-ordering operations, and
#'        better cache coherency) but the image will be transposed. Default: TRUE
#' @param flipy By default, the position [0, 0] is considered the top-left corner of the output image.
#'        Set flipy = TRUE for [0, 0] to represent the bottom-left corner.  This operation
#'        is very fast and has negligible impact on overall write speed.
#'        Default: flipy = FALSE.
#' @param invert invert all the pixel brightness values - as if the image were
#'        converted into a negative. Dark areas become bright and bright areas become dark.
#'        Default: FALSE
#' @param intensity_factor Multiplication factor applied to all values in image
#'        (note: no checking is performed to ensure values remain in range [0, 1]).
#'        If intensity_factor <= 0, then automatically determine (and apply) a multiplication factor
#'        to set the maximum value to 1.0. Default: intensity_factor = 1.0
#' @param pal integer matrix of size 128x3 or 256x3 with values in the range [0, 255]. Each
#'        row represents the r, g, b colour for a given grey index value. This
#'        GIF writer only supports 128 colours, so a 256x3 palette is reduced
#'        to 128 colours by selecting every second colour. If supplied with a
#'        128-colour-palette then it is used as-is.
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
write_gif <- function(data, filename,
                      convert_to_row_major = TRUE,
                      flipy                = FALSE,
                      invert               = FALSE,
                      intensity_factor     = 1,
                      pal                  = grey128) {
    invisible(.Call(`_foist_write_gif_core`, data, dim(data), filename,
                    convert_to_row_major, flipy, invert, intensity_factor, pal))
}
