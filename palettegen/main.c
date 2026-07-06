#include <stdint.h>
#include <stdio.h>

// gcc -o palettegen main.c

static inline int clamp(int value)
{
    if (value < 0) return 0;
    else if (value > 255) return 255;
    else return value;
}

int main(void)
{
    uint16_t palette[256];

    for (int b = 0; b < 4; b++)
		for (int g = 0; g < 8; g++)
			for (int r = 0; r < 8; r++)
				palette[(b << 5) | (g << 2) | r]
                    = (b * 31 / 5 << 10) | (g * 31 / 5 << 5) | (r * 31 / 5);

    FILE* fpalette = fopen("data/palette.bin", "wb");
    fwrite(palette, sizeof(palette), 1, fpalette);
    fclose(fpalette);

    uint8_t ycbcr_to_indexed[16 * 16 * 16];

    for (int iy = 0; iy < 16; iy++)
    {
        int y = (((iy << 4) - 16) * 76309) >> 16;

        for (int icb = 0; icb < 16; icb++)
        {
            int cb = (icb << 4) - 128;

            for (int icr = 0; icr < 16; icr++)
            {
                int cr = (icr << 4) - 128;

                int r = (cr * 104597) >> 16;
                int g = (cb * 25674 + cr * 53278) >> 16;
                int b = (cb * 132201) >> 16;

                ycbcr_to_indexed[(iy << 8) | (icb << 4) | icr]
                    = ((clamp(y + b) >> 6) << 5) | clamp(y - g) >> 5 << 2 | clamp(y + r) >> 5;
            }
        }
    }

    FILE *fycbcr = fopen("data/ycbcr_to_indexed.bin", "wb");
    fwrite(ycbcr_to_indexed, sizeof(ycbcr_to_indexed), 1, fycbcr);
    fclose(fycbcr);

    return 0;
}
