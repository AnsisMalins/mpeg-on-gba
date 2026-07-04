#include <tonc.h>

#define PL_MPEG_IMPLEMENTATION
#include "pl_mpeg.h"

#include "lunablep_bin.h"

void init_palette(COLOR* mem)
{
	// https://en.wikipedia.org/wiki/List_of_software_palettes#6_level_RGB
	for (int r = 0; r < 6; r++)
		for (int g = 0; g < 6; g++)
			for (int b = 0; b < 6; b++)
				mem[r * 36 + g * 6 + b] = RGB15(r * 31 / 5, g * 31 / 5, b * 31 / 5);
}

static inline uint8_t rgb_to_indexed(int r, int g, int b)
{
	return r * 5 / 255 * 36 + g * 5 / 255 * 6 + b * 5 / 255;
}

IWRAM_CODE void frame_to_indexed(plm_frame_t* frame, uint8_t* dest, int stride)
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
            int cr = frame->cr.data[c_index] - 128;
            int cb = frame->cb.data[c_index] - 128;
            int r = (cr * 104597) >> 16;
            int g = (cb * 25674 + cr * 53278) >> 16;
            int b = (cb * 132201) >> 16;

			y = ((frame->y.data[y_index + 0] - 16) * 76309) >> 16;
			uint8_t pixel0 = rgb_to_indexed(plm_clamp(y + r), plm_clamp(y - g), plm_clamp(y + b));

			y = ((frame->y.data[y_index + 1] - 16) * 76309) >> 16;
			uint8_t pixel1 = rgb_to_indexed(plm_clamp(y + r), plm_clamp(y - g), plm_clamp(y + b));

			*(uint16_t*)(dest + d_index) = (pixel1 << 8) | pixel0;

			y = ((frame->y.data[y_index + yw] - 16) * 76309) >> 16;
			uint8_t pixel2 = rgb_to_indexed(plm_clamp(y + r), plm_clamp(y - g), plm_clamp(y + b));

			y = ((frame->y.data[y_index + yw + 1] - 16) * 76309) >> 16;
			uint8_t pixel3 = rgb_to_indexed(plm_clamp(y + r), plm_clamp(y - g), plm_clamp(y + b));

			*(uint16_t*)(dest + d_index + stride) = (pixel3 << 8) | pixel2;

			c_index += 1;
            y_index += 2;
            d_index += 2;
        }
    }
}

int main(void)
{
	irq_init(NULL);
	irq_enable(II_VBLANK);

	// We want mode 4 (and BG2 on). Each pixel is then a 8 bit index into the
	// background palette located at 0x05000000.
	REG_DISPCNT = DCNT_MODE4 | DCNT_BG2;

	init_palette(pal_bg_mem);

	plm_t* plm = plm_create_with_memory((uint8_t*)lunablep_bin, lunablep_bin_size, FALSE);
	plm_set_audio_enabled(plm, FALSE);
	plm_set_loop(plm, TRUE);

	while (!plm_has_ended(plm))
	{
		plm_frame_t* frame = plm_decode_video(plm);

		frame_to_indexed(frame, 
			(uint8_t*)(MEM_VRAM + (M4_HEIGHT / 2 - frame->height / 2) * M4_WIDTH + (M4_WIDTH / 2 - frame->width / 2)),
			M4_WIDTH);

		VBlankIntrWait();
	}

	plm_destroy(plm);
	return 0;
}
