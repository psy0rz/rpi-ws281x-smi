//
// Created by psy on 07-08-21.
// Based on rpi_pixleds.c from Jeremy P Bentham
//

#define _DEFAULT_SOURCE

#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "smileds.h"
#include "smi/rpi_dma_utils.h"
#include "smi/rpi_smi_defs.h"


#if PHYS_REG_BASE == PI_4_REG_BASE        // Timings for RPi v4 (1.5 GHz)
#define SMI_TIMING       10, 15, 30, 15    // 400 ns cycle time
#else                                   // Timings for RPi v0-3 (1 GHz)
#define SMI_TIMING       10, 10, 20, 10   // 400 ns cycle time
#endif

#define LED_D0_PIN      8   // GPIO pin for D0 output
#define LED_NCHANS      8   // Number of LED channels (8 or 16)
#define LED_NBITS       24  // Number of data bits per LED
#define LED_PREBITS     4   // Number of zero bits before LED data
#define LED_POSTBITS    4   // Number of zero bits after LED data
#define BIT_NPULSES     3   // Number of O/P pulses per LED bit
#define CHAN_MAXLEDS    50  // Maximum number of LEDs per channel
#define CHASE_MSEC      100 // Delay time for chaser light test
#define REQUEST_THRESH  2   // DMA request threshold
#define DMA_CHAN        10  // DMA channel to use

// Length of data for 1 row (1 LED on each channel)
#define LED_DLEN        (LED_NBITS * BIT_NPULSES)

// Transmit data type, 8 or 16 bits
#if LED_NCHANS > 8
#define TXDATA_T        uint16_t
#else
#define TXDATA_T        uint8_t
#endif

// Structures for mapped I/O devices, and non-volatile memory
extern MEM_MAP gpio_regs, clk_regs, dma_regs;
MEM_MAP vc_mem, smi_regs;


// Pointers to SMI registers
volatile SMI_CS_REG *smi_cs;
volatile SMI_L_REG *smi_l;
volatile SMI_A_REG *smi_a;
volatile SMI_D_REG *smi_d;
volatile SMI_DMC_REG *smi_dmc;
volatile SMI_DSR_REG *smi_dsr;
volatile SMI_DSW_REG *smi_dsw;
volatile SMI_DCS_REG *smi_dcs;
volatile SMI_DCA_REG *smi_dca;
volatile SMI_DCD_REG *smi_dcd;

// Ofset into Tx data buffer, given LED number in chan
#define LED_TX_OSET(n)      (LED_PREBITS + (LED_DLEN * (n)))

// Size of data buffers & NV memory, given number of LEDs per chan
#define TX_BUFF_LEN(n)      (LED_TX_OSET(n) + LED_POSTBITS)
#define TX_BUFF_SIZE(n)     (TX_BUFF_LEN(n) * sizeof(TXDATA_T))
#define VC_MEM_SIZE         (PAGE_SIZE + TX_BUFF_SIZE(CHAN_MAXLEDS))

// RGB values for test mode (1 value for each of 16 channels)
int on_rgbs[16] = {0xff0000, 0x00ff00, 0x0000ff, 0xffffff,
                   0xff4040, 0x40ff40, 0x4040ff, 0x404040,
                   0xff0000, 0x00ff00, 0x0000ff, 0xffffff,
                   0xff4040, 0x40ff40, 0x4040ff, 0x404040};
int off_rgbs[16];

#if TX_TEST
// Data for simple transmission test
TXDATA_T tx_test_data[] = {1, 2, 3, 4, 5, 6, 7, 0};
#endif

TXDATA_T *txdata;                       // Pointer to uncached Tx data buffer
TXDATA_T tx_buffer[TX_BUFF_LEN(CHAN_MAXLEDS)];  // Tx buffer for assembling data
int rgb_data[CHAN_MAXLEDS][LED_NCHANS]; // RGB data
int chan_num;                           // Current channel for data I/P

// Map GPIO, DMA and SMI registers into virtual mem (user space)
// If any of these fail, program will be terminated
void map_devices(void) {
    map_periph(&gpio_regs, (void *) GPIO_BASE, PAGE_SIZE);
    map_periph(&dma_regs, (void *) DMA_BASE, PAGE_SIZE);
    map_periph(&clk_regs, (void *) CLK_BASE, PAGE_SIZE);
    map_periph(&smi_regs, (void *) SMI_BASE, PAGE_SIZE);
}

// Initialise SMI, given data width, time step, and setup/hold/strobe counts
// Step value is in nanoseconds: even numbers, 2 to 30
void init_smi(int width, int ns, int setup, int strobe, int hold) {
    int i, divi = ns / 2;

    smi_cs = (SMI_CS_REG *) REG32(smi_regs, SMI_CS);
    smi_l = (SMI_L_REG *) REG32(smi_regs, SMI_L);
    smi_a = (SMI_A_REG *) REG32(smi_regs, SMI_A);
    smi_d = (SMI_D_REG *) REG32(smi_regs, SMI_D);
    smi_dmc = (SMI_DMC_REG *) REG32(smi_regs, SMI_DMC);
    smi_dsr = (SMI_DSR_REG *) REG32(smi_regs, SMI_DSR0);
    smi_dsw = (SMI_DSW_REG *) REG32(smi_regs, SMI_DSW0);
    smi_dcs = (SMI_DCS_REG *) REG32(smi_regs, SMI_DCS);
    smi_dca = (SMI_DCA_REG *) REG32(smi_regs, SMI_DCA);
    smi_dcd = (SMI_DCD_REG *) REG32(smi_regs, SMI_DCD);
    smi_cs->value = smi_l->value = smi_a->value = 0;
    smi_dsr->value = smi_dsw->value = smi_dcs->value = smi_dca->value = 0;
    if (*REG32(clk_regs, CLK_SMI_DIV) != divi << 12) {
        *REG32(clk_regs, CLK_SMI_CTL) = CLK_PASSWD | (1 << 5);
        usleep(10);
        while (*REG32(clk_regs, CLK_SMI_CTL) & (1 << 7));
        usleep(10);
        *REG32(clk_regs, CLK_SMI_DIV) = CLK_PASSWD | (divi << 12);
        usleep(10);
        *REG32(clk_regs, CLK_SMI_CTL) = CLK_PASSWD | 6 | (1 << 4);
        usleep(10);
        while ((*REG32(clk_regs, CLK_SMI_CTL) & (1 << 7)) == 0);
        usleep(100);
    }
    if (smi_cs->seterr)
        smi_cs->seterr = 1;
    smi_dsr->rsetup = smi_dsw->wsetup = setup;
    smi_dsr->rstrobe = smi_dsw->wstrobe = strobe;
    smi_dsr->rhold = smi_dsw->whold = hold;
    smi_dmc->panicr = smi_dmc->panicw = 8;
    smi_dmc->reqr = smi_dmc->reqw = REQUEST_THRESH;
    smi_dsr->rwidth = smi_dsw->wwidth = width;
    for (i = 0; i < LED_NCHANS; i++)
        gpio_mode(LED_D0_PIN + i, GPIO_ALT1);
}

// Set up SMI transfers using DMA
void setup_smi_dma(MEM_MAP *mp, int nsamp) {
    DMA_CB *cbs = mp->virt;

    txdata = (TXDATA_T * )(cbs + 1);
    smi_dmc->dmaen = 1;
    smi_cs->enable = 1;
    smi_cs->clear = 1;
    smi_cs->pxldat = 1;
    smi_l->len = nsamp * sizeof(TXDATA_T);
    smi_cs->write = 1;
    enable_dma(DMA_CHAN);
    cbs[0].ti = DMA_DEST_DREQ | (DMA_SMI_DREQ << 16) | DMA_CB_SRCE_INC | DMA_WAIT_RESP;
    cbs[0].tfr_len = nsamp * sizeof(TXDATA_T);
    cbs[0].srce_ad = MEM_BUS_ADDR(mp, txdata);
    cbs[0].dest_ad = REG_BUS_ADDR(smi_regs, SMI_D);
}

// Start SMI DMA transfers
void start_smi(MEM_MAP *mp) {
    DMA_CB *cbs = mp->virt;

    start_dma(mp, DMA_CHAN, &cbs[0], 0);
    smi_cs->start = 1;
}

// Swap adjacent bytes in transmit data
void swap_bytes(void *data, int len) {
    uint16_t *wp = (uint16_t *) data;

    len = (len + 1) / 2;
    while (len-- > 0) {
        *wp = __builtin_bswap16(*wp);
        wp++;
    }
}


void init(int chan_ledcount) {
    map_devices();
    init_smi(LED_NCHANS > 8 ? SMI_16_BITS : SMI_8_BITS, SMI_TIMING);
    map_uncached_mem(&vc_mem, VC_MEM_SIZE);
    setup_smi_dma(&vc_mem, TX_BUFF_LEN(chan_ledcount));

    printf("smileds: Setting %u LED%s per channel, %u channels\n",
           chan_ledcount, chan_ledcount == 1 ? "" : "s", LED_NCHANS);
}


//set rgb values for a specific channel and pixel
void set_pixel(uint8_t channel, uint16_t pixel, uint32_t rgb) {
    TXDATA_T *tx_offset = &tx_buffer[LED_TX_OSET(pixel)];

    // For each bit of the 24-bit RGB values..
    for (uint16_t n = 0; n < LED_NBITS; n++) {
        // 1st always is a high pulse on all lines
        //XXX: do this one time ever?
        tx_offset[0] = (TXDATA_T) 0xffff;

        // 2nd is the actual bit
        if (rgb & (1 << n))
            tx_offset[1]|= ( 1 << channel );
        else
            tx_offset[1]&= ~( 1 << channel );

        // 3rd alawys is a low pulse on all lines
        tx_offset[2] = 0;

        tx_offset += BIT_NPULSES;

    }
}

void start_send(int chan_led_count) {
//    printf("send\n");
#if LED_NCHANS <= 8
    swap_bytes(tx_buffer, TX_BUFF_SIZE(chan_led_count));
#endif
    memcpy(txdata, tx_buffer, TX_BUFF_SIZE(chan_led_count));
    start_smi(&vc_mem);
}





void test()
{
    const int leds=10;

    init(leds);

    int on=0;
    while(1)
    {
        on=(on+1)%leds;
        for (int l=0; l<leds; l++)
        {

            for (int c=0; c<8; c++)
            {
                if (l==on)
                    set_pixel(c,l,0x000010);
                else
                    set_pixel(c,l,0x0);
            }
        }
        start_send(leds);
        usleep(1000000/60);
    }

}


// Set Tx data for 8 or 16 chans, 1 LED per chan, given 1 RGB val per chan
// Logic 1 is 0.8us high, 0.4 us low, logic 0 is 0.4us high, 0.8us low
void rgb_txdata(int *rgbs, TXDATA_T *txd) {
    int i, n, msk;

    // For each bit of the 24-bit RGB values..
    for (n = 0; n < LED_NBITS; n++) {
        // Mask to convert RGB to GRB, M.S bit first
        msk = n == 0 ? 0x8000 : n == 8 ? 0x800000 : n == 16 ? 0x80 : msk >> 1;
        // 1st byte or word is a high pulse on all lines
        txd[0] = (TXDATA_T) 0xffff;
        // 2nd has high or low bits from data
        // 3rd is a low pulse
        txd[1] = txd[2] = 0;
        for (i = 0; i < LED_NCHANS; i++) {
            if (rgbs[i] & msk)
                txd[1] |= (1 << i);
        }
        txd += BIT_NPULSES;
    }
}


//int testmode, chan_ledcount = 1;          // Command-line parameters
//
//void test() {
//    int args = 0, n, oset = 0;
//    testmode = 1;
//    chan_ledcount = 10;
////signal(SIGINT, terminate);
//
//init(chan_ledcount);
//
//    if (testmode) {
//        while (1) {
//            if (chan_ledcount < 2)
//                rgb_txdata(oset & 1 ? off_rgbs : on_rgbs, tx_buffer);
//            else {
//                for (n = 0; n < chan_ledcount; n++) {
//                    rgb_txdata(n == oset % chan_ledcount ? on_rgbs : off_rgbs,
//                               &tx_buffer[LED_TX_OSET(n)]);
//                }
//            }
//            oset++;
//
//
//            start_send(chan_ledcount);
//
//
//
//            usleep(CHASE_MSEC * 1000);
//        }
//    }
////terminate(0);
////    return (0);
//}


