//
// Created by psy on 07-08-21.
//

#ifndef RPI_WS281X_SMI_SMILEDS_H
#define RPI_WS281X_SMI_SMILEDS_H

#ifdef __cplusplus
extern "C"
{
#endif

    void init(int chan_led_count);
    void set_pixel(uint8_t channel, uint16_t pixel, uint32_t rgb);
    void start_send(int chan_led_count );
    void test();


#ifdef __cplusplus
}
#endif


#endif //RPI_WS281X_SMI_SMILEDS_H
