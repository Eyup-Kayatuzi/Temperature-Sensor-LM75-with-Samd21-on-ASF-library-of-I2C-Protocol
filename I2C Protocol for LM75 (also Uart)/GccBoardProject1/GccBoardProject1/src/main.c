/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * This is a bare minimum user application template.
 *
 * For documentation of the board, go \ref group_common_boards "here" for a link
 * to the board-specific documentation.
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# Minimal main function that starts with a call to system_init()
 * -# Basic usage of on-board LED and button
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#include <asf.h>
#include <string.h> // for memset
#include "SERCOMforUSART.h"

#define DATA_LENGTH 1static uint8_t wr_buffer[DATA_LENGTH] = {0x00};static uint8_t rd_buffer[2]; // datafloat cc = 0x00;uint8_t deger1, deger2;uint8_t str[7] = {0, 0, 0, 0, 0, '\r', '\n'};#define SLAVE_ADDRESS 0x4f // address of lm75 sensorstruct i2c_master_module i2c_master_instance;struct i2c_master_packet wr_packet;
struct i2c_master_packet rd_packet;void configure_i2c(void);
void i2c_write_complete_callback(struct i2c_master_module *const module);void i2c_read_complete_callback(struct i2c_master_module *const module);void configure_i2c_callbacks(void);char *	_EXFUN(gcvt,(double,int,char *));

int main (void)
{
	system_init();
	delay_init();
	PM->APBCMASK.reg |= PM_APBCMASK_SERCOM3;
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_SERCOM3_CORE;
	PORT->Group[0].PINCFG[22].bit.PMUXEN = 1;
	PORT->Group[0].PMUX[11].bit.PMUXE = 0x2; // pa22 is config as sercom3 pad[0]
	PORT->Group[0].PINCFG[23].bit.PMUXEN = 1;
	PORT->Group[0].PMUX[11].bit.PMUXO = 0x2; // pa23 is config as sercom3 pad[1]
	system_interrupt_enable_global(); // global interrupt is enabled
	uartInit(115200);
	configure_i2c();
	configure_i2c_callbacks();
	wr_packet.address = SLAVE_ADDRESS;
	wr_packet.data_length = DATA_LENGTH;
	wr_packet.data = wr_buffer;
	
	rd_packet.address     = SLAVE_ADDRESS;
	rd_packet.data_length = 2; // msb and lsb data is read
	rd_packet.data        = rd_buffer;
	
	while (1) {
		delay_ms(1000); 
		i2c_master_write_packet_job_no_stop(&i2c_master_instance, &wr_packet);		
	}
}

void i2c_write_complete_callback(struct i2c_master_module *const module)
{
	i2c_master_read_packet_job(&i2c_master_instance,&rd_packet);
}

void i2c_read_complete_callback(struct i2c_master_module *const module)
{
	deger1 = rd_buffer[0];
	deger2 = rd_buffer[1];
	deger1 <<= 1;
	deger2 &= 0x80;
	deger2 >>= 7;
	cc = (deger1 + deger2);
	cc *= 0.5f;
    gcvt(cc, 5, str);
	uartWrite(str, 7);
	memset(str, ' ', 5);
}

void configure_i2c(void)
{
	/* Initialize config structure and software module */
	struct i2c_master_config config_i2c_master;
	i2c_master_get_config_defaults(&config_i2c_master);
	/* Change buffer timeout to something longer */
	//config_i2c_master.baud_rate = 100;
	config_i2c_master.generator_source = GCLK_GENERATOR_0;
	config_i2c_master.buffer_timeout = 65535;
	/* Initialize and enable device with config */
	config_i2c_master.pinmux_pad0 = PINMUX_PA08D_SERCOM2_PAD0; // sda
	config_i2c_master.pinmux_pad1 = PINMUX_PA09D_SERCOM2_PAD1; // scl
	while(i2c_master_init(&i2c_master_instance, SERCOM2, &config_i2c_master) != STATUS_OK);
	i2c_master_enable(&i2c_master_instance);
}

void configure_i2c_callbacks(void)
{
	/* Register callback function. */
	i2c_master_register_callback(&i2c_master_instance, i2c_write_complete_callback, I2C_MASTER_CALLBACK_WRITE_COMPLETE);
	i2c_master_register_callback(&i2c_master_instance, i2c_read_complete_callback, I2C_MASTER_CALLBACK_READ_COMPLETE);
	i2c_master_enable_callback(&i2c_master_instance, I2C_MASTER_CALLBACK_WRITE_COMPLETE);
	i2c_master_enable_callback(&i2c_master_instance, I2C_MASTER_CALLBACK_READ_COMPLETE);
}
