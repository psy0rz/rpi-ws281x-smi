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
    void setPixel(uint8_t channel, uint16_t pixel, uint32_t rgb);
    void send(int chan_ledcount );
    void test();


#ifdef __cplusplus
}
#endif


#endif //RPI_WS281X_SMI_SMILEDS_H
