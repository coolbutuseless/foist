#include <fstream>
#include "Rcpp.h"

using namespace Rcpp;


#define BUFFER_ROWS 20

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Swap endianness for a 32bit unsigned int
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// static inline uint32_t bswap32(uint32_t x) {
// #if defined(__GNUC__) || defined(__clang__)
//   return __builtin_bswap32(x);
// #else
//   return (x >> 24) |
//         ((x >>  8) & 0x0000FF00) |
//         ((x <<  8) & 0x00FF0000) |
//          (x << 24);
// #endif
// }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//   .oooooo.    ooooo oooooooooooo  .ooooo.    .ooooo.
//  d8P'  `Y8b   `888' `888'     `8 d88'   `8. 888' `Y88.
// 888            888   888         Y88..  .8' 888    888  .oooo.
// 888            888   888oooo8     `88888b.   `Vbood888 `P  )88b
// 888     ooooo  888   888    "    .8'  ``88b       888'  .oP"888
// `88.    .88'   888   888         `8.   .88P     .88P'  d8(  888
//  `Y8bood8P'   o888o o888o         `boood8'    .oP'     `Y888""8o
//
//
//  - Write GIF header
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void write_gif_header(std::ofstream &outfile, const unsigned int ncol, const unsigned int nrow) {
  char GIF_header[10] = {
    0x47, 0x49, 0x46,  // "GIF89a
    0x38, 0x39, 0x61,
    0x00, 0x00,        // Width
    0x00, 0x00         // Height
  };

  GIF_header[6] = ncol      & 0xFF;
  GIF_header[7] = ncol >> 8 & 0xFF;
  GIF_header[8] = nrow      & 0xFF;
  GIF_header[9] = nrow >> 8 & 0xFF;

  outfile.write(&GIF_header[0], 10);
}



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
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void write_global_colour_table(std::ofstream &outfile, Rcpp::IntegerMatrix pal) {


    const unsigned int nrow = pal.nrow();

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Sanity check. Support both 128 colour palettes and 256 colour palettes.
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    if ((nrow != 128 & nrow != 256) | (pal.ncol() != 3)) {
      stop("\'pal\' must be a 128x3  or 256x3 IntegerMatrix with values in the range [0,255]");
    }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // In the case of 256 colour palettes, just skip over every second row
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    unsigned int pal_mult = nrow == 128 ? 1 : 2;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // PLTE header
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    unsigned char GCT_header[3] = {
      0xF6, // Highest bit set = Global colour table present.  "6" = bits/colour - 1
      0x00, // Background colour
      0x00  // Default pixel aspect ratio
    };

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Write PLTE header to output
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    outfile.write((const char *)&GCT_header[0], 3);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Convert the palette data to unsigned char and write to output
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    unsigned char ucpal[3 * 128];
    unsigned char *pucpal = &ucpal[0];
    for (unsigned int i=0; i < 128; i++) {
      *pucpal++ = (unsigned char)pal[i * pal_mult           ];
      *pucpal++ = (unsigned char)pal[i * pal_mult + nrow    ];
      *pucpal++ = (unsigned char)pal[i * pal_mult + nrow * 2];
    }
    outfile.write((const char *)&ucpal[0], 3 * 128);

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// oooooooooooo                   .o8
// `888'     `8                  "888
//  888         ooo. .oo.    .oooo888
//  888oooo8    `888P"Y88b  d88' `888
//  888    "     888   888  888   888
//  888       o  888   888  888   888
// o888ooooood8 o888o o888o `Y8bod88P"
//
//
// - Write IEND chunk
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void write_gif_terminator(std::ofstream &outfile) {
  unsigned char GIF_terminator[1] = { 0x3B };

  outfile.write((const char *)&GIF_terminator[0], 1);
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
void write_gif_data(std::ofstream &outfile,
                    const NumericVector vec,
                    const unsigned int ncol,
                    const unsigned int nrow,
                    const double scale_factor,
                    const double round_offset,
                    const bool convert_to_row_major,
                    const bool flipy) {



  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Image descriptor header
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  unsigned char image_descriptor[11] = {
    0x2c,
    0x00, 0x00, 0x00, 0x00,  // NW corner position of image
    0x00, 0x00, 0x00, 0x00,  // Image width and height in pixels
    0x00,                    // No local colour table
    0x07                     // Start of image - minimum code size = 7
  };


  image_descriptor[5] = ncol      & 0xFF;
  image_descriptor[6] = ncol >> 8 & 0xFF;
  image_descriptor[7] = nrow      & 0xFF;
  image_descriptor[8] = nrow >> 8 & 0xFF;

  outfile.write((char *)image_descriptor, sizeof(unsigned char) * 11);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set up buffer to write only BUFFER_ROWS rows a time
  // Reduces memory usage (by not allocating full size copy of the image)
  //
  // However, because we're writing an uncompressed GIF, need to do extra
  // work to write a CLEAR instruction no further apart than every 2^n-1 bytes.
  // Since our colour depth is n=7, every 126 bytes(at most) must be interupted
  // by a 'CLEAR' byte (value = 2^n = 2^7 = 128 = 0x80)
  //
  // See https://en.wikipedia.org/wiki/GIF section on 'Uncompressed GIF'
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  const unsigned int chunk_length        = 120;  // How many bytes can be output before a CLEAR code is needed? Max: 128-2
  const unsigned int full_chunks_per_row = ncol/chunk_length;
  const unsigned int leftover_bytes      = ncol % chunk_length;
  const unsigned int leftover_chunks     = leftover_bytes > 0 ? 1 : 0;
  const unsigned int total_chunks        = full_chunks_per_row + leftover_chunks;
  const unsigned int row_data_length     = ncol + total_chunks * 2; // 2 extra bytes per chunk

  // std::cout << "ncol:             " << ncol                << std::endl;
  // std::cout << "full chunks/row:  " << full_chunks_per_row << std::endl;
  // std::cout << "leftover bytes:   " << leftover_bytes      << std::endl;
  // std::cout << "leftover chunks:  " << leftover_chunks     << std::endl;
  // std::cout << "total chunks:     " << total_chunks        << std::endl;
  // std::cout << "raw data length:  " << row_data_length     << std::endl;


  const unsigned int buffer_size     =         BUFFER_ROWS  * row_data_length;
  const unsigned int remainder_size  = (nrow % BUFFER_ROWS) * row_data_length;
  unsigned char *uc0 = (unsigned char *) calloc(buffer_size, sizeof(unsigned char));
  if (!uc0) stop("write_pnm_grey_data(): out of memory");
  unsigned char *uc = uc0;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Get a pointer to the actual data in the supplied vector
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  double *v0 = (double *)vec.begin();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Convert from R's column-major ordering to row-major output order
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (convert_to_row_major) {
    for (unsigned int row = 0; row < nrow; row++) {
      unsigned int j = flipy ? nrow - 1 - row : row;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Write as many full chunks per row as possible
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      for (unsigned int chunk = 0; chunk < full_chunks_per_row; chunk++) {
        *uc++ = (unsigned char)(chunk_length + 1);
        *uc++ = 0x80;  // CLEAR
        for (unsigned int idx = 0; idx < chunk_length; idx++) {
          *uc++ = (unsigned char)(v0[j] * scale_factor + round_offset);
          j += nrow;
        }
      }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Write any leftover bytes in their own chunk. Inefficient, but
      // it's easier to think about each 'row' as a fully contained entity
      // (and not carry over chunks from one row to the next)
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      if (leftover_bytes > 0) {
        *uc++ = (unsigned char)(leftover_bytes + 1);
        *uc++ = 0x80;  // CLEAR
        for (unsigned int idx = 0; idx < leftover_bytes; idx++) {
          *uc++ = (unsigned char)(v0[j] * scale_factor + round_offset);
          j += nrow;
        }
      }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Flush the buffer to file
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      if ((row + 1) % BUFFER_ROWS == 0) {
        outfile.write((char *)uc0, sizeof(unsigned char) * buffer_size);
        uc = uc0;
      }
    }
  } else {
    for (unsigned int row = 0; row < nrow; row++) {
      const unsigned int offset = flipy ? nrow - 1 - row : row;
      double *v = v0 + ncol * offset;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Write as many full chunks per row as possible
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      for (unsigned int chunk = 0; chunk < full_chunks_per_row; chunk++) {
        *uc++ = (unsigned char)(chunk_length + 1);
        *uc++ = 0x80;  // CLEAR
        for (unsigned int idx = 0; idx < 15; idx++) { // chunk_length/8 = 15
          *uc++ = (unsigned char)(*v++ * scale_factor + round_offset);
          *uc++ = (unsigned char)(*v++ * scale_factor + round_offset);
          *uc++ = (unsigned char)(*v++ * scale_factor + round_offset);
          *uc++ = (unsigned char)(*v++ * scale_factor + round_offset);

          *uc++ = (unsigned char)(*v++ * scale_factor + round_offset);
          *uc++ = (unsigned char)(*v++ * scale_factor + round_offset);
          *uc++ = (unsigned char)(*v++ * scale_factor + round_offset);
          *uc++ = (unsigned char)(*v++ * scale_factor + round_offset);
        }
      }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Write any leftover bytes in their own chunk. Inefficient, but
      // it's easier to think about each 'row' as a fully contained entity
      // (and not carry over chunks from one row to the next)
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      if (leftover_bytes > 0) {
        *uc++ = (unsigned char)(leftover_bytes + 1);
        *uc++ = 0x80;  // CLEAR
        unsigned int idx = 0;
        for (; idx <= leftover_bytes - 8; idx += 8) {
          *uc++ = (unsigned char)(*v++ * scale_factor + round_offset);
          *uc++ = (unsigned char)(*v++ * scale_factor + round_offset);
          *uc++ = (unsigned char)(*v++ * scale_factor + round_offset);
          *uc++ = (unsigned char)(*v++ * scale_factor + round_offset);

          *uc++ = (unsigned char)(*v++ * scale_factor + round_offset);
          *uc++ = (unsigned char)(*v++ * scale_factor + round_offset);
          *uc++ = (unsigned char)(*v++ * scale_factor + round_offset);
          *uc++ = (unsigned char)(*v++ * scale_factor + round_offset);
        }

        for (; idx < leftover_bytes; idx++) {
          *uc++ = (unsigned char)(*v++ * scale_factor + round_offset);
        }
      }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Flush the buffer to file
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      if ((row + 1) % BUFFER_ROWS == 0) {
        outfile.write((char *)uc0, sizeof(unsigned char) * buffer_size);
        uc = uc0;
      }
    }
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Flush any remaining data to file
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  outfile.write((char *)uc0, sizeof(unsigned char) * remainder_size);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Write marker for End-of-Data
  //   0x01   - a block of length 1
  //   0x81   - STOP marker = 2^n + 1 = 2^7 + 1 = 129 = 0x81
  //   0x00   - end of image data
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  unsigned char image_end[3] = { 0x01, 0x81, 0x00 };
  outfile.write((char *)image_end, sizeof(unsigned char) * 3);

  free(uc0);
}




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//' Write a numeric matrix or array to a GIF file
//'
//' Write a numeric matrix or array to a GIF file
//'
//'
//' Write a numeric matrix to a GIF file as fast as I can - meaning
//' that corners are cut to make it happen quickly:
//'
//' \itemize{
//' \item{Data is not compressed.}
//' }
//'
//'
//' @param vec numeric 2d matrix
//' @param dims integer vector of length 2 i.e. \code{c(nrow, ncol)} for a matrix/grey image
//' @param filename output filename e.g. "example.ppm"
//' @param convert_to_row_major Convert to row-major order before output. R stores matrix
//'        and array data in column-major order. In order to output row-major order (as
//'        expected by PGM/PPM image format) data ordering must be converted. If this argument
//'        is set to FALSE, then image output will be faster (due to fewer data-ordering operations, and
//'        better cache coherency) but the image will be transposed. Default: TRUE
//' @param flipy By default, the position [0, 0] is considered the top-left corner of the output image.
//'        Set flipy = TRUE for [0, 0] to represent the bottom-left corner.  This operation
//'        is very fast and has negligible impact on overall write speed.
//'        Default: flipy = FALSE.
//' @param invert invert all the pixel brightness values - as if the image were
//'        converted into a negative. Dark areas become bright and bright areas become dark.
//'        Default: FALSE
//' @param intensity_factor Multiplication factor applied to all values in image
//'        (note: no checking is performed to ensure values remain in range [0, 1]).
//'        If intensity_factor <= 0, then automatically determine (and apply) a multiplication factor
//'        to set the maximum value to 1.0. Default: intensity_factor = 1.0
//' @param pal integer matrix of size 128x3 or 256x3 with values in the range [0, 255]. Each
//'        row represents the r, g, b colour for a given grey index value. This
//'        GIF writer only supports 128 colours, so a 256x3 palette is reduced
//'        to 128 colours by selecting every second colour. If supplied with a
//'        128-colour-palette then it is used as-is.
//'
//'
//'
// [[Rcpp::export]]
void write_gif_core(const NumericVector vec,
                    const IntegerVector dims,
                    const std::string filename,
                    const bool convert_to_row_major = true,
                    const bool flipy                = false,
                    const bool invert               = false,
                    const double intensity_factor   = 1,
                    Rcpp::IntegerMatrix pal = R_NilValue) {


  unsigned int nrow = dims[0];
  unsigned int ncol = dims[1];

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Check that the third dimensions is 3
  // ToDo: Tidy this.
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (dims.length() != 2) {
    stop("write_gif(): 'dims' must be length = 2");
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
  // Open stream
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  std::ofstream outfile;
  outfile.open(filename, std::ios::out | std::ios::binary);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Write PNG signature
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  write_gif_header(outfile, ncol, nrow);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Default scaling is to [0, 127]
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  double scale_factor = 127.0 - 3;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Write Palette
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  write_global_colour_table(outfile, pal);
  // scale_factor = pal_.nrow() - 1;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Scale the intensity
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (intensity_factor <= 0) {
    double *max_value = (double *)std::max_element(vec.begin(), vec.end());
    if (*max_value == 0) {
      *max_value = 1;
    }
    scale_factor /= *max_value;
  } else {
    scale_factor *= intensity_factor;
  }

  double round_offset = 0.5;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Invert the colours?
  // Rounding offset is -1.5 in order to take advantage of the way a
  // 'double' is truncated to an 'unsigned char'
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (invert) {
    round_offset = -1.5;
    scale_factor = -scale_factor;
  }


  write_gif_data(outfile, vec, ncol, nrow, scale_factor, round_offset, convert_to_row_major, flipy);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // IEND
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  write_gif_terminator(outfile);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Close stream
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  outfile.close();

}


