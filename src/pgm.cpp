#include <fstream>
#include "Rcpp.h"

using namespace Rcpp;

#define BUFFER_ROWS 20

//' Write a numeric matrix to a PGM file
//'
//' @param mat numeric matrix
//' @param filename output filename e.g "example.pgm"
//' @param convert_to_row_major Convert to row-major order before output. R stores matrix
//'        and array data in column-major order. In order to output row-major order (as
//'        expected by PGM/PPM image format) data ordering must be converted. If this argument
//'        is set to FALSE, then image output will be faster (due to fewer data-ordering operations, and
//'        better cache coherency) but the image will be transposed. Default: TRUE
//' @param flipy By default, the position [0, 0] is considered the top-left corner of the output image. Set flipy = TRUE
//'        for [0, 0] to represent the bottom-left corner. Default: flipy = FALSE
//' @param intensity_factor Multiplication factor applied to all values in image
//'        (note: no checking is performed to ensure values remain in range [0, 1]).
//'        If intensity_factor <= 0, then automatically determine (and apply) a multiplication factor
//'        to set the maximum value to 1.0. Default: intensity_factor = 1.0
//'
//'
// [[Rcpp::export]]
void write_pgm(NumericMatrix mat, std::string filename, bool convert_to_row_major = true,
                bool flipy = false, double intensity_factor = 1) {
  unsigned int nrow = mat.nrow(), ncol = mat.ncol();

  // If writing in column-major, swap 'nrow' and 'ncol'
  if (!convert_to_row_major) {
    unsigned int tmp = nrow;
    nrow = ncol;
    ncol = tmp;
  }

  // Set up buffer to write only BUFFER_ROWS rows a time
  // Reduces memory usage (over allocating full size image)
  unsigned int buffer_size = BUFFER_ROWS * ncol;
  unsigned int remainder_size = (nrow % BUFFER_ROWS) * ncol;
  unsigned char *uc0 = (unsigned char *) calloc(buffer_size, sizeof(unsigned char));
  if (!uc0) stop("write_pgm(): out of memory");
  unsigned char *uc = uc0;

  // Get a pointer to the actual data in the supplied matrix
  double *v0 = mat.begin();

  // Scaling
  double scale_factor = 255.0;
  if (intensity_factor <= 0) {
    double *max_value = std::max_element(mat.begin(), mat.end());
    if (*max_value == 0) {
      *max_value = 1;
    }
    scale_factor /= *max_value;
  } else {
    scale_factor *= intensity_factor;
  }

  // Open the output and write a PGM header
  std::ofstream outfile;
  outfile.open(filename, std::ios::out | std::ios::binary);
  outfile << "P5" << std::endl << ncol << " " << nrow << std::endl << 255 << std::endl;


  // Convert from R's column-major ordering to row-major output order
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

  // Flush any remaining values to file
  outfile.write((char *)uc0, sizeof(unsigned char) * remainder_size);


  outfile.close();
  free(uc0);
}



