context("validate PNG output against output from PNG package")


create_data <- function(ncol, nrow) {
  int_vec <- seq(nrow * ncol) %% 254L
  int_mat <- matrix(int_vec, nrow = nrow, ncol = ncol, byrow = TRUE)
  dbl_mat <- int_mat/255
  zero_mat <- dbl_mat
  zero_mat[] <- as.integer(runif(nrow* ncol, min=0, max=255))

  r       <- dbl_mat
  g       <- matrix(rep(seq(0, 255, length.out = nrow)/255, each = ncol), nrow, ncol, byrow = TRUE)
  b       <- dbl_mat[, rev(seq(ncol(dbl_mat)))  ]

  dbl_arr <- array(c(r, g, b), dim = c(nrow, ncol, 3))
  # dbl_arr <- array(c(dbl_mat, dbl_mat, zero_mat), dim = c(nrow, ncol, 3))

  list(mat = dbl_mat, arr = dbl_arr)
}



test_that("write_png_core works for grey", {

  testdir <- tempdir()

  new_png <- file.path(testdir, "new.png")
  new_pnm <- file.path(testdir, "new.pnm")

  ref_png <- file.path(testdir, "ref.png")
  ref_pnm <- file.path(testdir, "ref.pnm")

  size <- c(1, 10, 50, 1000)
  for (ncol in size) {
    for (nrow in size) {
      dbl_mat <- create_data(ncol = ncol, nrow = nrow)$mat

      write_png_core(dbl_mat, dims = dim(dbl_mat), filename = new_png)
      png::writePNG(dbl_mat, ref_png)

      system(glue::glue("pngtopnm {new_png} > {new_pnm}"))
      system(glue::glue("pngtopnm {ref_png} > {ref_pnm}"))


      dig1 <- readBin(new_pnm , what = 'raw', size = 1, n = 1e6)
      dig2 <- readBin(ref_pnm , what = 'raw', size = 1, n = 1e6)

      expect_identical(
        digest::digest(readBin(new_pnm , what = 'raw', size = 1, n = 1e6), algo = 'sha1'),
        digest::digest(readBin(ref_pnm , what = 'raw', size = 1, n = 1e6), algo = 'sha1')
      )

    }
  }

})



test_that("write_png_core works for RGB", {

  testdir <- tempdir()

  new_png <- file.path(testdir, "new.png")
  new_pnm <- file.path(testdir, "new.pnm")

  ref_png <- file.path(testdir, "ref.png")
  ref_pnm <- file.path(testdir, "ref.pnm")

  size <- c(10, 50, 1000)
  for (ncol in size) {
    for (nrow in size) {
      dbl_arr <- create_data(ncol = ncol, nrow = nrow)$arr

      write_png_core(dbl_arr, dims = dim(dbl_arr), filename = new_png)
      png::writePNG(dbl_arr, ref_png)

      system(glue::glue("pngtopnm {new_png} > {new_pnm}"))
      system(glue::glue("pngtopnm {ref_png} > {ref_pnm}"))

      dig1 <- readBin(new_pnm , what = 'raw', size = 1, n = 1e6)
      dig2 <- readBin(ref_pnm , what = 'raw', size = 1, n = 1e6)

      expect_identical(
        digest::digest(readBin(new_pnm , what = 'raw', size = 1, n = 1e6), algo = 'sha1'),
        digest::digest(readBin(ref_pnm , what = 'raw', size = 1, n = 1e6), algo = 'sha1')
      )
    }
  }

})
