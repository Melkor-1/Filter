#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif

#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif

#define _POSIX_C_SOURCE 200819L
#define _XOPEN_SOURCE   700

#include "hbmp.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define BMP_SCANLINE_PADDING 4
#define BF_UNPADDED_REGION_SIZE 12

static size_t determine_padding(size_t width)
{
    /* In BMP images, each scanline (a row of pixels) must be a multiple of
     * BMP_SCANLINE_PADDING bytes in size. If the width of the image in pixels 
     * multipled by the size of each pixel (in bytes) is not a multiple of 
     * BMP_SCANLINE_PADDING, padding is added to make it so. 
     */
    return (BMP_SCANLINE_PADDING -
            (width * sizeof (RGBTRIPLE)) % BMP_SCANLINE_PADDING) %
        BMP_SCANLINE_PADDING;
}

static int write_scanlines(FILE * out_file, size_t height, size_t width,
                           const RGBTRIPLE image[][width], size_t padding)
{
    const size_t pad_byte = 0x00;

    /* Write new pixels to outfile */
    for (size_t i = 0; i < height; ++i) {
        /* Write row to outfile, with padding at the end. */
        if (fwrite(image[i], sizeof image[i][0], width, out_file) != width
            || fwrite(&pad_byte, 1, padding, out_file) != padding) {
            return -1;
        }
    }

    return 0;
}

int write_image(const BITMAPFILEHEADER * restrict bf,
                const BITMAPINFOHEADER * restrict bi,
                FILE * restrict out_file, size_t height,
                size_t width, const RGBTRIPLE image[height][width])
{
    if (out_file != stdout && !(errno = 0, freopen(NULL, "wb", out_file))) {
        errno ? perror("freopen()") :
            (void) fputs("Error - failed to write to output file.\n", stderr);
        return -1;
    }

    if (fwrite(&bf->bf_type, sizeof bf->bf_type, 1, out_file) != 1
        || fwrite(&bf->bf_size, BF_UNPADDED_REGION_SIZE, 1, out_file) != 1
        || fwrite(bi, sizeof *bi, 1, out_file) != 1) {
        fputs("Error - failed to write to output file.\n", stderr);
        return -1;
    }

    const size_t padding = determine_padding(width);

    if (write_scanlines(out_file, height, width, image, padding) == -1) {
        fputs("Error - failed to write to output file.\n", stderr);
        return -1;
    }
    return out_file == stdout || !fclose(out_file);
}

static int read_scanlines(FILE * in_file, size_t height, size_t width,
                          RGBTRIPLE image[][width], size_t padding)
{
    /* Iterate over infile's scanlines */
    for (size_t i = 0; i < height; i++) {
        /* Read row into pixel array */
        if (fread(image[i], sizeof image[i][0], width, in_file) != width) {
            return -1;
        }

        /* Temporary buffer to read and discard padding. */
        uint8_t padding_buffer[BMP_SCANLINE_PADDING];

        if (fread(padding_buffer, 1, padding, in_file) != padding) {
            return -1;
        }
    }
    return 0;
}

void *read_image(BITMAPFILEHEADER * restrict bf,
                 BITMAPINFOHEADER * restrict bi,
                 size_t *restrict height_ptr,
                 size_t *restrict width_ptr, FILE * restrict in_file)
{
    /* Read infile's BITMAPFILEHEADER and BITMAPINFOHEADER. */
    if (fread(&bf->bf_type, sizeof bf->bf_type, 1, in_file) != 1
        || fread(&bf->bf_size, BF_UNPADDED_REGION_SIZE, 1, in_file) != 1
        || fread(bi, sizeof *bi, 1, in_file) != 1) {
        fputs("Error - failed to read input file.\n", stderr);
        return NULL;
    }

    /* Ensure infile is (likely) a 24-bit uncompressed BMP 4.0 */
    if (!bmp_check_header(bf, bi)) {
        fputs("Error - unsupported file format.\n", stderr);
        return NULL;
    }
#if 0
    /* If bi_height is positive, the bitmap is a bottom-up DIB with the origin 
     * at the lower left corner. It bi_height is negative, the bitmap is a top-
     * down DIB with the origin at the upper left corner. 
     * We currenly only support images stored as top-down, so bail if the format
     * is elsewise.
     */
    if (bi->bi_height > 0) {
        fputs("Error - Bottom-up BMP image format is not yet supported.\n",
              stderr);
        return NULL;
    }
#endif

    /* Get image's dimensions. */
    uint32_t abs_height = bi->bi_height < 0 ? 0u - (uint32_t) bi->bi_height :
        (uint32_t) bi->bi_height;

    /* If we are on a too small a machine, there is not much hope, so bail. */
    if (abs_height > SIZE_MAX) {
        fputs
            ("Error - Image dimensions are too large for this system to process.\n",
             stderr);
        return NULL;
    }

    size_t height = (size_t) abs_height;
    size_t width = (size_t) bi->bi_width;

    if (!height || !width) {
        fputs("Error - corrupted BMP file: width or height is zero.\n", stderr);
        return NULL;
    }

    if (width > (SIZE_MAX - sizeof (RGBTRIPLE)) / sizeof (RGBTRIPLE)) {
        fputs("Error - image width is too large for this system to process.\n",
              stderr);
        return NULL;
    }

    /* Allocate memory for image */
    RGBTRIPLE(*image)[width] = calloc(height, sizeof *image);

    if (!image) {
        fputs("Error - not enough memory to store image.\n", stderr);
        return NULL;
    }

    const size_t padding = determine_padding(width);

    if (read_scanlines(in_file, height, width, image, padding)) {
        fputs("Error - failed to read input file.\n", stderr);
        return NULL;
    }

    *height_ptr = height;
    *width_ptr = width;
    return image;
}
