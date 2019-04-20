

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
