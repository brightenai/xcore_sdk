/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
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
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "demo_main.h"
#include "tusb.h"

// echo to either Serial0 or Serial1
// with Serial0 as all lower case, Serial1 as all upper case
static void echo_serial_port(uint8_t itf, uint8_t buf[], uint32_t count)
{
  for(uint32_t i=0; i<count; i++)
  {
    if (itf == 0)
    {
      // echo back 1st port as lower case
      if (isupper(buf[i])) buf[i] += 'a' - 'A';
    }
    else
    {
      // echo back additional ports as upper case
      if (islower(buf[i])) buf[i] -= 'a' - 'A';
    }

    tud_cdc_n_write_char(itf, buf[i]);

    if ( buf[i] == '\r' ) tud_cdc_n_write_char(itf, '\n');
  }
  tud_cdc_n_write_flush(itf);
}

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
static void cdc_task(void *arg)
{
    uint8_t itf = *((uint8_t *)arg);

    while(1) {
        // connected() check for DTR bit
        // Most but not all terminal client set this when making connection
        if ( tud_cdc_n_connected(itf) )
        {
            if ( tud_cdc_n_available(itf) )
            {
                uint8_t buf[64];

                uint32_t count = tud_cdc_n_read(itf, buf, sizeof(buf));

                // echo back to both serial ports
                echo_serial_port(0, buf, count);
                echo_serial_port(1, buf, count);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    vPortFree(arg); // Should never get here
}

void create_tinyusb_demo(rtos_gpio_t *ctx, unsigned priority)
{
    for (uint8_t itf = 0; itf < CFG_TUD_CDC; itf++)
    {
        uint8_t *itf_num = pvPortMalloc(sizeof(uint8_t*));
        *itf_num = itf;
        xTaskCreate((TaskFunction_t) cdc_task,
                    "cdc_task",
                    portTASK_STACK_DEPTH(cdc_task),
                    itf_num,
                    priority,
                    NULL);
    }
}
