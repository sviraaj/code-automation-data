#include "hx711_dev.h"

void hx711_init(struct hx711_dev *hx)
{
    uint8_t status;
    status = hx->gpio_setup(hx->pin_slk, GPIO_MODE_OUTPUT, GPIO_LEVEL_LOW);
    status = hx->gpio_setup(hx->pin_dout, GPIO_MODE_INPUT, GPIO_LEVEL_LOW);

    set_gain(hx);
}

bool is_ready(struct hx711_dev *hx)
{
    uint8_t tmp = 1;
    hx->gpio_read(hx->pin_dout, &tmp);
    if (tmp == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

uint32_t read_hx(struct hx711_dev *hx)
{
    // wait for the chip to become ready
    while (!is_ready(hx))
    {
        // Will do nothing here but prevent resets of ESP8266 (Watchdog Issue)
    }
    unsigned uint32_t value = 0;
    uint8_t data[3] = {0};
    uint8_t filler = 0x00;
    uint8_t j;
    uint8_t i1;
    uint8_t i;
    uint8_t status;
    uint8_t tmp;

    // pulse the clock pin 24 times to read the data
    for (j = 3; j > 0; j--)
    {
        for (i = 8; i > 0; i--)
        {
            hx->gpio_write(hx->pin_slk, 1);
            status = hx->gpio_read(hx->pin_dout, &tmp);
            data[j - 1] |= (tmp << (i - 1));
            hx->gpio_write(hx->pin_slk, 0);
        }
    }

    // set the channel and the gain factor for the next reading using the clock
    // pin
    for (i1 = 0; i1 < (hx->gain_bit); i1++)
    {
        hx->gpio_write(hx->pin_slk, 1);
        hx->gpio_write(hx->pin_slk, 0);
    }

    // Replicate the most significant bit to pad out a 32-bit signed integer
    if (data[2] & 0x80)
    {
        filler = 0xFF;
    }
    else
    {
        filler = 0x00;
    }

    // Construct a 32-bit signed integer
    value = ((unsigned uint32_t)(filler) << 24 |
             (unsigned uint32_t)(data[2]) << 16 |
             (unsigned uint32_t)(data[1]) << 8 | (unsigned uint32_t)(data[0]));

    return (uint32_t)(value);
}

void set_gain(struct hx711_dev *hx)
{
    uint8_t status;
    switch (hx->gain)
    {
        case 128:  // channel A, gain factor 128
            hx->gain_bit = 1;
            break;
        case 64:  // channel A, gain factor 64
            hx->gain_bit = 3;
            break;
        case 32:  // channel B, gain factor 32
            hx->gain_bit = 2;
            break;
    }
    status = hx->gpio_write(hx->pin_slk, 0);
    read_hx(hx);
}

uint32_t read_average(struct hx711_dev *hx)
{
    uint32_t sum = 0;
    uint8_t i;
    for (i = 0; i < hx->times; i++)
    {
        sum += read_hx(hx);
    }
    return sum / hx->times;
}

double get_value(struct hx711_dev *hx) { return read_average(hx) - hx->offset; }
float get_units(struct hx711_dev *hx) { return get_value(hx) / hx->scale; }
void tare(struct hx711_dev *hx)
{
    double sum = read_average(hx);
    set_offset(hx, sum);
}

void set_scale(struct hx711_dev *hx, float scale1) { hx->scale = scale1; }
float get_scale(struct hx711_dev *hx) { return hx->scale; }
void set_offset(struct hx711_dev *hx, uint32_t offset1)
{
    hx->offset = offset1;
}

uint32_t get_offset(struct hx711_dev *hx) { return hx->offset; }
void power_down(struct hx711_dev *hx)
{
    hx->gpio_write(hx->pin_slk, 0);
    hx->gpio_write(hx->pin_slk, 1);
}

void power_up(struct hx711_dev *hx) { hx->gpio_write(hx->pin_slk, 0); }
