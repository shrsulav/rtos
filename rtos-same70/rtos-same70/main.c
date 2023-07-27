/**
 * \file
 *
 * \brief Application implement
 *
 * Copyright (c) 2015-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include "atmel_start.h"
#include "atmel_start_pins.h"
#include <string.h>
#include "led.h"
#include "delay.h"
#include "print_utility.h"
#include "peripheral_clk_config.h"
#include "task.h"
static uint8_t example_hello_world[14] = "Hello World!\r\n";

volatile static uint32_t data_arrived = 0;

static void tx_cb_EDBG_COM(const struct usart_async_descriptor *const io_descr)
{
	/* Transfer completed */
	gpio_toggle_pin_level(LED0);
}

static void rx_cb_EDBG_COM(const struct usart_async_descriptor *const io_descr)
{
	/* Receive completed */
	data_arrived = 1;
}

static void err_cb_EDBG_COM(const struct usart_async_descriptor *const io_descr)
{
	/* error handle */
	io_write(&EDBG_COM.io, example_hello_world, 14);
}

void task1(void)
{
    while(1){
        set_led_on();
        while (io_write(&EDBG_COM.io, "x", 1) != 1) {}
        delay(0x0FFFFFFF);        
    }
}

void task2(void)
{
    while(1)
    {
        set_led_off();
        while (io_write(&EDBG_COM.io, "y", 1) != 1) {}
        delay(0x0FFFFFF);
    }
}

extern tcb_t g_tcbs[10];

extern tcb_t *g_current_task, *g_next_task;

int main(void)
{
	uint8_t recv_char;

	atmel_start_init();

	usart_async_register_callback(&EDBG_COM, USART_ASYNC_TXC_CB, tx_cb_EDBG_COM);
	usart_async_register_callback(&EDBG_COM, USART_ASYNC_RXC_CB, rx_cb_EDBG_COM);
	usart_async_register_callback(&EDBG_COM, USART_ASYNC_ERROR_CB, err_cb_EDBG_COM);
	usart_async_enable(&EDBG_COM);

	// io_write(&EDBG_COM.io, example_hello_world, 14);
    uint8_t led_status = 0;
    
    init_os();
    create_task(task1);
    create_task(task2);
    g_tcbs[1].next_task = &g_tcbs[2];
    g_tcbs[2].next_task = &g_tcbs[1];
    g_current_task = &g_tcbs[1];
    
	SysTick_Config(CONF_HCLK_FREQUENCY/1000);
    run_scheduler();
    while (1) {
        
        led_status = (led_status+1)%2;
        gpio_set_pin_level(LED0, led_status);
        
        while (io_write(&EDBG_COM.io, "a", 1) != 1) {}
            
        delay(0x0FFFFFFF);
	}
}