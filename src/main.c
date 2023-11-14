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

#include "hbmp.h"

/* ARRAY_CARDINALITY(x) calculates the number of elements in the array 'x'.
 * If 'x' is a pointer, it will trigger an assertion.
 */
#define ARRAY_CARDINALITY(x) \
        (assert((void *)&(x) == (void *)(x)), sizeof (x) / sizeof *(x))

struct flags {
    bool sflag;                 /* Sepia flag. */
    bool rflag;                 /* Reverse flag. */
    bool gflag;                 /* Greyscale flag. */
    bool bflag;                 /* Blur flag. */
    FILE *out_file;             /* Output to file. */
};

static inline bool is_little_endian(void)
{
    int tmp = 1;

    return *(char *) &tmp;
}

static void help(void)
{
    puts("Usage: filter [OPTIONS] [FILE]\n"
         "\n\tTransform your BMP images with powerful filters.\n\n"
         "Options:\n"
         "    -s, --sepia           Apply a sepia filter for a warm, vintage look.\n"
         "    -r, --reverse         Create a horizontal reflection for a mirror effect.\n"
         "    -g, --grayscale       Convert the image to classic greyscale.\n"
         "    -b, --blur            Add a soft blur to the image.\n"
         "    -o, --output=FILE     Writes the output to the specified file.\n"
         "    -h, --help            displays this message and exit.\n");
    exit(EXIT_SUCCESS);
}

static void err_and_exit(void)
{
    fputs("Usage: filter [OPTIONS] <infile> <outfile>\n"
          "Try filter -h for help.\n", stderr);
    exit(EXIT_FAILURE);
}

static void parse_options(const struct option *restrict long_options,
                          const char *restrict short_options,
                          struct flags *restrict opt_ptr, int argc,
                          char *const argv[])
{
    int c;

    while ((c =
            getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
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
                /* We'll seek to the beginning once we've read input,
                 * in case it's the same file. 
                 */
                opt_ptr->out_file = (errno = 0, fopen(optarg, "ab"));

                if (!opt_ptr->out_file) {
                    errno ? perror(optarg) : (void)
                        fputs("Error - failed to open output file.", stderr);
                }
                break;

                /* case '?' */
            default:
                err_and_exit();
                break;
        }
    }
}

static void apply_filter(const struct flags *options, size_t height,
                         size_t width, RGBTRIPLE image[height][width])
{
    struct {
        bool flag;
        void (*const func)(size_t height, size_t width,
                           RGBTRIPLE image[height][width]);
    } group[] = {
        { options->sflag, sepia },
        { options->rflag, reflect },
        { options->gflag, grayscale },
        { options->bflag, blur },
    };

    for (size_t i = 0; i < ARRAY_CARDINALITY(group); ++i) {
        if (group[i].flag) {
            group[i].func(height, width, image);
        }
    }
}

static int process_image(const struct flags *restrict options,
                         FILE * restrict in_file, FILE * restrict out_file)
{
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;

    size_t height = 0;
    size_t width = 0;

    void *const image = read_image(&bf, &bi, &height, &width, in_file);

    if (!image) {
        return -1;
    }

    apply_filter(options, height, width, image);

    if (write_image(&bf, &bi, out_file, height, width, image) == -1) {
        return -1;
    }

    free(image);
    return 0;
}

int main(int argc, char *argv[])
{
    /* Sanity check. POSIX requires the invoking process to pass a non-NULL 
     * argv[0].
     */
    if (!argv[0]) {
        fputs("A NULL argv[0] was passed through an exec system call.\n",
              stderr);
        return EXIT_FAILURE;
    }

    if (!is_little_endian()) {
        fputs("Error - platform is not little-endian.\n", stderr);
        return EXIT_FAILURE;
    }

    /* Define allowable filters */
    static const struct option long_options[] = {
        { "grayscale", no_argument, NULL, 'g' },
        { "reverse", no_argument, NULL, 'r' },
        { "sepia", no_argument, NULL, 's' },
        { "blur", no_argument, NULL, 'b' },
        { "help", no_argument, NULL, 'h' },
        { "output", required_argument, NULL, 'o' },
        { NULL, 0, NULL, 0 }
    };

    FILE *in_file = stdin;
    struct flags options = { false, false, false, false, stdout };
    int result = EXIT_SUCCESS;

    parse_options(long_options, "grsbho:", &options, argc, argv);

    if ((optind + 1) == argc) {
        in_file = (errno = 0, fopen(argv[optind], "rb"));

        if (!in_file) {
            errno ? perror(argv[optind]) : (void)
                fputs("Error - failed to open input file.", stderr);
            return EXIT_FAILURE;
        }
    } else if (optind > argc) {
        err_and_exit();
    }

    if (process_image(&options, in_file, options.out_file) == -1) {
        result = EXIT_FAILURE;
    }

    if (in_file != stdin) {
        fclose(in_file);
    }

    return result;
}
