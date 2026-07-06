#include <tonc.h>

#define PL_MPEG_IMPLEMENTATION
#define PLM_NO_STDIO
#include "pl_mpeg.h"

void frame_to_indexed(plm_frame_t* frame, uint8_t* dest, int stride, const uint8_t* ycbcr_to_indexed)
{
    int cols = frame->width >> 1;
    int rows = frame->height >> 1;
    int yw = frame->y.width;
    int cw = frame->cb.width;
    
    for (int row = 0; row < rows; row++)
    {
        int c_index = row * cw;
        int y_index = row * 2 * yw;
        int d_index = row * 2 * stride;

        for (int col = 0; col < cols; col++)
        {
            int y;
            int cbcr = (frame->cb.data[c_index] & 0xf0) | (frame->cr.data[c_index] >> 4);

			y = frame->y.data[y_index] & 0xf0 << 4;
			uint8_t pixel0 = ycbcr_to_indexed[y | cbcr];

			y = frame->y.data[y_index + 1] & 0xf0 << 4;
			uint8_t pixel1 = ycbcr_to_indexed[y | cbcr];

			*(uint16_t*)(dest + d_index) = (pixel1 << 8) | pixel0;

			y = frame->y.data[y_index + yw] & 0xf0 << 4;
			uint8_t pixel2 = ycbcr_to_indexed[y | cbcr];

			y = frame->y.data[y_index + yw + 1] & 0xf0 << 4;
			uint8_t pixel3 = ycbcr_to_indexed[y | cbcr];

			*(uint16_t*)(dest + d_index + stride) = (pixel3 << 8) | pixel2;

			c_index += 1;
            y_index += 2;
            d_index += 2;
        }
    }
}
