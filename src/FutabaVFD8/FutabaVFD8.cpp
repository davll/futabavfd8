#include "FutabaVFD8.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <Arduino.h>

namespace futabavfd8 {

enum {
  TCSCP = 1,
  TCPCS = 1,
  TDOFF = 2,
  TCSW = 1,
};

static inline void vfd_send_byte(byte data)
{
    // From LSB to MSB
    for (int i = 0; i < 8; i++, data >>= 1) {
        digitalWrite(PIN_DA, ((data & 0x01) ? HIGH : LOW));
        digitalWrite(PIN_CK, LOW);
        digitalWrite(PIN_CK, HIGH);
    }
}

static inline void vfd_start_serial()
{
    digitalWrite(PIN_CS, LOW);
    delayMicroseconds(TCSCP);
}

static inline void vfd_stop_serial()
{
    digitalWrite(PIN_CS, HIGH);
    delayMicroseconds(TCSW);
}

static inline void vfd_send_cmd(byte c0)
{
    vfd_start_serial();
    vfd_send_byte(c0);
    delayMicroseconds(TCPCS);
    vfd_stop_serial();
}

static inline void vfd_send_cmd(byte c0, byte c1)
{
    vfd_start_serial();
    vfd_send_byte(c0);
    delayMicroseconds(TDOFF);
    vfd_send_byte(c1);
    delayMicroseconds(TCPCS);
    vfd_stop_serial();
}

static inline void vfd_send_cmd(byte c0, int nbytes, const byte* data)
{
    vfd_start_serial();
    vfd_send_byte(c0);
    for (; nbytes; --nbytes, ++data) {
        delayMicroseconds(TDOFF);
        vfd_send_byte(*data);
    }
    delayMicroseconds(TCPCS);
    vfd_stop_serial();
}

static inline void vfd_send_cmd_P(byte c0, int nbytes, const byte* data)
{
    vfd_start_serial();
    vfd_send_byte(c0);
    for (; nbytes; --nbytes, ++data) {
        delayMicroseconds(TDOFF);
        vfd_send_byte(pgm_read_byte_near(data));
    }
    delayMicroseconds(TCPCS);
    vfd_stop_serial();
}

static inline void vfd_send_cmd_repeat(byte c0, int n, byte data)
{
    vfd_start_serial();
    vfd_send_byte(c0);
    for (; n; --n) {
        delayMicroseconds(TDOFF);
        vfd_send_byte(data);
    }
    delayMicroseconds(TCPCS);
    vfd_stop_serial();
}

static void vfd_mode_normal()
{
    vfd_send_cmd(0xE8);
}

static void vfd_mode_all()
{
    vfd_send_cmd(0xE9);
}

static void vfd_mode_none()
{
    vfd_send_cmd(0xEA);
}

static void vfd_set_digits()
{
    vfd_send_cmd(0xE0, 0x07);
}

static void vfd_set_brightness(unsigned char level)
{
    vfd_send_cmd(0xE4, level);
}

VFD::VFD(uint8_t pinRS, uint8_t pinEN)
: m_PinRS(pinRS)
, m_PinEN(pinEN)
{
}

VFD::~VFD()
{
}

void VFD::Init()
{
    pinMode(PIN_CK, OUTPUT);
    pinMode(PIN_DA, OUTPUT);
    pinMode(PIN_CS, OUTPUT);
    pinMode(m_PinEN, OUTPUT);
    pinMode(m_PinRS, OUTPUT);
    Reset();
}

void VFD::Reset()
{
    digitalWrite(m_PinEN, HIGH);
    digitalWrite(m_PinRS, LOW);
    delay(10);
    digitalWrite(m_PinRS, HIGH);
    vfd_set_digits();
    vfd_set_brightness(0xFF);
}

void VFD::Write(int pos, uint8_t data)
{
    vfd_send_cmd(0x20 | (pos & 0x7), data);
    vfd_mode_normal();
}

void VFD::Write(int pos, int n, const uint8_t* buf)
{
    pos = pos & 0x7;
    if (pos + n > 8)
        n = 8 - pos;
    vfd_send_cmd(0x20 | pos, n, buf);
    vfd_mode_normal();
}

void VFD::Write_P(int pos, int n, const uint8_t* buf)
{
    pos = pos & 0x7;
    if (pos + n > 8)
        n = 8 - pos;
    vfd_send_cmd_P(0x20 | pos, n, buf);
    vfd_mode_normal();
}

void VFD::Printf(const char* fmt, ...)
{
    char buf[9];

    // fill formatted content
    va_list va;
    va_start(va, fmt);
    int n = vsnprintf(buf, 9, fmt, va);
    va_end(va);

    // fill white space
    for (; n < 8; ++n)
        buf[n] = 0x20;

    // send command
    Write(0, 8, (const byte*)buf);
}

void VFD::Printf_P(const char* fmt, ...)
{
    char buf[9];

    // fill formatted content
    va_list va;
    va_start(va, fmt);
    int n = vsnprintf_P(buf, 9, fmt, va);
    va_end(va);

    // fill white space
    for (; n < 8; ++n)
        buf[n] = 0x20;

    // send command
    Write(0, 8, (const byte*)buf);
}

void VFD::LPrintf(int ts, const char* fmt, ...)
{
    va_list va;

    // compute buffer size
    va_start(va, fmt);
    int needed = vsnprintf(NULL, 0, fmt, va) + 1;
    va_end(va);

    // allocate buffer
    char* buf = (char*) malloc(needed+8);
    memset(buf, 0x20, 8);

    // fill formatted content
    va_start(va, fmt);
    vsnprintf(buf+8, needed, fmt, va);
    va_end(va);

    // animate
    for (int i = 0; i < needed; ++i) {
        // send command
        Write(0, 8, (const byte*)buf+i);
        // wait for next iteration
        delay(ts);
    }

    free(buf);
}

void VFD::LPrintf_P(int ts, const char* fmt, ...)
{
    va_list va;

    // compute buffer size
    va_start(va, fmt);
    int needed = vsnprintf_P(NULL, 0, fmt, va) + 1;
    va_end(va);

    // allocate buffer
    char* buf = (char*) malloc(needed+8);
    memset(buf, 0x20, 8);

    // fill formatted content
    va_start(va, fmt);
    vsnprintf_P(buf+8, needed, fmt, va);
    va_end(va);

    // animate
    for (int i = 0; i < needed; ++i) {
        // send command
        Write(0, 8, (const byte*)buf+i);
        // wait for next iteration
        delay(ts);
    }

    free(buf);
}

void VFD::Draw(int pos, const uint8_t patterns[5])
{
    SetPatterns(pos, 5, patterns);
    Write(pos, pos & 0x7);
}

void VFD::Draw_P(int pos, const uint8_t patterns[5])
{
    SetPatterns_P(pos, 5, patterns);
    Write(pos, pos & 0x7);
}

void VFD::Draw(const uint8_t patterns[40])
{
    static const PROGMEM byte idx[8] = { 0,1,2,3,4,5,6,7 };
    SetPatterns(0, 40, patterns);
    Write_P(0, 8, idx);
}

void VFD::SetPatterns(int addr, int n, const uint8_t* patterns)
{
    addr = addr & 0x7;
    vfd_send_cmd(0x40 | addr, n, patterns);
}

void VFD::SetPatterns_P(int addr, int n, const uint8_t* patterns)
{
    addr = addr & 0x7;
    vfd_send_cmd_P(0x40 | addr, n, patterns);
}

} // namespace futabavfd8
