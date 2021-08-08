//
// Created by psy on 07-08-21.
//

#include <stdbool.h>

#ifndef RPI_WS281X_SMI_SMILEDS_H
#define RPI_WS281X_SMI_SMILEDS_H



#ifdef __cplusplus
extern "C"
{
#endif

    bool leds_init(int init_led_count);
    void leds_set_pixel(uint8_t channel, uint16_t pixel, uint32_t rgb);
    void leds_send();
    void leds_clear();
    void test();


#ifdef __cplusplus
}
#endif


#endif //RPI_WS281X_SMI_SMILEDS_H
