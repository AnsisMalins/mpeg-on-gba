#include <tonc.h>
#include "mpeg.h"
#include "lunablep_bin.h"

void init_palette(COLOR* mem)
{
	// https://en.wikipedia.org/wiki/List_of_software_palettes#6_level_RGB
	for (int r = 0; r < 6; r++)
		for (int g = 0; g < 6; g++)
			for (int b = 0; b < 6; b++)
				mem[r * 36 + g * 6 + b] = RGB15(r * 31 / 5, g * 31 / 5, b * 31 / 5);
}

IWRAM_CODE void main_iwram(void)
{
	plm_t* plm = plm_create_with_memory((uint8_t*)lunablep_bin, lunablep_bin_size, FALSE);
	plm_set_audio_enabled(plm, FALSE);
	plm_set_loop(plm, TRUE);

	ptrdiff_t offset = (M4_HEIGHT / 2 - plm_get_height(plm) / 2) * M4_WIDTH
		+ (M4_WIDTH / 2 - plm_get_width(plm) / 2);

	while (!plm_has_ended(plm))
	{
		plm_frame_t* frame = plm_decode_video(plm);
		frame_to_indexed(frame, (uint8_t*)vid_page + offset, M4_WIDTH);
		VBlankIntrWait();
		vid_flip();
	}

	plm_destroy(plm);
}

int main(void)
{
	irq_init(NULL);
	irq_enable(II_VBLANK);

	// We want mode 4 (and BG2 on). Each pixel is then a 8 bit index into the
	// background palette located at 0x05000000.
	REG_DISPCNT = DCNT_MODE4 | DCNT_BG2;

	init_palette(pal_bg_mem);
	main_iwram();
	
	return 0;
}
