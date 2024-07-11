#include "pico/stdlib.h"

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"


#include "pico/binary_info.h"

#include "hardware/i2c.h"

#include "pico-ssd1306/ssd1306.h"
#include "pico-ssd1306/textRenderer/TextRenderer.h"

void gpio_callback(uint gpio, uint32_t event_mask) {
	printf("INTERRUPT\n");
}
 
void vInputHandler(void* unused_arg) {
	int loops = 0;

	for (;;) {
		printf("Running for %d seconds...\n", loops);
		loops++;
		vTaskDelay(1000);
	}
}



bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

void vI2cScanTask(void* unused_arg) {
	vTaskDelay(5000);
	// This example will use I2C0 on the default SDA and SCL pins (GP4, GP5 on a Pico)
	i2c_init(i2c0, 100 * 1000);
	gpio_set_function(4, GPIO_FUNC_I2C);
	gpio_set_function(5, GPIO_FUNC_I2C);
	gpio_pull_up(4);
	gpio_pull_up(5);
	// Make the I2C pins available to picotool
	bi_decl(bi_2pins_with_func(4, 5, GPIO_FUNC_I2C));

	printf("\nI2C Bus Scan\n");
	printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

	for (int addr = 0; addr < (1 << 7); ++addr) {
		if (addr % 16 == 0) {
		printf("%02x ", addr);
        }

        // Perform a 1-byte dummy read from the probe address. If a slave
        // acknowledges this address, the function returns the number of bytes
        // transferred. If the address byte is ignored, the function returns
        // -1.

        // Skip over any reserved addresses.
        int ret;
        uint8_t rxdata;
        if (reserved_addr(addr))
		ret = PICO_ERROR_GENERIC;
	else
		ret = i2c_read_blocking(i2c_default, addr, &rxdata, 1, false);

        printf(ret < 0 ? "." : "@");
        printf(addr % 16 == 15 ? "\n" : "  ");
    }
    printf("Done.\n");
}

void vDisplayTask(void* unused_arg) {
	const uint8_t DISPLAY_I2C_PIN_SDA = 4;
	const uint8_t DISPLAY_I2C_PIN_SCL = 5;

	i2c_init(i2c0, 1000000); //Use i2c port with baud rate of 1Mhz
	//Set pins for I2C operation
	gpio_set_function(DISPLAY_I2C_PIN_SDA, GPIO_FUNC_I2C);
	gpio_set_function(DISPLAY_I2C_PIN_SCL, GPIO_FUNC_I2C);
	gpio_pull_up(DISPLAY_I2C_PIN_SDA);
	gpio_pull_up(DISPLAY_I2C_PIN_SCL);
	
	//Create a new display object
	pico_ssd1306::SSD1306 display = pico_ssd1306::SSD1306(i2c0, 0x3C, pico_ssd1306::Size::W128xH64);
	display.setOrientation(0);

	/*
	//create a vertical line on x: 64 y:0-63
	for (int y = 0; y < 64; y++){
	    display.setPixel(64, y);
	}
      */


	bool flash = 0;

	for (int i = 0; i < 256; i++) {

		display.clear();

		drawText(&display, font_8x8, "TEST text", 0 , 0);

		if (flash==0) {display.setContrast(1); flash = 1;} else {display.setContrast(255); flash = 0;}
 
		display.sendBuffer(); //Send buffer to device and show on screen
		
		vTaskDelay(500);
	}
}

int main() {
	stdio_init_all();

	
	gpio_init(11);
	gpio_set_dir(11, GPIO_IN);
	gpio_pull_up(11);

	gpio_set_irq_enabled_with_callback(11, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
	

	xTaskCreate(vInputHandler, "Read rotary encoder", 8192, NULL, 1, NULL);
	xTaskCreate(vDisplayTask, "Initialise and print to the display", 8192, NULL, 1, NULL);
	//xTaskCreate(vI2cScanTask, "Scan I2C Ports", 8192, NULL, 1, NULL);

	vTaskStartScheduler();
}
