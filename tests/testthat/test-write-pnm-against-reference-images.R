context("test-pnm-against-reference-images")


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Set up data
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ncol    <- 256
nrow    <- 100
int_vec <- rep.int(seq(ncol) - 1, nrow) %% 256L
int_mat <- matrix(int_vec, nrow = nrow, ncol = ncol, byrow = TRUE)
dbl_mat <- int_mat/255


r <- dbl_mat
g <- dbl_mat[  rev(seq(nrow(dbl_mat))), ]
b <- dbl_mat[, rev(seq(ncol(dbl_mat)))  ]

dbl_arr <- array(c(r, g, b), dim = c(nrow, ncol, 3))

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# PGM output works
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
test_that("PGM works", {

  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Set up filenames
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  png_filename <- "reference_images/grey.png"
  ref_filename <- "reference_images/grey-ref.pgm"

  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Create the reference image by saving to PNG and converting
  # to PGM using imagemagick
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (interactive()) {
    pgm_filename <- "reference_images/grey.pgm"
    png::writePNG(dbl_mat, png_filename)
    system(glue::glue("convert '{png_filename}' '{ref_filename}'"))
  } else{
    pgm_filename <- tempfile("grey-", fileext = ".pgm")
  }



  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Write the PGM using 'foist'
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  write_pnm(dbl_mat, pgm_filename)

  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Assert that the output pgm is identical to the one via png + imagemagick
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ref_image  <- readBin(ref_filename, what = 'raw', n = 1e6)
  test_image <- readBin(pgm_filename, what = 'raw', n = 1e6)
  expect_identical(ref_image, test_image)
})





#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# PPM output works
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
test_that("PPM from array works", {

  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Set up filenames
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  png_filename <- "reference_images/col.png"
  ref_filename <- "reference_images/col-ref.ppm"

  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Create the reference image by saving to PNG and converting
  # to PGM using imagemagick
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (interactive()) {
    ppm_filename <- "reference_images/col.ppm"
    png::writePNG(dbl_arr, png_filename)
    # jpeg::writeJPEG(dbl_arr, "reference_images/col.jpg")
    system(glue::glue("convert '{png_filename}' '{ref_filename}'"))
  } else {
    ppm_filename <- tempfile("col-", fileext = ".ppm")
  }



  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Write the PPM using 'foist'
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  write_pnm(dbl_arr, filename = ppm_filename)

  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Assert that the output pgm is identical to the one via png + imagemagick
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ref_image  <- readBin(ref_filename, what = 'raw', n = 1e6)
  test_image <- readBin(ppm_filename, what = 'raw', n = 1e6)
  expect_identical(ref_image, test_image)
})

