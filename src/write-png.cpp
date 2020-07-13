#include <fstream>
#include "Rcpp.h"

using namespace Rcpp;

#include "crc32.h"
#include "adler32.h"


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Use an SSE version of ADLER32?
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// #define ADLER32_SSE

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Swap endianness for a 32bit unsigned int
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static inline uint32_t bswap32(uint32_t x) {
#if defined(__GNUC__) || defined(__clang__)
  return __builtin_bswap32(x);
#else
  return (x >> 24) |
        ((x >>  8) & 0x0000FF00) |
        ((x <<  8) & 0x00FF0000) |
         (x << 24);
#endif
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// ooooooooo.   ooooo      ooo   .oooooo.        .oooooo..o  o8o
// `888   `Y88. `888b.     `8'  d8P'  `Y8b      d8P'    `Y8  `"'
//  888   .d88'  8 `88b.    8  888              Y88bo.      oooo   .oooooooo
//  888ooo88P'   8   `88b.  8  888               `"Y8888o.  `888  888' `88b
//  888          8     `88b.8  888     ooooo         `"Y88b  888  888   888
//  888          8       `888  `88.    .88'     oo     .d8P  888  `88bod8P'
// o888o        o8o        `8   `Y8bood8P'      8""88888P'  o888o `8oooooo.
//                                                                d"     YD
//                                                                "Y88888P'
//
//  - Write PNG header - 8 bytes.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void write_PNG_signature(std::ofstream &outfile) {
  const unsigned char PNG_header[12] = {
    0x89, 0x50, 0x4e, 0x47,   // ".PNG"
    0x0d, 0x0a, 0x1a, 0x0a    // CRC32
  };

  outfile.write(reinterpret_cast<const char *>(&PNG_header[0]), 8);
  // outfile.write(&PNG_header[0], 8);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// ooooo ooooo   ooooo oooooooooo.   ooooooooo.
// `888' `888'   `888' `888'   `Y8b  `888   `Y88.
//  888   888     888   888      888  888   .d88'
//  888   888ooooo888   888      888  888ooo88P'
//  888   888     888   888      888  888`88b.
//  888   888     888   888     d88'  888  `88b.
// o888o o888o   o888o o888bood8P'   o888o  o888o
//
//
// - Write IHDR chunk
// - 17 bytes of goodness
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void write_IHDR(std::ofstream &outfile, unsigned int ncol, unsigned int nrow, unsigned int colour_type) {
  unsigned char IHDR[17] = {
    0x49, 0x48, 0x44, 0x52, // "IHDR"
    0, 0, 0, 0,  // width  - Overwrite later
    0, 0, 0, 0,  // height - Overwrite later
    8,           // bit-depth - 8 bits per channel
    0,           // colour-type. 0 = greyscale.  2 = RGB, 3 = Palette/indexed
    0,           // compression method. Set to 0
    0,           // filter method.      Set to 0
    0            // interlace method.   Set to 0
  };

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Colour type
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  IHDR[13] = colour_type; // 0 = greyscale.  2 = RGB, 3 = Palette/indexed

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Insert the width into the IHDR chunk
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  IHDR[ 4] = (ncol >> 24) & 0xFF;
  IHDR[ 5] = (ncol >> 16) & 0xFF;
  IHDR[ 6] = (ncol >>  8) & 0xFF;
  IHDR[ 7] = (ncol      ) & 0xFF;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Insert the height into the IHDR chunk
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  IHDR[ 8] = (nrow >> 24) & 0xFF;
  IHDR[ 9] = (nrow >> 16) & 0xFF;
  IHDR[10] = (nrow >>  8) & 0xFF;
  IHDR[11] = (nrow      ) & 0xFF;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Write IHDR data length to output
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  uint32_t data_length = 13;
  data_length = bswap32(data_length);
  outfile.write(reinterpret_cast<const char *>(&data_length), sizeof(data_length));

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Write IHDR to output
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  uint32_t crc32 = 0;
  outfile.write((const char *)&IHDR[0], 17);
  crc32 = crc32_16bytes(&IHDR[0], 17, crc32);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Write IHDR CRC32 to output
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  crc32 = bswap32(crc32);
  outfile.write(reinterpret_cast<const char *>(&crc32), sizeof(crc32));
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  ooooooooo.   ooooo        ooooooooooooo oooooooooooo
//  `888   `Y88. `888'        8'   888   `8 `888'     `8
//   888   .d88'  888              888       888
//   888ooo88P'   888              888       888oooo8
//   888          888              888       888    "
//   888          888       o      888       888       o
//  o888o        o888ooooood8     o888o     o888ooooood8
//
//
// - Write out a PLTE (palette) chunk
// - Reference: https://www.w3.org/TR/PNG/#11PLTE
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void write_PLTE(std::ofstream &outfile, Rcpp::IntegerMatrix pal) {

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Sanity check
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    if (pal.nrow() < 2 | pal.nrow() > 256 | pal.ncol() != 3) {
      stop("\'pal\' must be a N x 3 IntegerMatrix with values in the range [0,255]");
    }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // PLTE header
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    unsigned char PLTE[4] = {
      80, 76, 84, 69  // "PLTE"
    };

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Write PLTE header to output
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    unsigned int nrow = pal.nrow();
    uint32_t data_length = 3 * nrow;
    data_length = bswap32(data_length);
    outfile.write(reinterpret_cast<const char *>(&data_length), sizeof(data_length));

    uint32_t crc32 = 0;
    outfile.write((const char *)&PLTE[0], 4);
    crc32 = crc32_16bytes(&PLTE[0], 4, crc32);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Convert the palette data to unsigned char and write to output
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    unsigned char ucpal[3*256];
    unsigned char *pucpal = &ucpal[0];
    for (unsigned int i=0; i < nrow; i++) {
      *pucpal++ = (unsigned char)pal[i           ];
      *pucpal++ = (unsigned char)pal[i + nrow    ];
      *pucpal++ = (unsigned char)pal[i + nrow * 2];
    }
    outfile.write((const char *)&ucpal[0], 3*nrow);
    crc32 = crc32_16bytes(&ucpal[0], 3*nrow, crc32);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Write PLTE CRC32 to output
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    crc32 = bswap32(crc32);
    outfile.write(reinterpret_cast<const char *>(&crc32), sizeof(crc32));
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// ooooo oooooooooo.         .o.       ooooooooooooo
// `888' `888'   `Y8b       .888.      8'   888   `8
//  888   888      888     .8"888.          888
//  888   888      888    .8' `888.         888
//  888   888      888   .88ooo8888.        888
//  888   888     d88'  .8'     `888.       888
// o888o o888bood8P'   o88o     o8888o     o888o
//
//
// Write out an IDAT chunk
//
// Reference: https://stackoverflow.com/questions/9050260/what-does-a-zlib-header-look-like
// Only using uncompressed DEFLATE blocks, see:
//   - https://tools.ietf.org/html/rfc1950
//   = https://tools.ietf.org/html/rfc1951
//
// Maximum size of IDAT block is limited by the LEN part of the DEFALTE header
// header. Since it's only 2 bytes, max deflate size is then 2^16 (65535)
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void write_IDAT(std::ofstream &outfile, unsigned char *uc0, unsigned int nbytes,
                uint32_t &adler32,
                bool first_idat_chunk, bool final_idat_chunk) {

  uint32_t data_length   = 0;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // IDAT marker
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  unsigned char IDAT[4] = {0x49, 0x44, 0x41, 0x54}; // "IDAT" text

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // ZLIB header - https://tools.ietf.org/html/rfc1950
  // 2 bytes = CFM + FLG.
  // Setting to {78, 01} is largest window = 32k
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  unsigned char ZLIB_header[2] = {0x78, 0x01};

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // DEFLATE header - https://tools.ietf.org/html/rfc1951
  // 3.2.3. Details of block format
  // Each block of compressed data begins with 3 header bits
  //    containing the following data:
  //    5 Empty bits - BTYPE (2 bits) - BFINAL (1 bit)
  //  BTYPE = 00 for No compression
  //  BFINAL = set if and only if this is the last block of the data set.
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  unsigned char DEFLATE_header[5] = {
    0x00,        // Is this the last block? 1/0 = Y/N.
    0x00, 0x00,  // LEN  = Data size not exceeding 32k.
    0x00, 0x00,  // NLEN = 1s complement of above datasize
  };

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Only if this is the final_idat_chunk do we set the BFINAL bit
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (final_idat_chunk) {
    DEFLATE_header[0] = 1;
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Write the LEN and NLEN data into the zip block header. Little Endian
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  uint16_t LEN  = nbytes;
  uint16_t NLEN = ~LEN; // 1s complement

  DEFLATE_header[1] =  LEN       & 0xFF;
  DEFLATE_header[2] =  LEN >> 8  & 0xFF;
  DEFLATE_header[3] = NLEN       & 0xFF;
  DEFLATE_header[4] = NLEN >> 8  & 0xFF;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // IDAT prep
  //   If this is the first chunk, then we need to write the zlib header.
  //   For all subsequent chunks, we only write the DEFLATE header
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (first_idat_chunk) {
    data_length = 2 + 5 + LEN; // (zlib header) + (DEFLATE header) + LEN
  } else {
    data_length =     5 + LEN; //                 (DEFLATE header) + LEN
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Adler32 is calculated over the entire data, and written at the end
  // of the final DEFLATE block. it is 32 bits = 4 bytes.
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (final_idat_chunk) {
    data_length += 4;
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Write IDAT data length
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  data_length = bswap32(data_length);
  outfile.write(reinterpret_cast<const char *>(&data_length), sizeof(data_length));


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Write IDAT header
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  uint32_t crc32 = 0;
  outfile.write((const char *)&IDAT[0], 4);
  crc32 = crc32_16bytes(&IDAT[0], 4, crc32);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // if this is first IDAT, then write ZLIB header
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (first_idat_chunk) {
    outfile.write((const char *)&ZLIB_header[0], 2);
    crc32 = crc32_16bytes(&ZLIB_header[0], 2, crc32);
  }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // write DEFLATE header
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  outfile.write((const char *)&DEFLATE_header[0], 5);
  crc32 = crc32_16bytes(&DEFLATE_header[0], 5, crc32);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Write the data
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  outfile.write((const char *)&uc0[0], nbytes);
  crc32 = crc32_16bytes(&uc0[0], nbytes, crc32);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Update the ADLER32
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  adler32 = update_adler32(adler32, &uc0[0], nbytes);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Output ADLER32 - only if this is the last DEFLATE block
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (final_idat_chunk) {
    adler32 = bswap32(adler32);
    outfile.write(reinterpret_cast<const char *>(&adler32), sizeof(adler32));
    crc32 = crc32_16bytes(&adler32, 4, crc32);
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Output CRC32
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  crc32 = bswap32(crc32);
  outfile.write(reinterpret_cast<const char *>(&crc32), sizeof(crc32));
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// ooooo oooooooooooo ooooo      ooo oooooooooo.
// `888' `888'     `8 `888b.     `8' `888'   `Y8b
//  888   888          8 `88b.    8   888      888
//  888   888oooo8     8   `88b.  8   888      888
//  888   888    "     8     `88b.8   888      888
//  888   888       o  8       `888   888     d88'
// o888o o888ooooood8 o8o        `8  o888bood8P'
//
//
// - Write IEND chunk
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void write_IEND(std::ofstream &outfile) {
  unsigned char IEND[12] = {
    0x00, 0x00, 0x00, 0x00,  // Data length for IEND is always 0
    0x49, 0x45, 0x4E, 0x44,  // "IEND"
    0xAe, 0x42, 0x60, 0x82   // CRC32 for "IEND"
  };

  outfile.write((const char *)&IEND[0], 12);
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
void write_png_grey_data(std::ofstream &outfile,
                         const NumericVector vec,
                         const unsigned int ncol,
                         const unsigned int nrow,
                         const double scale_factor,
                         const double round_offset,
                         const bool convert_to_row_major,
                         const bool flipy) {

  const unsigned int depth = 1;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Calculate a number of rows that fit into an IDAT (with some leeway)
  // The data for each row actually has a zero-byte pre-pended to it.
  // Not sure why this is done!
  // Calculate the number of rows that would fit in a maximally sized deflate block.
  // Maximum size os 'LEN' in DEFLATE header is 2 bytes = 65535
  // Want to make the defalte blocks as large as possible so that
  //   - the number of IDATs is reduced
  //   - CRC32 calculations which operate on larger buffers can really get
  //     their money's worth e.g. splice-by-8 and splice-by-16
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if ((ncol * depth + 1) > 65535) {
    stop("Images wider than 65535/depth not currently handled.");
  }
  unsigned int nrow_buffer = 65535/(ncol * depth + 1);
  if (nrow_buffer > nrow) {
    nrow_buffer = nrow;
  }
  unsigned int buffer_size = nrow_buffer * (ncol * depth + 1);
  unsigned int remainder_size = (nrow % nrow_buffer) * (ncol * depth + 1);
  unsigned char *uc0 = (unsigned char *) calloc(buffer_size, sizeof(unsigned char));
  if (!uc0) stop("write_png_grey_data(): out of memory");
  unsigned char *uc = uc0;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Get a pointer to the actual data in the supplied matrix
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  double *v0 = (double *)vec.begin();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Initialise the ADLER32 checksum. This is the checksum across the
  // entirity of the raw data
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  uint32_t adler32 = 1;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // There are some things which are only done for the first IDAT chunk
  // output e.g. outputting the full DEFLATE header
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  bool first_idat  = true;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  //  Prepare a buffer of data. Either transposing it (be default) or
  // leaving it in 'column-major' form which writes the raw data in the same
  // it is stored in R
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (convert_to_row_major) {
    for (unsigned int row = 0; row < nrow; row++) {
      unsigned int j = flipy ? nrow - 1 - row : row;
      *uc++ = 0; // First byte of every row is set to zero? No idea why.
      for (unsigned int col = 0; col < ncol; col++) {
        *uc++ = (unsigned char)(v0[j] * scale_factor + round_offset);
        j += nrow;
      }

      // Flush the buffer to file
      if ((row + 1) % nrow_buffer == 0) {
        write_IDAT(outfile, uc0, buffer_size, adler32,
                   first_idat,          // first IDAT
                   (row + 1) == nrow);  // final IDAT
        first_idat = false;
        uc = uc0;
      }
    }
  } else {
    // Write pixels in R's column-major ordering
    for (unsigned int row = 0; row < nrow; row++) {
      unsigned int col = 0;
      *uc++ = 0; // First byte of every row is set to zero? No idea why.
      const unsigned int offset = flipy ? nrow - 1 - row : row;
      double *v = v0 + ncol * offset;
      for (; col <= ncol - 8; col+=8) {
        *uc++ = (unsigned char)(*v++ * scale_factor + round_offset);
        *uc++ = (unsigned char)(*v++ * scale_factor + round_offset);
        *uc++ = (unsigned char)(*v++ * scale_factor + round_offset);
        *uc++ = (unsigned char)(*v++ * scale_factor + round_offset);

        *uc++ = (unsigned char)(*v++ * scale_factor + round_offset);
        *uc++ = (unsigned char)(*v++ * scale_factor + round_offset);
        *uc++ = (unsigned char)(*v++ * scale_factor + round_offset);
        *uc++ = (unsigned char)(*v++ * scale_factor + round_offset);
      }
      for (; col < ncol; col++) {
        *uc++ = (unsigned char)(*v++ * scale_factor + round_offset);
      }

      // Flush the buffer to file
      if ((row + 1) % nrow_buffer == 0) {
        write_IDAT(outfile, uc0, buffer_size, adler32,
                   first_idat,          // first IDAT
                   (row + 1) == nrow);  // final IDAT
        first_idat = false;
        uc = uc0;
      }

    }
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Write final IDAT chunk ff there's any remaining data in the buffer
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (remainder_size > 0) {
    write_IDAT(outfile, uc0, remainder_size, adler32, first_idat, true);
  }
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
void write_png_RGB_data(std::ofstream &outfile,
                        const NumericVector vec,
                        const unsigned int ncol,
                        const unsigned int nrow,
                        const double scale_factor,
                        const double round_offset,
                        const bool convert_to_row_major,
                        const bool flipy) {

  const unsigned int depth = 3;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Calculate a number of rows that fit into an IDAT (with some leeway)
  // The data for each row actually has a zero-byte pre-pended to it.
  // Not sure why this is done!
  // Calculate the number of rows that would fit in a maximally sized deflate block.
  // Maximum size os 'LEN' in DEFLATE header is 2 bytes = 65535
  // Want to make the defalte blocks as large as possible so that
  //   - the number of IDATs is reduced
  //   - CRC32 calculations which operate on larger buffers can really get
  //     their money's worth e.g. splice-by-8 and splice-by-16
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if ((ncol * depth + 1) > 65535) {
    stop("Images wider than 65535/depth not currently handled.");
  }
  unsigned int nrow_buffer = 65535/(ncol * depth + 1);
  if (nrow_buffer > nrow) {
    nrow_buffer = nrow;
  }
  unsigned int buffer_size = nrow_buffer * (ncol * depth + 1);
  unsigned int remainder_size = (nrow % nrow_buffer) * (ncol * depth + 1);
  unsigned char *uc0 = (unsigned char *) calloc(buffer_size, sizeof(unsigned char));
  if (!uc0) stop("write_png_RGB_data(): out of memory");
  unsigned char *uc = uc0;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Get a pointer to the actual data in the supplied matrix
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  double *v0 = (double *)vec.begin();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Initialise the ADLER32 checksum. This is the checksum across the
  // entirity of the raw data
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  uint32_t adler32 = 1;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // There are some things which are only done for the first IDAT chunk
  // output e.g. outputting the full DEFLATE header
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  bool first_idat  = true;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  //  Prepare a buffer of data. Either transposing it (be default) or
  // leaving it in 'column-major' form which writes the raw data in the same
  // it is stored in R
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (convert_to_row_major) {
    // Convert from R's column-major ordering to row-major output order
    // Red, Green and Blue values are in different array planes, but
    // reordered to be written consecutively
    for (unsigned int row = 0; row < nrow; row++) {
      const unsigned int offset = flipy ? nrow - 1 - row : row;
      unsigned int r = offset;
      unsigned int g = offset + nrow * ncol;
      unsigned int b = offset + nrow * ncol * 2;
      *uc++ = 0;
      for (unsigned int col = 0; col < ncol; col ++) {
        *uc++ = (unsigned char)(v0[r] * scale_factor + round_offset);
        *uc++ = (unsigned char)(v0[g] * scale_factor + round_offset);
        *uc++ = (unsigned char)(v0[b] * scale_factor + round_offset);
        r += nrow;
        g += nrow;
        b += nrow;
      }

      // Flush the buffer to file
      if ((row + 1) % nrow_buffer == 0) {
        write_IDAT(outfile, uc0, buffer_size, adler32,
                   first_idat,          // first IDAT
                   (row + 1) == nrow);  // final IDAT
        first_idat = false;
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
      *uc++ = 0;
      for (unsigned int col = 0; col < ncol; col ++) {
        *uc++ = (unsigned char)(*r++ * scale_factor + round_offset);
        *uc++ = (unsigned char)(*g++ * scale_factor + round_offset);
        *uc++ = (unsigned char)(*b++ * scale_factor + round_offset);
      }

      // Flush the buffer to file
      if ((row + 1) % nrow_buffer == 0) {
        write_IDAT(outfile, uc0, buffer_size, adler32,
                   first_idat,          // first IDAT
                   (row + 1) == nrow);  // final IDAT
        first_idat = false;
        uc = uc0;
      }
    }
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Write final IDAT chunk ff there's any remaining data in the buffer
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (remainder_size > 0) {
    write_IDAT(outfile, uc0, remainder_size, adler32, first_idat, true);
  }

  free(uc0);
}




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//' Write a numeric matrix or array to a PNG file
//'
//' Write a numeric matrix or array to a PNG file
//'
//'
//' Write a numeric matrix or array to a PNG file as fast as possible - meaning
//' that corners are cut to make it happen quickly:
//'
//' \itemize{
//' \item{Data is not compressed.}
//' \item{Matrix or array must be of type \code{numeric}}
//' }
//'
//' Design decisions
//'
//' \itemize{
//' \item{no PNG pixel filtering}
//' \item{no compression}
//' \item{each IDAT contains one-and-only-one deflate block.  This is purely for
//'    my convenience. Most other PNG writers have the IDAT and DEFLATE blocks
//'    update independently i.e. usually 1 DEFLATE block would span multiple IDATs.
//'    By having a one-to-one correspondence between DEFLATE blocks and IDAT
//'    chunks, the complexity of the code is greatly reduced}
//' \item{All DEFLATE windows are hard-coded to the maximum size of 32kb. Varying
//'    the specified window size might be useful on embedded systems with little
//'    memory, but not for this use case.}
//' }
//'
//' @param vec numeric 2d matrix or 3d array (with 3 planes)
//' @param dims integer vector of length 2 i.e. \code{c(nrow, ncol)} for a matrix/grey image and of
//'        length 3 i.e \code{c(nrow, ncol, 3)} for array/RGB output.
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
//' @param pal integer matrix of size 256x3 with values in the range [0, 255]. Each
//'        row represents the r, g, b colour for a given grey index value. Only used
//'        if \code{data} is a matrix
//'
//'
//'
// [[Rcpp::export]]
void write_png_core(const NumericVector vec,
                    const IntegerVector dims,
                    const std::string filename,
                    const bool convert_to_row_major = true,
                    const bool flipy                = false,
                    const bool invert               = false,
                    const double intensity_factor   = 1,
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
  // Open stream
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  std::ofstream outfile;
  outfile.open(filename, std::ios::out | std::ios::binary);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Write PNG signature
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  write_PNG_signature(outfile);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Write the IHDR chunk
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  unsigned int colour_type = 0; // Grey by default
  bool has_palette = pal.isNotNull();
  if (depth == 3) {
    colour_type = 2; // RGB
  }
  if (has_palette) {
    if (depth != 1) {
      stop("Can't have a palette unless depth = 1");
    }
    colour_type = 3; // Indexed Palette PNG
  }
  write_IHDR(outfile, ncol, nrow, colour_type);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Default scaling is to [0, 255]
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  double scale_factor = 255.0;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If a palette given, then write out a PLTE chunk.
  // Also set the scale factor dependent upon the number of palette colours
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (has_palette) {
    Rcpp::IntegerMatrix pal_(pal);
    write_PLTE(outfile, pal_);
    scale_factor = pal_.nrow() - 1;
  }

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


  if (depth == 1) {
    write_png_grey_data(outfile, vec, ncol, nrow, scale_factor, round_offset, convert_to_row_major, flipy);
  } else {
    write_png_RGB_data (outfile, vec, ncol, nrow, scale_factor, round_offset, convert_to_row_major, flipy);
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // IEND
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  write_IEND(outfile);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Close stream
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  outfile.close();

}




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Comparing the CRC32 implementations
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// void test_crc32(size_t N = 1e6) {
//   unsigned char *uc;
//   uc = (unsigned char *)calloc(N, sizeof(unsigned char));
//   uint32_t crc32 = 0;
//
//   for (unsigned int i = 0; i < N; i++) {
//     uc[i] = (unsigned char)i;
//   }
//
//   // std::cout << __BYTE_ORDER << std::endl;
//
//
//   crc32 = 0; std::cout << "naive   " <<  update_crc32  (uc, N, crc32) << std::endl;
//   crc32 = 0; std::cout << "fast  1 " <<   crc32_1byte  (uc, N, crc32) << std::endl;
//   crc32 = 0; std::cout << "fast  4 " <<   crc32_4bytes (uc, N, crc32) << std::endl;
//   crc32 = 0; std::cout << "fast  8 " <<   crc32_8bytes (uc, N, crc32) << std::endl;
//   crc32 = 0; std::cout << "fast 16 " <<   crc32_16bytes(uc, N, crc32) << std::endl;
// }

