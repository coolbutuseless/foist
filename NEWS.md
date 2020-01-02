
# foist 0.1.7

* Fix an error with writing the PNG header
* Update package 'Suggests' for packages needed for testing

# foist 0.1.6

* Added `write_gif()`


# foist 0.1.5

* Added `invert` option to `write_png()` and `write_pnm()`
    * The default output maps 0 to black and 1 to white.
    * Setting `invert = TRUE` switches 1 to be black and 0 to be white.


# foist 0.1.4

* Now correctly rounding doubles when scaling and converting to unsigned char for output.
* Support palettes from 2 to 256 colours


# foist 0.1.3

* Added PNG support


# foist 0.1.2

* Refactored `write_pgm()`, `write_ppm()`, `write_pal_pgm()` 
  into single function `write_pnm()`


# foist 0.1.1

* Added `flipy` flag
* Better handling of column-major/row-major by swapping nrow and ncol 
* Since image size and buffer size are both known, the process of buffer output
  to file has been streamlined.


# foist 0.1.0

* Initial Release
