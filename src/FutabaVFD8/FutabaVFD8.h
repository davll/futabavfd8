//
// FutabaVFD8.h - Library for controlling Futaba VFD
//
#ifndef FUTABAVFD8_H_
#define FUTABAVFD8_H_

#include <stdint.h>

namespace futabavfd8 {

enum {
    PIN_DIN = 11, // SPI -> MOSI
    PIN_CLK = 13, // SPI -> SCK
    PIN_CS  = 10, // SPI -> SS
    PIN_DA  = PIN_DIN,
    PIN_CK  = PIN_CLK,
};

// Fubata 5x7 dot matrix vacuum fluorescent display (8 glyphs)
class VFD {
public:
    VFD(uint8_t pinRS, uint8_t pinEN);
    ~VFD();

    void Init();
    void Reset();

    void Write(int pos, uint8_t data);
    void Write(int pos, int n, const uint8_t* buf);
    void Write_P(int pos, int n, const uint8_t* buf);
    void Printf(const char* fmt, ...);
    void Printf_P(const char* fmt, ...);
    void LPrintf(int ts, const char* fmt, ...);
    void LPrintf_P(int ts, const char* fmt, ...);

    void Draw(int pos, const uint8_t patterns[5]);
    void Draw_P(int pos, const uint8_t patterns[5]);
    void Draw(const uint8_t patterns[40]);

    void SetPatterns(int addr, int n, const uint8_t* patterns);
    void SetPatterns_P(int addr, int n, const uint8_t* patterns);

private:
    const uint8_t m_PinRS;
    const uint8_t m_PinEN;
};

} // namespace futabavfd8

#endif // FUTABAVFD8_H_
