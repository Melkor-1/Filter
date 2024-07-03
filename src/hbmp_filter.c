#include "hbmp.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define NEIGHBORHOOD_SIZE   9
#define SCALE               8192
#define SCALE_UP(x)         ((uint_fast32_t) ((x) * SCALE + 0.5))
#define BLUR_TIMES          3

#define MIN(x, y)   ((x) < (y) ? (x) : (y))

void grayscale(size_t height, size_t width, RGBTRIPLE image[height][width])
{
    for (size_t i = 0; i < height; ++i) {
        for (size_t j = 0; j < width; ++j) {
            unsigned average =
                (unsigned) (image[i][j].rgbt_blue + image[i][j].rgbt_red +
                image[i][j].rgbt_green);
            average = (average + (average & 1u) + 1u) / 3u;
            image[i][j].rgbt_red = image[i][j].rgbt_green =
                image[i][j].rgbt_blue = (uint8_t) average;
        }
    }
}

void sepia(size_t height, size_t width, RGBTRIPLE image[height][width])
{
    for (size_t i = 0; i < height; ++i) {
        for (size_t j = 0; j < width; ++j) {
            const unsigned long sepia_red =
                ((SCALE_UP(0.393) * image[i][j].rgbt_red +
                  SCALE_UP(0.769) * image[i][j].rgbt_green +
                  SCALE_UP(0.189) * image[i][j].rgbt_blue) + SCALE / 2) / SCALE;
            const unsigned long sepia_green =
                ((SCALE_UP(0.349) * image[i][j].rgbt_red +
                  SCALE_UP(0.686) * image[i][j].rgbt_green +
                  SCALE_UP(0.168) * image[i][j].rgbt_blue) + SCALE / 2) / SCALE;
            const unsigned long sepia_blue =
                ((SCALE_UP(0.272) * image[i][j].rgbt_red +
                  SCALE_UP(0.534) * image[i][j].rgbt_green +
                  SCALE_UP(0.131) * image[i][j].rgbt_blue) + SCALE / 2) / SCALE;
            image[i][j].rgbt_red = (uint8_t) MIN(255, sepia_red);
            image[i][j].rgbt_blue = (uint8_t) MIN(255, sepia_blue);
            image[i][j].rgbt_green = (uint8_t) MIN(255, sepia_green);
        }
    }
}

static inline void swap(RGBTRIPLE * restrict lhs, RGBTRIPLE * restrict rhs)
{
    RGBTRIPLE tmp = *lhs;

    *lhs = *rhs;
    *rhs = tmp;
}

void reflect(size_t height, size_t width, RGBTRIPLE image[height][width])
{
    for (size_t i = 0; i < height; ++i) {
        size_t start = 0;
        size_t end = width;

        while (start < end) {
            --end;
            swap(&image[i][start], &image[i][end]);
            ++start;
        }
    }
}

void box_blur(size_t height, size_t width, RGBTRIPLE image[height][width])
{
    RGBTRIPLE(*temp)[width + 2] = (errno = 0, calloc(height + 2, sizeof *temp));

    if (!temp) {
        errno ? perror("calloc()") : (void)
            fputs("Error - failed to allocate memory for the image.", stderr);
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < height; ++i) {
        for (size_t j = 0; j < width; ++j) {
            temp[i + 1][j + 1] = image[i][j];
        }
    }

    for (size_t i = 0; i < height + 2; ++i) {
        temp[i][0] = temp[i][1];        /* Copy left edge. */
        temp[i][width + 1] = temp[i][width];    /* Copy right edge. */
    }

    for (size_t j = 0; j < width + 2; ++j) {
        temp[0][j] = temp[1][j];        /* Copy top edge. */
        temp[height + 1][j] = temp[height][j];  /* Copy bottom edge. */
    }

    for (size_t i = 1; i < height + 1; ++i) {
        for (size_t j = 1; j < width + 1; ++j) {
            size_t blue = 0, red = 0, green = 0;

            for (size_t k = i - 1; k < i + 2; ++k) {
                for (size_t l = j - 1; l < j + 2; ++l) {
                    red += temp[k][l].rgbt_red;
                    green += temp[k][l].rgbt_green;
                    blue += temp[k][l].rgbt_blue;
                }
            }

            image[i - 1][j - 1].rgbt_red =
                (uint8_t) ((red + NEIGHBORHOOD_SIZE / 2) / NEIGHBORHOOD_SIZE);
            image[i - 1][j - 1].rgbt_blue =
                (uint8_t) ((blue + NEIGHBORHOOD_SIZE / 2) / NEIGHBORHOOD_SIZE);
            image[i - 1][j - 1].rgbt_green =
                (uint8_t) ((green + NEIGHBORHOOD_SIZE / 2) / NEIGHBORHOOD_SIZE);
        }
    }

    free(temp);
}

void blur(size_t height, size_t width, RGBTRIPLE image[height][width])
{
    /* We try to approximate a Gaussian blur. */
    for (size_t i = 0; i < BLUR_TIMES; ++i) {
        box_blur(height, width, image);
    }
}

#undef NEIGHBORHOOD_SIZE
#undef SCALE
#undef SCALE_UP
#undef BLUR_TIMES
