#ifndef hx711_dev_H_
#define hx711_dev_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

struct hx711_dev
{
    uint8_t pin_dout;
    uint8_t pin_slk;
    uint32_t gain;
    uint8_t gain_bit;
    uint8_t times;
    float scale;
    uint32_t offset;
    int8_t (*gpio_setup)(uint8_t pin, uint8_t mode, uint8_t default_level);
    int8_t (*gpio_read)(uint8_t pin, uint8_t *gpio_val);
    int8_t (*gpio_write)(uint8_t pin, uint8_t gpio_val);
};


void hx711_init(struct hx711_dev *hx);
bool is_ready(struct hx711_dev *hx);
void set_gain(struct hx711_dev *hx);
uint32_t read_hx(struct hx711_dev *hx);
uint32_t read_average(struct hx711_dev *hx);
double get_value(struct hx711_dev *hx);
float get_units(struct hx711_dev *hx);
void tare(struct hx711_dev *hx);
void set_scale(struct hx711_dev *hx, float scale1);
float get_scale(struct hx711_dev *hx);
void set_offset(struct hx711_dev *hx, uint32_t offset1);
uint32_t get_offset(struct hx711_dev *hx);
void power_down(struct hx711_dev *hx);
void power_up(struct hx711_dev *hx);

#endif /* hx711_dev_H_ */
