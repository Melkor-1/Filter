#include "hbmp.h"

#define SUPPORTED_BF_TYPE           0x4d42
#define SUPPORTED_BF_OFF_BITS       54
#define SUPPORTED_BI_SIZE           40
#define SUPPORTED_BI_BIT_COUNT      24
#define SUPPORTED_BI_COMPRESSION    0

bool bmp_check_header(const BITMAPFILEHEADER * restrict bf,
                      const BITMAPINFOHEADER * restrict bi)
{
    return bf->bf_type == SUPPORTED_BF_TYPE
        && bf->bf_offbits == SUPPORTED_BF_OFF_BITS
        && bi->bi_size == SUPPORTED_BI_SIZE
        && bi->bi_bitcount == SUPPORTED_BI_BIT_COUNT
        && bi->bi_compression == SUPPORTED_BI_COMPRESSION;
}
