/* SWSPI, Software SPI library
 * Copyright (c) 2012-2014, David R. Van Wagner, http://techwithdave.blogspot.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <mbed.h>
#include "SWSPI.h"

SWSPI::SWSPI(PinName mosi_pin, PinName miso_pin, PinName sclk_pin):_fast(false)
{
    mosi = new DigitalInOut(mosi_pin);
    mosi->input();
    mosi->mode(PullNone);
    miso = new DigitalInOut(miso_pin);
    miso->input();
    miso->mode(PullNone);
    sclk = new DigitalInOut(sclk_pin);
    sclk->input();
    sclk->mode(PullNone);
    format(8);
    frequency();
}

SWSPI::~SWSPI()
{
    delete mosi;
    delete miso;
    delete sclk;
}

void SWSPI::format(int bits, int mode)
{
    this->bits = bits;
    this->mode = mode;
    polarity = (mode >> 1) & 1;
    phase = mode & 1;
    sclk->write(polarity);
}

void SWSPI::frequency(int hz)
{
    this->freq = hz;
    _fast = (hz >= 1000000) ? true : false;
}

#pragma Otime

int SWSPI::write(int value)
{
    mosi->output();
    mosi->mode(PullNone);
    miso->input();
    miso->mode(PullNone);
    sclk->output();
    sclk->mode(PullNone);

    int read = 0;
    if (_fast) {
        read = fast_write(value);
    }
    for (int bit = bits-1; bit >= 0; --bit)
    {
        mosi->write(((value >> bit) & 0x01) != 0);

        if (phase == 0)
        {
            if (miso->read())
                read |= (1 << bit);
        }

        sclk->write(!polarity);

        wait_us(1000000/freq/2);

        if (phase == 1)
        {
            if (miso->read())
                read |= (1 << bit);
        }

        sclk->write(polarity);

        wait_us(1000000/freq/2);
    }

    mosi->input();
    mosi->mode(PullNone);
    miso->input();
    miso->mode(PullNone);
    sclk->input();
    sclk->mode(PullNone);
    
    return read;
}

uint8_t SWSPI::fast_write(uint8_t value)
{
    uint8_t data = 0;
    for(uint8_t mask = 0x80; mask; mask >>= 1) {
        mosi->write((value & mask) ? 1 : 0);
        if (miso->read()) {
            data |= mask;
        }
        sclk->write(1);
        sclk->write(0);
    }
    return data;
}

