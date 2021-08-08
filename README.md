# Drive WS281x ledstrips with 8 or 16 parallel channels on a Raspberry

__Still a work in progress__

This uses the Secondary Memory Interface (SMI) that all the Rapsberry PI's have.

Its based on the excellent ideas and code of Jeremy P Bentham.

Look at https://hackaday.com/2020/10/12/running-way-more-led-strips-on-a-raspberry-pi-with-dma/ for the technical details.

The code this is based on: https://github.com/jbentham/rpi .

## My test configuration:

 * Rapsberry PI 3
 * Running ubuntu 21.0 (32-bits)
 * NodeJS 14
 * WS2812 ledmatrixes with 8x32 pixels per matrix. (and 8 of those matrixes parallel)

Updating 8 x 8 x 32 leds at 60fps from nodejs uses around 18% CPU which it pretty amazing! (this probably even could be optmized)

## How to test yourself

 * Make sure you have a Raspberry with a 32-bit OS. 
 * Make sure you have installed a recent NodeJS. (Preferably nodejs 14 so you can use my animation framework https://github.com/psy0rz/ledder)
 * Install cmake-js globally (otherwise it wont run as root)
 * Install build-essenentials and cmake and stuff.
 * Blacklist soundcard: https://github.com/beyondscreen/node-rpi-ws281x-native#raspberry-integrated-soundcard
 * Edit smi/rpi_dma_utils.h and select correct raspberry. (wont be neccesary later on)

Since its a work in progress its still messy.









