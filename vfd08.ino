#include "src/FutabaVFD8/FutabaVFD8.h"

futabavfd8::VFD vfd(8,7);

static const PROGMEM byte GLYPH_FILLED[5] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static const PROGMEM byte WAVEFRONT[16] = {
    0x08, 0x10, 0x30, 0x60, 0x40, 0x60, 0x30, 0x10,
    0x08, 0x04, 0x02, 0x03, 0x01, 0x03, 0x02, 0x04,
};

void setup()
{
    vfd.Init();
}

static void wave()
{
    static byte glyphs[40];
    memset(glyphs, 0, 40);
    for (int i = 0; i < 20; ++i)
        glyphs[i] = 0x02;

    for (int n = 0; n < 160; ++n) {
        // move patterns
        memmove(glyphs, glyphs+1, 39);
        // set wavefront
        glyphs[39] = pgm_read_byte_near(WAVEFRONT + (n & 0xF));
        // render
        vfd.Draw(glyphs);
        // wait
        delay(10);
    }
}

void loop()
{
    for (int i = 0; i < 8; ++i) {
        vfd.Draw_P(i, GLYPH_FILLED);
        delay(300);
    }
    vfd.Printf_P(PSTR("Hello World"));
    delay(1000);
    vfd.Printf_P(PSTR("%02d:%02d:%02d"), 12, 24, 36);
    delay(1000);
    vfd.LPrintf_P(300, PSTR("The quick brown fox jumps over the lazy dog %d%d%d%d%d"), 12, 34, 56, 78, 90);
    delay(1000);
    wave();
    delay(1000);
}
