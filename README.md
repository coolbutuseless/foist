
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

**`foist` can write lossless grey image files \~5.5x faster than the
`png` library.**

**`foist` can write lossless RGB image files \~7.5x faster than the
`png` library.**

**`foist` allocates a lot less memory than other methods.**

<br/>

  - Supports writing lossless images in two formats:
      - [NETPBM](http://netpbm.sourceforge.net/) PGM (grey) and PPM
        (RGB) files
      - [PNG](https://en.wikipedia.org/wiki/Portable_Network_Graphics)
        files - Grey, RGB or with a fixed 256 colour palette.
  - Only supports 8-bits-per-channel grey and RGB images.
  - Part of the speed of `foist` comes from using
    [Rcpp](https://cran.r-project.org/package=Rcpp) to quickly scale,
    manipulate and re-order data for image output.
  - The fastest output speed is achieved by writing the data in the same
    order in which it is stored in R (column major ordering). The price
    paid for this speed is that the image will appear transposed in the
    output.

## What’s in the box

  - `write_pnm()` and `write_png()` which can both:
      - write an array to an RGB image
      - write a matrix to a grey image
      - write a matrix to a RGB image with a supplied palette
  - `vir` 5 palettes from
    [viridis](https://cran.r-project.org/package=viridis) in the
    appropriate format

This package would not be possible without:

  - [Rcpp](https://cran.r-project.org/package=Rcpp) - The easiest way to
    get fast C/C++ code into R.
  - [viridis](https://cran.r-project.org/package=viridis) - Wnderful
    palettes originally from [matplotlib](http://matplotlib.org).
  - [NETPBM](http://netpbm.sourceforge.net) - A 30-year-old, rock-solid,
    uncompressed image format.
  - [PNG](https://www.w3.org/TR/PNG/) - A 20-year old, rock-solid image
    format with lossless compression (which also supports uncompressed
    image data).

## Caveats

Don’t look at my C/C++ code unless you’d like a heart attack.

## Technical Notes

  - `foist` contains a bespoke, minimalist PNG encoder I wrote in C++
      - This was written so I’d have complete control over the image
        output.
      - There is no lossless compression enabled in this PNG encoder
        i.e. only uncompressed DEFLATE blocks are used (see
        <https://datatracker.ietf.org/doc/rfc1951> Sect 3.2.4).
      - The IDAT and ZLIB/DEFLATE blocks are output in sync
        (one-DEFLATE-block-per-IDAT-chunk) as this made the PNG
        implementation much simpler.
      - The encoder uses Mark Adler’s `adler32.c` code from
        [zlib](https://www.zlib.net/) Copyright (C) 1995-2011, 2016 Mark
        Adler.
      - A SIMD version of `adler32()` is included but not enabled by
        default. The speed gains weren’t significant enough for the
        machine imcompatibility headaches it would introduce. See
        `#define ADLER32_SSE` in `write-png.cpp` if you’d like to try
        it.
      - `crc32` implementation is a very fast slice-by-16 implementation
        by [Stephan Brumme](https://create.stephan-brumme.com/crc32/).
        This is noticeably much faster than the slice-by-4 crc32 that
        comes with the standard [zlib library](https://www.zlib.net/).
  - When converting the numeric values to unsigned bytes for image
    output, they are truncated to integer rather than rounded.
  - Because PNG data also needs CRC32 and ADLER32 checksumming it is
    generally slower than PGM/PPM output.
  - However, writing a matrix with a palette will be faster in PNG as it
    has direct support for indexed colours, whereas for a NETPBM PPM
    file the intensity values need to be explicitly mapped to an RGB
    triplet and then written out in full.

## Installation

You can install the package from
[GitHub](https://github.com/coolbutuseless/foist) with:

``` r
# install.packages("devtools")
devtools::install_github("coolbutuseless/foist")
```

## Setup data

  - `dbl_mat` - A 2D numeric matrix for output to a grey image. All
    values in range \[0, 1\]
  - `dbl_arr` - A 3d numeric array for output to an RGB image. All
    values in range \[0, 1\]

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

`write_png()` and `write_pnm()` will save a 2D numeric **matrix** as a
grey image.

  - The matrix values must be in the range \[0, 1\].
  - Use the `intensity_factor` argument to scale image values on-the-fly
    as they are written to file.

<!-- end list -->

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

`write_png()` and `write_pnm()` will save a 3D numeric **array** as an
RGB image.

  - Array dimensions must be NxMx3 where the 3 colour planes correspond
    to the third dimension of the array.
  - The matrix values must be in the range \[0, 1\].
  - Use the `intensity_factor` argument to scale image values on-the-fly
    as they are written to file.

<!-- end list -->

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

`write_png()` and `write_pnm()` will save a 2D numeric **matrix** as an
RGB image if also supplied with a colour palette.

  - A palette must be an integer matrix with dimensions 256 x 3 and
    values in the range \[0, 255\].
  - Pixel values in the matrix are first scaled into the range \[0,
    255\] and are then mapped to one of the RGB colours in the palette.
  - The matrix values must be in the range \[0, 1\].
  - Use the `intensity_factor` argument to scale image values on-the-fly
    as they are written to file.

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
  - `foist::write_png()` in both row-major and column-major ordering
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

| expression                      |    min |   mean |  median | itr/sec | mem\_alloc |
| :------------------------------ | -----: | -----: | ------: | ------: | ---------: |
| foist::write\_pnm()             | 3.06ms | 4.41ms |  4.16ms |     227 |     2.49KB |
| foist::write\_pnm(column-major) |  2.1ms | 2.62ms |  2.44ms |     382 |     2.49KB |
| foist::write\_png()             | 3.38ms | 4.29ms |  4.02ms |     233 |     2.49KB |
| foist::write\_png(column-major) | 2.33ms |  2.9ms |  2.75ms |     344 |     2.49KB |
| png::writePNG()                 | 12.4ms | 14.4ms | 14.27ms |      69 |   673.21KB |

Benchmark results

<img src="man/figures/README-benchmark_grey-1.png" width="100%" />

## Benchmark: Saving an array as an RGB image

The following benchmark compares the time to output a colour image
using:

  - `foist::write_pnm()` in both row-major and column-major ordering
  - `foist::write_png()` in both row-major and column-major ordering
  - `png::writePNG()`

<!-- end list -->

``` r
tmp <- tempfile()

res <- bench::mark(
  `foist::write_pnm()`                    = foist::write_pnm(dbl_arr, tmp),
  `foist::write_pnm(column-major)`        = foist::write_pnm(dbl_arr, tmp, convert_to_row_major = FALSE),
  
  `foist::write_png()`                    = foist::write_png(dbl_arr, tmp),
  `foist::write_png(column-major)`        = foist::write_png(dbl_arr, tmp, convert_to_row_major = FALSE),
  
  `foist::write_png(indexed colour)`      = foist::write_png(dbl_mat, tmp, convert_to_row_major = FALSE, pal = foist::vir$magma),
  
  `png::writePNG()`                       = png::writePNG   (dbl_arr, tmp),
  min_time = 2, check = FALSE
)
```

| expression                        |     min |    mean |  median | itr/sec | mem\_alloc |
| :-------------------------------- | ------: | ------: | ------: | ------: | ---------: |
| foist::write\_pnm()               | 18.73ms |  22.5ms | 22.58ms |      44 |     2.49KB |
| foist::write\_pnm(column-major)   |  4.86ms |  6.45ms |  6.05ms |     155 |     2.49KB |
| foist::write\_png()               | 19.69ms | 22.28ms | 22.23ms |      45 |     2.49KB |
| foist::write\_png(column-major)   |  6.27ms |  7.68ms |  7.31ms |     130 |     2.49KB |
| foist::write\_png(indexed colour) |  2.41ms |  2.98ms |  2.77ms |     336 |     2.49KB |
| png::writePNG()                   |  46.4ms | 50.09ms |  49.7ms |      20 |     1.88MB |

Benchmark results

<img src="man/figures/README-benchmark_rgb-1.png" width="100%" />
