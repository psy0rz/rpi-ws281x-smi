//
// Created by psy on 07-08-21.
// Based on rpi_pixleds.c from Jeremy P Bentham
//

#include "smileds.h"
#include "smi/rpi_dma_utils.h"
#include "smi/rpi_smi_defs.h"

extern MEM_MAP gpio_regs, clk_regs, dma_regs;
MEM_MAP vc_mem, smi_regs;

// Map GPIO, DMA and SMI registers into virtual mem (user space)
// If any of these fail, program will be terminated
void map_devices(void)
{
    map_periph(&gpio_regs, (void *)GPIO_BASE, PAGE_SIZE);
    map_periph(&dma_regs, (void *)DMA_BASE, PAGE_SIZE);
    map_periph(&clk_regs, (void *)CLK_BASE, PAGE_SIZE);
    map_periph(&smi_regs, (void *)SMI_BASE, PAGE_SIZE);
}


void init(int chan_led_count)
{
    map_devices();


}