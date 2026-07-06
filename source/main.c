#include <tonc.h>
#include "mpeg.h"
#include "palette_bin.h"
#include "video_bin.h"
#include "ycbcr_to_indexed_bin.h"

// Convert a video like this:
// ffmpeg -i ~/Downloads/YP-1A-01x03.mkv -an -vf "framestep=2,setpts=0.5*PTS,fps=ntsc_film,scale=192:108:force_divisible_by=4:reset_sar=1" -vcodec mpeg1video -f mpeg data/video.bin

IWRAM_CODE void main_iwram(void)
{
	plm_t* plm = plm_create_with_memory((uint8_t*)video_bin, video_bin_size, FALSE);
	plm_set_audio_enabled(plm, FALSE);
	plm_set_loop(plm, TRUE);

	ptrdiff_t offset = (M4_HEIGHT / 2 - plm_get_height(plm) / 2) * M4_WIDTH
		+ (M4_WIDTH / 2 - plm_get_width(plm) / 2);

	while (!plm_has_ended(plm))
	{
		plm_frame_t* frame = plm_decode_video(plm);
		frame_to_indexed(frame, (uint8_t*)vid_page + offset, M4_WIDTH, ycbcr_to_indexed_bin);
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

	memcpy32(pal_bg_mem, palette_bin, 256 * sizeof(COLOR) / sizeof(int));

	main_iwram();

	return 0;
}
