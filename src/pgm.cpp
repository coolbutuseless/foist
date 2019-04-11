#include <fstream>
#include "Rcpp.h"

using namespace Rcpp;

//' Write a numeric matrix to a PGM file
//'
//' @param mat numeric matrix
//' @param filename output filename e.g "example.pgm"
//' @param convert_to_row_major Convert to row-major order before output. R stores matrix
//'        and array data in column-major order. In order to output row-major order (as
//'        expected by PGM/PPM image format) data ordering must be converted. If this argument
//'        is set to FALSE, then image output will be faster (due to fewer data-ordering operations, and
//'        better cache coherency) but the image will be transposed. Default: TRUE
//' @param intensity_factor Multiplication factor applied to all values in image
//'        (note: no checking is performed to ensure values remain in range [0, 1]).
//'        If intensity_factor <= 0, then automatically determine (and apply) a multiplication factor
//'        to set the maximum value to 1.0. Default: intensity_factor = 1.0
//'
//'
// [[Rcpp::export]]
void write_pgm(NumericMatrix mat, std::string filename, bool convert_to_row_major = true, double intensity_factor = 1) {
  unsigned int nrow = mat.nrow(), ncol = mat.ncol();

  // Set up buffer to write only 10 rows a time
  // Reduces memory usage. May help with IO.
  // Negligible overall speed impact on my machine
  unsigned int buffer_size = 10 * ncol;
  unsigned char *uc = (unsigned char *) calloc(buffer_size, sizeof(unsigned char));
  if (!uc) stop("write_pgm(): out of memory");

  // Get a pointer to the actual data in the supplied matrix
  double *v = mat.begin();

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
  if (convert_to_row_major) {
    outfile << "P5" << std::endl << ncol << " " << nrow << std::endl << 255 << std::endl;
  } else {
    outfile << "P5" << std::endl << nrow << " " << ncol << std::endl << 255 << std::endl;
  }


  unsigned int out = 0;
  if (convert_to_row_major) {
    // Convert from R's column-major ordering to row-major output order
    for (unsigned int row = 0; row < nrow; row++) {
      unsigned int j = row;
      for (unsigned int col = 0; col < ncol; col++) {
        uc[out++] = (unsigned char)(v[j] * scale_factor);
        j += nrow;
      }

      // Flush the buffer to file
      if (out >= buffer_size) {
        outfile.write((char *)uc, sizeof(unsigned char) * out);
        out = 0;
      }
    }
  } else {
    // Write pixels in R's column-major ordering
    for (unsigned int row = 0; row < nrow; row++) {
      unsigned int col = 0;
      for (; col <= ncol - 8; col+=8) {
        uc[out++] = (unsigned char)(*v++ * scale_factor);
        uc[out++] = (unsigned char)(*v++ * scale_factor);
        uc[out++] = (unsigned char)(*v++ * scale_factor);
        uc[out++] = (unsigned char)(*v++ * scale_factor);
        uc[out++] = (unsigned char)(*v++ * scale_factor);
        uc[out++] = (unsigned char)(*v++ * scale_factor);
        uc[out++] = (unsigned char)(*v++ * scale_factor);
        uc[out++] = (unsigned char)(*v++ * scale_factor);
      }
      for (; col < ncol; col++) {
        uc[out++] = (unsigned char)(*v++ * scale_factor);
      }


      // Flush the buffer to file
      if (out >= buffer_size) {
        outfile.write((char *)uc, sizeof(unsigned char) * out);
        out = 0;
      }
    }
  }

  // Flush any remaining values to file
  if (out > 0) {
    outfile.write((char *)uc, sizeof(unsigned char) * out);
  }


  outfile.close();
  free(uc);
}
