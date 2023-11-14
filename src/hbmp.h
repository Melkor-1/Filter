#ifndef HBMP_H
#define HBMP_H 1

/**
 * @file hbmp.h
 * @brief BMP-related data types based on Microsoft's own, and image processing
 *        functions.
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/** 
 * @struct BITMAPFILEHEADER
 * @brief The BITMAPFILEHEADER structure contains information about the type, size,
 *        and layout of a file that contains a DIB [device-independent bitmap].
 *        Adapted from http://msdn.microsoft.com/en-us/library/dd183374(VS.85).aspx.
 */
typedef struct {
    uint16_t bf_type;       /**< The file type; should be 'BM' for BMP. */
    uint32_t bf_size;       /**< The size of the BMP file in bytes. */
    uint16_t bf_reserved1;  /**< Reserved; must be set to 0. */
    uint16_t bf_reserved2;  /**< Reserved; must be set to 0. */
    uint32_t bf_offbits;    /**< The offset, in bytes, from the beginning of the
                                 file to the bitmap data. */
} BITMAPFILEHEADER;

/**
 * @struct BITMAPINFOHEADER
 * @brief  The BITMAPINFOHEADER structure contains information about the
 *         dimensions and color format of a DIB [device-independent bitmap].
 *         Adapted from http://msdn.microsoft.com/en-us/library/dd183376(VS.85).aspx.
 */
typedef struct {
    uint32_t bi_size;               /**< The size of this header, in bytes. */
    int32_t bi_width;               /**< The width of the image in pixels. */
    int32_t bi_height;              /**< The height of the image in pixels. */
    uint16_t bi_planes;             /**< The number of color planes; must be 1/ */
    uint16_t bi_bitcount;           /**< The number of bits per pixel. */
    uint32_t bi_compression;        /**< The type of compression used; 0 for no compression. */
    uint32_t bi_size_image;         /**< The size of the image in bytes. */
    int32_t bi_x_resolution_ppm;    /**< The horizontal resolution, in pixels per meter. */
    int32_t bi_y_resolution_ppm;    /**< The vertical resolution, in pixels per meter. */
    uint32_t bi_clr_used;           /**< The number of colors used in the image. */
    uint32_t bi_clr_important;      /**< The numebr of important colors; 0 for all colors. */
} BITMAPINFOHEADER;

/**
 * @struct RGBTRIPLE
 * @brief  The RGBTRIPLE structure describes a color consisting of relative intensities of
 *         red, green, and blue. Adapted from http://msdn.microsoft.com/en-us/library/aa922590.aspx.
 */
typedef struct {
    uint8_t rgbt_blue;
    uint8_t rgbt_green;
    uint8_t rgbt_red;
} RGBTRIPLE;

/**
 * @brief Checks if the BMP file header and info header are compatible with the
 *        supported BMP file format.
 *
 * @param bf The BMP file header.
 * @param bi The BMP info header.
 * @return true if the headers are compatible, false otherwise.
 */
bool bmp_check_header(const BITMAPFILEHEADER * restrict bf,
                      const BITMAPINFOHEADER * restrict bi);

/**
 * @brief Writes an image to a BMP file.
 *
 * This function writes the provided BMP file header, info header, and image data to a BMP file.
 *
 * @param bf The BMP file header.
 * @param bi The BMP info header.
 * @param out_file The output file stream.
 * @param height The height of the image.
 * @param width The width of the image.
 * @param image The 2D array representing the image pixels.
 * @return 0 on success, -1 on failure.
 */
int write_image(const BITMAPFILEHEADER * restrict bf,
                const BITMAPINFOHEADER * restrict bi,
                FILE * restrict out_file, size_t height,
                size_t width, const RGBTRIPLE image[height][width]);

/**
 * @brief Read an image from a BMP file.
 *
 * This function reads an image from the specified input file stream,
 * allocating memory for the image and populating the height and width.
 *
 * @param bf A pointer to the BITMAPFILEHEADER structure.
 * @param bi A pointer to the BITMAPINFOHEADER structure.
 * @param height_ptr A pointer to store the height of the read image.
 * @param width_ptr A pointer to store the width of the read image.
 * @param in_file The input file stream.
 * @return A pointer to the allocated image data on success, NULL on failure.
 *
 */
void *read_image(BITMAPFILEHEADER * restrict bf,
                 BITMAPINFOHEADER * restrict bi,
                 size_t *restrict height_ptr,
                 size_t *restrict width_ptr, FILE * restrict in_file);
/**
 * @brief Convert an image to grayscale.
 * 
 * This function converts the color image to grayscale by averaging the red, green,
 * and blue values of each pixel.
 *
 * @param height The height of the image.
 * @param width The width of the image.
 * @param image The 2D array representing the image.
 *
 */
void grayscale(size_t height, size_t width, RGBTRIPLE image[height][width]);

/**
 * @brief Convert an image to sepia tone.
 * 
 * This function gives the image a sepia tone effect by adjusting the color values
 * of each pixel. Sepia tone is a reddish-brown monochrome tint often associated with
 * vintage and aged photographs. 
 *
 * @param height The height of the image.
 * @param width The width of the image.
 * @param image The 2D array representing the image.
 */
void sepia(size_t height, size_t width, RGBTRIPLE image[height][width]);

/**
 * @brief Reflect an image horizontally.
 *
* This function reflects the image horizontally by swapping the pixels between the
 * left and right sides of each row.
 *
 * @param height The height of the image.
 * @param width The width of the image.
 * @param image The 2D array representing the image.
 *
 */
void reflect(size_t height, size_t width, RGBTRIPLE image[height][width]);

/**
 * @brief Apply a blur filter to an image.
 *
 * This function applies a blur filter to the provided image. The blur filter is
 * a simple averaging filter that calculates the average color value of the pixels
 * in a neighborhood around each pixel. 
 *
 * @param height The height of the image.
 * @param width The width of the image.
 * @param image The 2D array representing the image.
 */
void blur(size_t height, size_t width, RGBTRIPLE image[height][width]);

#endif                          /* HBMP_H */
