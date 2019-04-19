#include <fstream>
#include "Rcpp.h"

using namespace Rcpp;

#define BUFFER_ROWS 20



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// ooooooooo.             oooo                .       .
// `888   `Y88.           `888              .o8     .o8
//  888   .d88'  .oooo.    888   .ooooo.  .o888oo .o888oo  .ooooo.
//  888ooo88P'  `P  )88b   888  d88' `88b   888     888   d88' `88b
//  888          .oP"888   888  888ooo888   888     888   888ooo888
//  888         d8(  888   888  888    .o   888 .   888 . 888    .o
// o888o        `Y888""8o o888o `Y8bod8P'   "888"   "888" `Y8bod8P'
//
//
// - Write PALETTE image data
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void write_pnm_grey_data_with_palette(std::ofstream &outfile, NumericVector vec,
                                      unsigned int ncol,
                                      unsigned int nrow,
                                      double scale_factor,
                                      bool convert_to_row_major,
                                      bool flipy,
                                      IntegerMatrix pal) {

  unsigned int depth = 3;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Sanity check the palette is in the correct format
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (pal.nrow() != 256 | pal.ncol() != 3) {
    stop("\'pal\' must be a 256x3 IntegerMatrix with values in the range [0,255]");
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set up buffer to write only BUFFER_ROWS rows a time
  // Reduces memory usage (by not allocating full size copy of the image)
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  unsigned int buffer_size = BUFFER_ROWS * ncol * depth;
  unsigned int remainder_size = (nrow % BUFFER_ROWS) * ncol * depth;
  unsigned char *uc0 = (unsigned char *) calloc(buffer_size, sizeof(unsigned char));
  if (!uc0) stop("write_pnm(): out of memory");
  unsigned char *uc = uc0;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Get a pointer to the actual data in the supplied matrix
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  double *v0 = vec.begin();


  if (convert_to_row_major) {
    // Convert from R's column-major ordering to row-major output order
    for (unsigned int row = 0; row < nrow; row++) {
      unsigned int j = flipy ? nrow - 1 - row : row;
      for (unsigned int col = 0; col < ncol; col ++) {
        unsigned char val = (unsigned char)(v0[j] * scale_factor);
        *uc++ = pal(val, 0);
        *uc++ = pal(val, 1);
        *uc++ = pal(val, 2);
        j += nrow;
      }

      // Flush the buffer to file
      if ((row + 1) % BUFFER_ROWS == 0) {
        outfile.write((char *)uc0, sizeof(unsigned char) * buffer_size);
        uc = uc0;
      }
    }
  } else {
    // Write pixels in R's column-major ordering
    for (unsigned int row = 0; row < nrow; row++) {
      const unsigned int offset = flipy ? nrow - 1 - row : row;
      double *v = v0 + ncol * offset;
      for (unsigned int col = 0; col < ncol; col ++) {
        unsigned char val = (unsigned char)(*v++ * scale_factor);
        *uc++ = pal(val, 0);
        *uc++ = pal(val, 1);
        *uc++ = pal(val, 2);
      }

      // Flush the buffer to file
      if ((row + 1) % BUFFER_ROWS == 0) {
        outfile.write((char *)uc0, sizeof(unsigned char) * buffer_size);
        uc = uc0;
      }
    }
  }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Flush any remaining values to file
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  outfile.write((char *)uc0, sizeof(unsigned char) * remainder_size);

  free(uc0);
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// ooooooooo.     .oooooo.    oooooooooo.
// `888   `Y88.  d8P'  `Y8b   `888'   `Y8b
//  888   .d88' 888            888     888
//  888ooo88P'  888            888oooo888'
//  888`88b.    888     ooooo  888    `88b
//  888  `88b.  `88.    .88'   888    .88P
// o888o  o888o  `Y8bood8P'   o888bood8P'
//
//
// - Write RGB data
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void write_pnm_RGB_data(std::ofstream &outfile,
                        NumericVector vec,
                        unsigned int ncol,
                        unsigned int nrow,
                        double scale_factor,
                        bool convert_to_row_major,
                        bool flipy) {

  unsigned int depth = 3;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set up buffer to write only BUFFER_ROWS rows a time
  // Reduces memory usage (by not allocating full size copy of the image)
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  unsigned int buffer_size = BUFFER_ROWS * ncol * depth;
  unsigned int remainder_size = (nrow % BUFFER_ROWS) * ncol * depth;
  unsigned char *uc0 = (unsigned char *) calloc(buffer_size, sizeof(unsigned char));
  if (!uc0) stop("write_pnm(): out of memory");
  unsigned char *uc = uc0;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Get a pointer to the actual data in the supplied matrix
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  double *v0 = vec.begin();


  if (convert_to_row_major) {
    // Convert from R's column-major ordering to row-major output order
    // Red, Green and Blue values are in different array planes, but
    // reordered to be written consecutively
    for (unsigned int row = 0; row < nrow; row++) {
      const unsigned int offset = flipy ? nrow - 1 - row : row;
      unsigned int r = offset;
      unsigned int g = offset + nrow * ncol;
      unsigned int b = offset + nrow * ncol * 2;
      for (unsigned int col = 0; col < ncol; col ++) {
        *uc++ = (unsigned char)(v0[r] * scale_factor);
        *uc++ = (unsigned char)(v0[g] * scale_factor);
        *uc++ = (unsigned char)(v0[b] * scale_factor);
        r += nrow;
        g += nrow;
        b += nrow;
      }

      // Flush the buffer to file
      if ((row + 1) % BUFFER_ROWS == 0) {
        outfile.write((char *)uc0, sizeof(unsigned char) * buffer_size);
        uc = uc0;
      }
    }
  } else {
    // Write pixels in R's column-major ordering
    for (unsigned int row = 0; row < nrow; row++) {
      const unsigned int offset = flipy ? nrow - 1 - row : row;
      double *r = v0 + ncol * offset;
      double *g = v0 + ncol * offset + nrow * ncol;
      double *b = v0 + ncol * offset + nrow * ncol * 2;
      for (unsigned int col = 0; col < ncol; col ++) {
        *uc++ = (unsigned char)(*r++ * scale_factor);
        *uc++ = (unsigned char)(*g++ * scale_factor);
        *uc++ = (unsigned char)(*b++ * scale_factor);
      }

      // Flush the buffer to file
      if ((row + 1) % BUFFER_ROWS == 0) {
        outfile.write((char *)uc0, sizeof(unsigned char) * buffer_size);
        uc = uc0;
      }
    }
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Flush any remaining values to file
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  outfile.write((char *)uc0, sizeof(unsigned char) * remainder_size);

  free(uc0);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//   .oooooo.
//  d8P'  `Y8b
// 888           oooo d8b  .ooooo.  oooo    ooo
// 888           `888""8P d88' `88b  `88.  .8'
// 888     ooooo  888     888ooo888   `88..8'
// `88.    .88'   888     888    .o    `888'
//  `Y8bood8P'   d888b    `Y8bod8P'     .8'
//                                  .o..P'
//                                  `Y8P'
//
//
// - Write GREY data
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void write_pnm_grey_data(std::ofstream &outfile,
                         NumericVector vec,
                         unsigned int ncol,
                         unsigned int nrow,
                         double scale_factor,
                         bool convert_to_row_major,
                         bool flipy) {

  unsigned int depth = 1;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set up buffer to write only BUFFER_ROWS rows a time
  // Reduces memory usage (by not allocating full size copy of the image)
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  unsigned int buffer_size = BUFFER_ROWS * ncol * depth;
  unsigned int remainder_size = (nrow % BUFFER_ROWS) * ncol * depth;
  unsigned char *uc0 = (unsigned char *) calloc(buffer_size, sizeof(unsigned char));
  if (!uc0) stop("write_pnm(): out of memory");
  unsigned char *uc = uc0;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Get a pointer to the actual data in the supplied vector
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  double *v0 = vec.begin();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Convert from R's column-major ordering to row-major output order
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (convert_to_row_major) {
    for (unsigned int row = 0; row < nrow; row++) {
      unsigned int j = flipy ? nrow - 1 - row : row;
      for (unsigned int col = 0; col < ncol; col++) {
        *uc++ = (unsigned char)(v0[j] * scale_factor);
        j += nrow;
      }

      // Flush the buffer to file
      if ((row + 1) % BUFFER_ROWS == 0) {
        outfile.write((char *)uc0, sizeof(unsigned char) * buffer_size);
        uc = uc0;
      }
    }
  } else {
    // Write pixels in R's column-major ordering
    for (unsigned int row = 0; row < nrow; row++) {
      unsigned int col = 0;
      const unsigned int offset = flipy ? nrow - 1 - row : row;
      double *v = v0 + ncol * offset;
      for (; col <= ncol - 8; col+=8) {
        *uc++ = (unsigned char)(*v++ * scale_factor);
        *uc++ = (unsigned char)(*v++ * scale_factor);
        *uc++ = (unsigned char)(*v++ * scale_factor);
        *uc++ = (unsigned char)(*v++ * scale_factor);

        *uc++ = (unsigned char)(*v++ * scale_factor);
        *uc++ = (unsigned char)(*v++ * scale_factor);
        *uc++ = (unsigned char)(*v++ * scale_factor);
        *uc++ = (unsigned char)(*v++ * scale_factor);
      }
      for (; col < ncol; col++) {
        *uc++ = (unsigned char)(*v++ * scale_factor);
      }


      // Flush the buffer to file
      if ((row + 1) % BUFFER_ROWS == 0) {
        outfile.write((char *)uc0, sizeof(unsigned char) * buffer_size);
        uc = uc0;
      }
    }
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Flush any remaining values to file
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  outfile.write((char *)uc0, sizeof(unsigned char) * remainder_size);

  free(uc0);
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//' Write a vector of numeric data to a PNM file
//'
//' @param vec numeric vector of data
//' @param dims integer vector of length 2 i.e. \code{c(nrow, ncol)} for a matrix/grey image and of
//'        length 3 i.e \code{c(nrow, ncol, 3)} for array/RGB output.
//' @param filename output filename e.g "example.pgm"
//' @param convert_to_row_major Convert to row-major order before output. R stores matrix
//'        and array data in column-major order. In order to output row-major order (as
//'        expected by most image formats) data ordering must be converted. If this argument
//'        is set to FALSE, then image output will be faster (due to fewer data-ordering operations, and
//'        better cache coherency) but the image will appear transposed. Default: TRUE
//' @param flipy By default, the position [0, 0] is considered the top-left corner of the output image.
//'        Set flipy = TRUE for [0, 0] to represent the bottom-left corner.  This operation
//'        is very fast and has negligible impact on overall write speed.
//'        Default: flipy = FALSE.
//' @param intensity_factor Multiplication factor applied to all values in image
//'        (note: no checking is performed to ensure values remain in range [0, 1]).
//'        If intensity_factor <= 0, then automatically determine (and apply) a multiplication factor
//'        to set the maximum value to 1.0. Default: intensity_factor = 1.0
//' @param pal integer matrix of size 256x3 with values in the range [0, 255]. Each
//'        row represents the r, g, b colour for a given grey index value. Only used
//'        if \code{vec} is a matrix
//'
//'
// [[Rcpp::export]]
void write_pnm_core(NumericVector vec,
                    IntegerVector dims,
                    std::string filename,
                    bool convert_to_row_major = true,
                    bool flipy = false,
                    double intensity_factor = 1,
                    Rcpp::Nullable<Rcpp::IntegerMatrix> pal = R_NilValue) {

  unsigned int nrow = dims[0];
  unsigned int ncol = dims[1];
  unsigned int depth = 1;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Check that the third dimensions is 3
  // ToDo: Tidy this.
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (dims.length() > 3 || (dims.length() == 3 && dims[2] != 3)) {
      stop("write_png(): If passing in an array, must have 3 planes");
  }
  if (dims.length() == 3) {
    depth = 3;
  }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If writing in column-major, swap 'nrow' and 'ncol'
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (!convert_to_row_major) {
    unsigned int tmp = nrow;
    nrow = ncol;
    ncol = tmp;
  }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Scale the intensity
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  double scale_factor = 255.0;
  if (intensity_factor <= 0) {
    double *max_value = std::max_element(vec.begin(), vec.end());
    if (*max_value == 0) {
      *max_value = 1;
    }
    scale_factor /= *max_value;
  } else {
    scale_factor *= intensity_factor;
  }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Check for palette
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  bool has_palette = pal.isNotNull();

  if (has_palette && depth != 1) {
    stop("Can't have a palette unless depth = 1");
  }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Open the output and write a PNM header
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  std::ofstream outfile;
  outfile.open(filename, std::ios::out | std::ios::binary);
  if (depth == 1 && !has_palette) {
    outfile << "P5" << std::endl << ncol << " " << nrow << std::endl << 255 << std::endl;
  } else {
    outfile << "P6" << std::endl << ncol << " " << nrow << std::endl << 255 << std::endl;
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Write the data appropriately
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (depth == 1 && !has_palette) {
    write_pnm_grey_data(outfile, vec, ncol, nrow, scale_factor, convert_to_row_major, flipy);
  } else if (depth == 1 && has_palette) {
    Rcpp::IntegerMatrix pal_(pal);
    write_pnm_grey_data_with_palette(outfile, vec, ncol, nrow, scale_factor, convert_to_row_major, flipy, pal_);
  } else {
    write_pnm_RGB_data (outfile, vec, ncol, nrow, scale_factor, convert_to_row_major, flipy);
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Close the output stream
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  outfile.close();
}



