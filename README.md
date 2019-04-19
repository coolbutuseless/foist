
<!-- README.md is generated from README.Rmd. Please edit that file -->

# FOIst - Fast Output of Images

<!-- badges: start -->

![](https://img.shields.io/badge/Status-alpha-orange.svg)
![](https://img.shields.io/badge/Version-0.1.3-blue.svg)
![](https://img.shields.io/badge/Output-PNG-green.svg)
![](https://img.shields.io/badge/Output-PGM-green.svg)
![](https://img.shields.io/badge/Output-PPM-green.svg)
<!-- badges: end -->

#### `foist` is a very fast way to output a matrix or array to a lossless image file.

<br/>

**`foist` writes lossless grey image files \~5.5x faster than the `png`
library.**

**`foist` writes lossless RGB image files \~7.5x faster than the `png`
library**

**`foist` allocates 300-1000x *less* memory than png.**

<br/>

  - Only supports writing (lossless) images:
      - [NETPBM](http://netpbm.sourceforge.net/) PGM (grey) and PPM
        (RGB) files
      - [PNG](https://en.wikipedia.org/wiki/Portable_Network_Graphics)
        files - Grey, RGB or with a fixed 256 colour palette.
  - Only supports 8-bits-per-channel grey and RGB images.
  - Uses [Rcpp](https://cran.r-project.org/package=Rcpp) to do
      - type conversion from *double* to *unsigned byte*, and
      - data re-ordering from R’s column-major ordering to image output
        with row-major ordering
  - By specifying `convert_to_row_major = FALSE` the image data-ordering
    is not converted from R’s native data ordering - this makes image
    saving very fast as data manipulation operations are minimised and
    cache coherency is much improved. However, the image will appear
    tranposed in the output.

## What’s in the box

  - `write_pnm()` and `write_png()` which can
      - write an array to an RGB image
      - write a matrix to a grey image
      - write a matrix to a RGB image with a supplied palette
  - `vir` 5 palettes from
    [viridis](https://cran.r-project.org/package=viridis) in the
    appropriate format

This package would not be possible without:

  - [Rcpp](https://cran.r-project.org/package=Rcpp) - The easiest way to
    get fast C/C++ code into R
  - [viridis](https://cran.r-project.org/package=viridis) - The
    wonderful palettes originally from
    [matplotlib](http://matplotlib.org)
  - [NETPBM](http://netpbm.sourceforge.net) - A 30-year-old, rock-solid,
    uncompressed image format
  - [PNG](https://www.w3.org/TR/PNG/)

## Caveats

Don’t look at my C/C++ code unless you’d like a heart attack.

## Technical Notes

  - `foist` contains a bespoke PNG encoder I wrote in C++
      - Encoder uses Mark Adler’s `adler32.c` code from
        [zlib](https://www.zlib.net/) Copyright (C) 1995-2011, 2016 Mark
        Adler
      - A SIMD version of `adler32()` is included but not enabled by
        default. See `#define ADLER_SIMD` in `write-png.cpp`
      - `crc32` implementation is a very fast slice-by-16 implementation
        by [Stephan Brumme](https://create.stephan-brumme.com/crc32/).
        This is noticeably much faster than the slice-by-4 crc32 that
        comes with the standard [zlib library](https://www.zlib.net/) or
        in the R package
        [digest](https://cran.r-project.org/package=digest)
  - When converting the numeric values to unsigned bytes for image
    output, they are truncated to integer rather than rounded

## Installation

You can install the development version from
[GitHub](https://github.com/coolbutuseless/foist) with:

``` r
# install.packages("devtools")
devtools::install_github("coolbutuseless/foist")
```

## Setup data

  - `dbl_mat` - A numeric matrix to output to a grey image. All values
    in range \[0, 1\]
  - `dbl_arr` - A 3d numeric array to output to an RGB image. All values
    in range \[0, 1\]

<!-- end list -->

``` r
ncol    <- 256
nrow    <- 160
int_vec <- seq(nrow * ncol) %% 254L
int_mat <- matrix(int_vec, nrow = nrow, ncol = ncol, byrow = TRUE)
dbl_mat <- int_mat/255

# A non-boring RGB array/image
r       <- dbl_mat
g       <- matrix(rep(seq(0, 255, length.out = nrow)/255, each = ncol), nrow, ncol, byrow = TRUE)
b       <- dbl_mat[, rev(seq(ncol(dbl_mat)))  ]
dbl_arr <- array(c(r, g, b), dim = c(nrow, ncol, 3))
```

## Save a *2D matrix* as a grey image

A 2D numeric **matrix** can be saved as a grey image..

The matrix values must be in the range \[0, 1\]. Use the
`intensity_factor` argument to scale image values on-the-fly as they are
written to file.

``` r
# NETPBM PGM
write_pnm(dbl_mat, "man/figures/col-0-n.pgm")
write_pnm(dbl_mat, "man/figures/col-0-f.pgm", flipy = TRUE)
write_pnm(dbl_mat, "man/figures/col-0-t.pgm", convert_to_row_major = FALSE)

# PNG
write_png(dbl_mat, "man/figures/col-0-n.png")
write_png(dbl_mat, "man/figures/col-0-f.png", flipy = TRUE)
write_png(dbl_mat, "man/figures/col-0-t.png", convert_to_row_major = FALSE)
```

<div>

<img src = "man/figures/col-convert-0-n.png"  width = "30%" title = "no scaling">
<img src = "man/figures/col-convert-0-f.png"  width = "30%" title = "flipy = TRUE"            style = "margin-left: 30px;">
<img src = "man/figures/col-convert-0-t.png"  width = "19%" title = "intensity_factor = 0.5"  style = "margin-left: 30px;">

</div>

## Save a *3D array* as an RGB image

An NxMx3 **array** can be saved as an RGB image. Each of the colours is
represented of one of the 3 planes of the array.

The array values must be in the range \[0, 1\]. Use the
`intensity_factor` argument to scale image values on-the-fly as they are
written to file.

``` r
# NETPBM PPM format
write_pnm(dbl_arr, filename = "man/figures/col-1-n.ppm")
write_pnm(dbl_arr, filename = "man/figures/col-1-f.ppm", flipy = TRUE)
write_pnm(dbl_arr, filename = "man/figures/col-1-t.ppm", convert_to_row_major = FALSE)

# PNG
write_pnm(dbl_arr, filename = "man/figures/col-1-n.png")
write_pnm(dbl_arr, filename = "man/figures/col-1-f.png", flipy = TRUE)
write_pnm(dbl_arr, filename = "man/figures/col-1-t.png", convert_to_row_major = FALSE)
```

<div>

<img src = "man/figures/col-convert-1-n.png"  width = "30%" title = "convert_to_row_major = TRUE">
<img src = "man/figures/col-convert-1-f.png"  width = "30%" title = "flipy = TRUE"                  style = "margin-left: 30px;">
<img src = "man/figures/col-convert-1-t.png"  width = "19%" title = "convert_to_row_major = FALSE"  style = "margin-left: 30px;">

</div>

## Save a *matrix* to an RGB image using a palette lookup

`foist` can write a grey image as an RGB image by using each grey pixel
value to lookup an RGB colour in a given palette.

Using `write_png()` to write a paletted image will be faster than
`write_pnm()` because the PNG format directly supports palettes.

A palette must be an integer matrix with dimensions 256 x 3 and values
in the range \[0, 255\].

`foist` includes the 5 palettes from
[viridis](https://cran.r-project.org/package=viridis) as `vir$magma`
etc.

``` r
# NETPBM format
foist::write_pnm(dbl_mat,                           "man/figures/col-0.pgm")
foist::write_pnm(dbl_mat, pal = foist::vir$magma  , "man/figures/col-3.ppm")
foist::write_pnm(dbl_mat, pal = foist::vir$inferno, "man/figures/col-4.ppm")
foist::write_pnm(dbl_mat, pal = foist::vir$plasma , "man/figures/col-5.ppm")
foist::write_pnm(dbl_mat, pal = foist::vir$viridis, "man/figures/col-6.ppm")
foist::write_pnm(dbl_mat, pal = foist::vir$cividis, "man/figures/col-7.ppm")

# PNG format
foist::write_png(dbl_mat,                           "man/figures/col-0.png")
foist::write_png(dbl_mat, pal = foist::vir$magma  , "man/figures/col-3.png")
foist::write_png(dbl_mat, pal = foist::vir$inferno, "man/figures/col-4.png")
foist::write_png(dbl_mat, pal = foist::vir$plasma , "man/figures/col-5.png")
foist::write_png(dbl_mat, pal = foist::vir$viridis, "man/figures/col-6.png")
foist::write_png(dbl_mat, pal = foist::vir$cividis, "man/figures/col-7.png")
```

<div>

<img src = "man/figures/col-convert-0-n.png" width = "30%" title = "grey">
<img src = "man/figures/col-convert-3.png"   width = "30%" title = "magma">
<img src = "man/figures/col-convert-4.png"   width = "30%" title = "inferno">
<img src = "man/figures/col-convert-5.png"   width = "30%" title = "plasma">
<img src = "man/figures/col-convert-6.png"   width = "30%" title = "viridis">
<img src = "man/figures/col-convert-7.png"   width = "30%" title = "cividis">

</div>

## Benchmark: Saving a matrix as a grey image

The following benchmark compares the time to output of a grey image
using:

  - `foist::write_pnm()` in both row-major and column-major ordering
      - by **not** converting to row-major ordering the data is written
        in the same order it is stored in R. By minimizing this data
        manipulation some significant speedups are achieved.
  - `png::writePNG()`

<!-- end list -->

``` r
tmp <- tempfile()

res <- bench::mark(
  `foist::write_pnm()`                    = foist::write_pnm(dbl_mat, tmp),
  `foist::write_pnm(column-major)`        = foist::write_pnm(dbl_mat, tmp, convert_to_row_major = FALSE),
  
  `foist::write_png()`                    = foist::write_png(dbl_mat, tmp),
  `foist::write_png(column-major)`        = foist::write_png(dbl_mat, tmp, convert_to_row_major = FALSE),
  
  `png::writePNG()`                       = png::writePNG   (dbl_mat, tmp),
  min_time = 2, check = FALSE
)
```

| expression                      |     min |    mean |  median | itr/sec | mem\_alloc |
| :------------------------------ | ------: | ------: | ------: | ------: | ---------: |
| foist::write\_pnm()             |  3.04ms |   4.5ms |  4.16ms |     222 |     2.49KB |
| foist::write\_pnm(column-major) |  2.07ms |  2.63ms |  2.44ms |     380 |     2.49KB |
| foist::write\_png()             |  3.34ms |  4.11ms |  3.84ms |     243 |     2.49KB |
| foist::write\_png(column-major) |  2.45ms |  2.95ms |  2.76ms |     339 |     2.49KB |
| png::writePNG()                 | 12.31ms | 14.45ms | 14.22ms |      69 |   673.21KB |

Benchmark results

<img src="man/figures/README-benchmark_grey-1.png" width="100%" />

## Benchmark: Saving an array as an RGB image

The following benchmark compares the time to output a colour image
using:

  - `foist::write_pnm()` in both row-major and column-major ordering
      - by **not** converting to row-major ordering the data is written
        in the same order it is stored in R. By minimizing this data
        manipulation some significant speedups are achieved.
  - `png::writePNG()`

<!-- end list -->

``` r
tmp <- tempfile()

res <- bench::mark(
  `foist::write_pnm()`                    = foist::write_pnm(dbl_arr, tmp),
  `foist::write_pnm(column-major)`        = foist::write_pnm(dbl_arr, tmp, convert_to_row_major = FALSE),
  
  `foist::write_png()`                    = foist::write_png(dbl_arr, tmp),
  `foist::write_png(column-major)`        = foist::write_png(dbl_arr, tmp, convert_to_row_major = FALSE),
  
  `png::writePNG()`                       = png::writePNG   (dbl_arr, tmp),
  min_time = 2, check = FALSE
)
```

| expression                      |     min |    mean |  median | itr/sec | mem\_alloc |
| :------------------------------ | ------: | ------: | ------: | ------: | ---------: |
| foist::write\_pnm()             | 19.24ms | 22.57ms | 21.15ms |      44 |     2.49KB |
| foist::write\_pnm(column-major) |  4.61ms |  6.27ms |  5.86ms |     160 |     2.49KB |
| foist::write\_png()             | 20.11ms | 22.05ms | 21.79ms |      45 |     2.49KB |
| foist::write\_png(column-major) |  6.12ms |  7.61ms |  7.18ms |     131 |     2.49KB |
| png::writePNG()                 | 45.77ms | 49.74ms | 49.39ms |      20 |     1.88MB |

Benchmark results

<img src="man/figures/README-benchmark_rgb-1.png" width="100%" />
