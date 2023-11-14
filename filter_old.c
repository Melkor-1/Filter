#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif

#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif

#define _POSIX_C_SOURCE 200819L
#define _XOPEN_SOURCE   700

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <getopt.h>

#include "helpers.h"

/* TODO:      2) Support more than one filter.
 *            3) Support input and output redirection.
 *            4) Support BMP images stored as top-down.
 *            6) Fix the dangerous casts in the last filter in helpers.c.
 */

/* ARRAY_CARDINALITY(x) calculates the number of elements in the array 'x'.
 * It also includes a safety check to ensure 'x' is an array (not a pointer).
 * If 'x' is a pointer, it will trigger an assertion.
 */
#define ARRAY_CARDINALITY(x) \
        (assert((void *)&(x) == (void *)(x)), sizeof (x) / sizeof *(x))

struct flags {
    bool sflag;     /* Sepia flag. */
    bool rflag;     /* Reverse flag. */
    bool gflag;     /* Greyscale flag. */ 
    bool bflag;     /* Blur flag. */
    FILE *output;   /* Output to file. */
}

static void parse_options(const struct option *restrict long_options, const char *restrict short_options, struct flags *restrict opt_ptr, int argc, char *const argv[]) 
{
    int c;

    while ((c = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        switch (c) {
            case 's':
                opt_ptr->sflag = true;
                break;
            case 'r':
                opt_ptr->rflag = true;
                break;
            case 'g':
                opt_ptr->gflag = true;
                break;
            case 'b':
                opt_ptr->bflag = true;
                break;
            case 'h':
                help();
                break;
            case 'o':
                opt_ptr->output = (errno = 0, fopen(optarg, "a"));

                if (!opt_ptr->output) {
                    errno ? perror(optarg) : (void) fputs("Error - failed to open output file.", stderr);
                }
                break;

            /* case '?' */
            default:
                err_msg();
                break;
        }
    }
}
static void help(void)
{
    puts("Usage: filter [OPTIONS] <infile> <outfile>\n"
         "\n\tTransform your BMP images with powerful filters.\n\n"
         "Options:\n"
         "    -s, --sepia           Apply a sepia filter for a warm, vintage look.\n"
         "    -r, --reverse         Create a horizontal reflection for a mirror effect.\n"
         "    -g, --grayscale       Convert the image to classic greyscale.\n"
         "    -b, --blur            Add a soft blur to the image.\n"
         "    -h, --help            displays this message and exit.\n");
    exit(EXIT_SUCCESS);
}

static void apply_filter(char filter, size_t height, size_t width,
                         RGBTRIPLE image[height][width])
{
    struct filter_map {
        const char filter_char;
        void (*const func)(size_t x, size_t y, RGBTRIPLE image[x][y]);
    } const filter_map[] = {
        { 'b', blur },
        { 'g', grayscale },
        { 'r', reflect },
        { 's', sepia }
    };

    for (size_t i = 0; i < ARRAY_CARDINALITY(filter_map); ++i) {
        if (filter_map[i].filter_char == filter) {
            filter_map[i].func(height, width, image);
            return;
        }
    }
}

static size_t determine_padding(size_t width)
{
    /* In BMP images, each scanline (a row of pixels) must be a multiple of 4 
     * bytes in size. If the width of the image in pixels multipled by the size 
     * of each pixel (in bytes) is not a multiple of 4, padding is added to make 
     * it so. 
     */
    return (4 - (width * sizeof (RGBTRIPLE)) % 4) % 4;
}

static int write_scanlines(FILE * outptr, size_t height, size_t width,
                           const RGBTRIPLE image[][width], size_t padding)
{
    const size_t pad_byte = 0x00;

    /* Write new pixels to outfile */
    for (size_t i = 0; i < height; ++i) {
        /* Write row to outfile, with padding at the end. */
        if (fwrite(image[i], sizeof image[i][0], width, outptr) != width
            || fwrite(&pad_byte, 1, padding, outptr) != padding) {
            return -1;
        }
    }

    return 0;
}

static int write_image(const BITMAPFILEHEADER * restrict bf,
                       const BITMAPINFOHEADER * restrict bi,
                       FILE * restrict outptr, size_t height,
                       size_t width, const RGBTRIPLE image[height][width])
{
    if (fwrite(&bf->bf_type, sizeof bf->bf_type, 1, outptr) != 1
        || fwrite(&bf->bf_size, 12, 1, outptr) != 1
        || fwrite(bi, sizeof *bi, 1, outptr) != 1) {
        fputs("Error - failed to write to file.\n", stderr);
        return -1;
    }

    const size_t padding = determine_padding(width);

    if (write_scanlines(outptr, height, width, image, padding) == -1) {
        fputs("Error - failed to write to file.\n", stderr);
        return -1;
    }
    return 0;
}

static int read_scanlines(FILE * inptr, size_t height, size_t width,
                          RGBTRIPLE image[][width], size_t padding)
{
    /* Iterate over infile's scanlines */
    for (size_t i = 0; i < height; i++) {
        /* Read row into pixel array */
        if (fread(image[i], sizeof image[i][0], width, inptr) != width) {
            return -1;
        }

        /* Temporary buffer to read and discard padding. */
        uint8_t padding_buffer[4];

        if (fread(padding_buffer, 1, padding, inptr) != padding) {
            return -1;
        }
    }
    return 0;
}

static void *read_image(BITMAPFILEHEADER *restrict bf,
                        BITMAPINFOHEADER *restrict bi, 
                        size_t *restrict height_ptr,
                        size_t *restrict width_ptr, FILE * restrict inptr)
{
    /* Read infile's BITMAPFILEHEADER and BITMAPINFOHEADER. */
    if (fread(&bf->bf_type, 2, 1, inptr) != 1
        || fread(&bf->bf_size, 12, 1, inptr) != 1
        || fread(bi, sizeof *bi, 1, inptr) != 1) {
        fputs("Error - failed to read file.\n", stderr);
        return NULL;
    }

    /* Ensure infile is (likely) a 24-bit uncompressed BMP 4.0 */
    if (!bmp_check_header(bf, bi)) {
        fputs("Error - unsupported file format.\n", stderr);
        return NULL;
    }

    /* Get image's dimensions.
     * We currently only process BMP images stored as bottom-up, so we convert
     * the height to positive.
     */
    uint32_t abs_height = bi->bi_height < 0 ? 0u - (uint32_t) bi->bi_height :
        (uint32_t) bi->bi_height;
    
    /* If we are on a too small a machine, there is not much hope, so bail. */
    if (abs_height > SIZE_MAX) {
        fputs("Error - Image dimensions are too large for this system to process.\n", stderr);
        return NULL;
    }

    size_t height = (size_t) abs_height;
    size_t width = (size_t) bi->bi_width;
    
    if (!height || !width) {
        fputs("Error - corrupted BMP file: width or height is zero.\n", stderr);
        return NULL;
    }

    if (width > (SIZE_MAX - sizeof (RGBTRIPLE)) / sizeof (RGBTRIPLE)) {
        fputs("Error - image width is too large for this system to process.\n", stderr);
        return NULL;
    }

    /* Allocate memory for image */
    RGBTRIPLE (*image)[width] = calloc(height, sizeof *image);

    if (!image) {
        fputs("Error - not enough memory to store image.\n", stderr);
        return NULL;
    }

    const size_t padding = determine_padding(width);

    if (read_scanlines(inptr, height, width, image, padding)) {
        fputs("Error - failed to read file.\n", stderr);
        return NULL;
    }

    *height_ptr = height;
    *width_ptr = width;
    return image;
}

static int process_image(char filter, FILE * restrict inptr,
                         FILE * restrict outptr)
{
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;

    size_t height = 0;
    size_t width = 0;

    void *const image = read_image(&bf, &bi, &height, &width, inptr);

    if (!image) {
        return -1;
    }

    apply_filter(filter, height, width, image);

    if (write_image(&bf, &bi, outptr, height, width, image) == -1) {
        return -1;
    }

    free(image);
    return 0;
}

static char parse_args(int argc, char *argv[])
{
    /* Define allowable filters */
    static const struct option long_options[] = {
        { "grayscale", no_argument, NULL, 'g' },
        { "reverse", no_argument, NULL, 'r' },
        { "sepia", no_argument, NULL, 's' },
        { "blur", no_argument, NULL, 'b' },
        { "help", no_argument, NULL, 'h' },
        { "output", require_argument, NULL, 'o' },
        { NULL, 0, NULL, 0 }
    };

    /* Get filter flag and check validity */
    const int filter = getopt_long(argc, argv, "grsbho:", long_options, NULL);

    if (filter == '?') {
        fputs("Invalid filter.\n", stderr);
        return '\0';
    }

    if (filter == -1 || filter != 'h') {
        if (argc != optind + 2) {
            fputs("Usage: filter [OPTIONS] <infile> <outfile>\n"
                  "Try filter -h for help.\n", stderr);
            return '\0';
        }
        return (char) filter;
    }
    /* Ensure single flag */
    if (getopt_long(argc, argv, "grsbh", long_options, NULL) != -1) {
        fputs("Only one filter allowed.\n", stderr);
        return '\0';
    }

    if (filter == 'h' && optind == argc) {
        return (char) filter;
    }

    fputs("Usage: filter [OPTIONS] <infile> <outfile>\n"
          "Try filter -h for help.\n", stderr);
    return '\0';
}

int main(int argc, char *argv[])
{
    const char filter = parse_args(argc, argv);

    if (filter == '\0') {
        return EXIT_FAILURE;
    } else if (filter == 'h') {
        help();
    }

    const char *const infile = argv[optind];
    const char *const outfile = argv[optind + 1];
    FILE *const inptr = (errno = 0, fopen(infile, "rb"));

    /* The cast to void is required as ISO C forbids conditional expr with only 
     * one void side. 
     */
    if (!inptr) {
        errno ? perror("fopen()") : (void)
            fputs("Error - failed to open infile.\n", stderr);
        return EXIT_FAILURE;
    }

    FILE *const outptr = (errno = 0, fopen(outfile, "wb"));

    if (!outptr) {
        fclose(inptr);
        errno ? perror("fopen()") : (void)
            fputs("Error - failed to open outfile.\n", stderr);
        return EXIT_FAILURE;
    }

    if (process_image(filter, inptr, outptr) == -1) {
        fclose(inptr);
        fclose(outptr);
        return EXIT_FAILURE;
    }

    fclose(inptr);
    fclose(outptr);
    return EXIT_SUCCESS;
}
