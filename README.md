
<!-- README.md is generated from README.Rmd. Please edit that file -->

# FOIst - Fast Output of Images

<!-- badges: start -->

![](https://img.shields.io/badge/Status-alpha-orange.svg)
![](https://img.shields.io/badge/Version-0.1.2-blue.svg)
<!-- badges: end -->

#### `foist` is a very fast way to output a matrix or array to a lossless image file.

**`foist` writes lossless grey image files \~5.5x faster than the `png`
library.**

**`foist` writes lossless RGB image files \~7.5x faster than the `png`
library**

**`foist` allocates 300-1000x *less* memory than png.**

  - Only supports writing (lossless)
    [NETPBM](http://netpbm.sourceforge.net/) PGM (grey) and PPM (RGB)
    files
  - Only supports 8-bits-per-channel grey and RGB images.
  - Uses [Rcpp](https://cran.r-project.org/package=Rcpp) to do
      - **type conversion** from *double* to *unsigned byte*, and
      - **data re-ordering** from R’s column-major ordering to image
        output with row-major ordering
  - By specifying `convert_to_row_major = FALSE` the image data-ordering
    is not converted from R’s native data ordering - this makes image
    saving very fast as data manipulation operations are minimised and
    cache coherency is much improved. However, the image will appear
    tranposed in the output.

## What’s in the box

  - `write_pnm()` which can
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

## Caveats

Don’t look at my C code unless you’d like a heart attack - or perhaps to
lend a hand\!

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
write_pnm(dbl_mat, "man/figures/col-0-n.pgm")
write_pnm(dbl_mat, "man/figures/col-0-f.pgm", flipy = TRUE)
write_pnm(dbl_mat, "man/figures/col-0-t.pgm", convert_to_row_major = FALSE)
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
write_pnm(dbl_arr, filename = "man/figures/col-1-n.ppm")
write_pnm(dbl_arr, filename = "man/figures/col-1-f.ppm", flipy = TRUE)
write_pnm(dbl_arr, filename = "man/figures/col-1-t.ppm", convert_to_row_major = FALSE)
```

<div>

<img src = "man/figures/col-convert-1-n.png"  width = "30%" title = "convert_to_row_major = TRUE">
<img src = "man/figures/col-convert-1-f.png"  width = "30%" title = "flipy = TRUE"                  style = "margin-left: 30px;">
<img src = "man/figures/col-convert-1-t.png"  width = "19%" title = "convert_to_row_major = FALSE"  style = "margin-left: 30px;">

</div>

## Save a *matrix* to an RGB image using a palette lookup

`foist` can write a grey image as an RGB image by using each grey pixel
value to lookup an RGB colour in a given palette.

A palette must be an integer matrix with dimensions 256 x 3 and values
in the range \[0, 255\].

`foist` includes the 5 palettes from
[viridis](https://cran.r-project.org/package=viridis) as `vir$magma`
etc.

``` r
foist::write_pnm(dbl_mat,                           "man/figures/col-0.pgm")
foist::write_pnm(dbl_mat, pal = foist::vir$magma  , "man/figures/col-3.ppm")
foist::write_pnm(dbl_mat, pal = foist::vir$inferno, "man/figures/col-4.ppm")
foist::write_pnm(dbl_mat, pal = foist::vir$plasma , "man/figures/col-5.ppm")
foist::write_pnm(dbl_mat, pal = foist::vir$viridis, "man/figures/col-6.ppm")
foist::write_pnm(dbl_mat, pal = foist::vir$cividis, "man/figures/col-7.ppm")
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

The following benchmarks the output of a 2D numeric matrix to a grey
image.

  - `foist::write_pnm()` in both row-major and column-major ordering
      - by **not** converting to row-major ordering the data is written
        in the same order it is stored in R. By minimizing this data
        manipulation some significant speedups are achieved.
  - `png::writePNG()`
  - `jpeg::writeJPEG()` - I’m not actually interested in lossy output,
    but it is interesting to note just how fast the jpeg library is.

<!-- end list -->

``` r
tmp <- tempfile()

res <- bench::mark(
  `foist::write_pnm()`                    = foist::write_pnm(dbl_mat, tmp),
  `foist::write_pnm(column-major)`        = foist::write_pnm(dbl_mat, tmp, convert_to_row_major = FALSE),
  `foist::write_pnm(column-major, flipy)` = foist::write_pnm(dbl_mat, tmp, convert_to_row_major = FALSE, flipy = TRUE),
  `png::writePNG()`                       = png::writePNG   (dbl_mat, tmp),
  `jpeg::writeJPEG`                       = jpeg::writeJPEG (dbl_mat, tmp),
  min_time = 2, check = FALSE
)
```

| expression                             |     min |    mean |  median | itr/sec | mem\_alloc |
| :------------------------------------- | ------: | ------: | ------: | ------: | ---------: |
| foist::write\_pnm()                    |  2.95ms |  3.87ms |  3.56ms |     258 |     2.49KB |
| foist::write\_pnm(column-major)        |  2.05ms |  2.48ms |  2.36ms |     403 |     2.49KB |
| foist::write\_pnm(column-major, flipy) |  2.06ms |  2.57ms |  2.38ms |     390 |     2.49KB |
| png::writePNG()                        | 12.09ms | 13.63ms | 13.65ms |      73 |   673.21KB |
| jpeg::writeJPEG                        |  6.09ms |  7.58ms |   7.5ms |     132 |   663.55KB |

Benchmark results

<img src="man/figures/README-benchmark_grey-1.png" width="100%" />

## Benchmark: Saving an array as an RGB image

The following benchmarks the output of a 3D numeric array to a colour
image.

  - `foist::write_pnm()` in both row-major and column-major ordering
      - by **not** converting to row-major ordering the data is written
        in the same order it is stored in R. By minimizing this data
        manipulation some significant speedups are achieved.
  - `png::writePNG()`
  - `jpeg::writeJPEG()` - I’m not actually interested in lossy output,
    but it is interesting to note just how fast the jpeg library is.

<!-- end list -->

``` r
tmp <- tempfile()

res <- bench::mark(
  `foist::write_pnm()`                    = foist::write_pnm(dbl_arr, tmp),
  `foist::write_pnm(column-major)`        = foist::write_pnm(dbl_arr, tmp, convert_to_row_major = FALSE),
  `foist::write_pnm(column-major, flipy)` = foist::write_pnm(dbl_arr, tmp, convert_to_row_major = FALSE, flipy = TRUE),
  `png::writePNG()`                       = png::writePNG   (dbl_arr, tmp),
  `jpeg::writeJPEG`                       = jpeg::writeJPEG (dbl_arr, tmp),
  min_time = 2, check = FALSE
)
```

| expression                             |     min |    mean |  median | itr/sec | mem\_alloc |
| :------------------------------------- | ------: | ------: | ------: | ------: | ---------: |
| foist::write\_pnm()                    | 18.45ms | 21.83ms | 21.67ms |      46 |     2.49KB |
| foist::write\_pnm(column-major)        |  4.76ms |  5.99ms |  5.73ms |     167 |     2.49KB |
| foist::write\_pnm(column-major, flipy) |     5ms |  6.33ms |  5.82ms |     158 |     2.49KB |
| png::writePNG()                        | 45.55ms | 48.29ms | 47.83ms |      21 |     1.88MB |
| jpeg::writeJPEG                        | 25.82ms | 28.44ms | 28.33ms |      35 |     1.88MB |

Benchmark results

<img src="man/figures/README-benchmark_rgb-1.png" width="100%" />
