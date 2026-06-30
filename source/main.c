#include <tonc.h>

#define PL_MPEG_IMPLEMENTATION
#include "pl_mpeg.h"

#include "gross_bin.h"

int main(void)
{
	irq_init(NULL);
	irq_enable(II_VBLANK);

	// We want mode 4 (and BG2 on). Each pixel is then a 8 bit index into the
	// background palette located at 0x05000000.
	//REG_DISPCNT = DCNT_MODE4 | DCNT_BG2;

	REG_DISPCNT = DCNT_MODE0 | DCNT_BG0 | DCNT_OBJ | DCNT_OBJ_1D;

	// Can't write single bytes. Must write 2 or 4 bytes at a time.

	plm_t* plm = plm_create_with_memory((uint8_t*)gross_bin, gross_bin_size, FALSE);
	plm_set_audio_enabled(plm, FALSE);

	plm_frame_t *frame = NULL;

	frame = plm_decode_video(plm);

	plm_destroy(plm);

	tte_init_chr4c_default(0, BG_CBB(0) | BG_SBB(31));
	tte_set_pos(92, 68);
	tte_write("Hello World!");

	while (1) {
		VBlankIntrWait();
	}
}
