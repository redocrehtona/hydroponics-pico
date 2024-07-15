#define SSD1306_ASCII_FULL

#include "pico/stdlib.h"

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"


#include "pico/binary_info.h"

#include "hardware/i2c.h"

#include "pico-ssd1306/ssd1306.h"
#include "pico-ssd1306/textRenderer/TextRenderer.h"

bool inputState = 0;			// Becomes true when in setting mode
bool encoderButton = 0;			// Becomes true if the encoder button has been pressed 
bool encoderB = 0;			// Becomes true if encoder is starting to turn clockwise
bool encoderA = 0;			// Becomes true if encoder is starting to turn counterclockwise
int encoderRotation = 0;		// Increases with each full click clockwise, decreases counterclockwise


void gpio_callback(uint gpio, uint32_t event_mask) {
	printf("INTERRUPT %d\n", gpio);

	switch (gpio) {	
		// ENC A
		case 9:
			if (!encoderB && !encoderA) {
				encoderA = 1;
			} else if (encoderB && !encoderA) {
				encoderRotation -= 1;
				encoderB = 0;
			} else if (encoderB && encoderA) {
				printf("ENCODER ERROR: BOTH DIRECTIONS");
				encoderA = 0;
				encoderB = 0;
			}
			break;
		// ENC B
		case 10:
			if (!encoderB && !encoderA) {
				encoderB = 1;
			} else if (!encoderB && encoderA) {
				encoderRotation += 1;
				encoderA = 0;
			} else if (encoderB && encoderA) {
				printf("ENCODER ERROR: BOTH DIRECTIONS");
				encoderA = 0;
				encoderB = 0;
			}
			break;	
		// Encoder button
		case 11: 
			encoderButton = 1;
			break;
	}
}


 
void vInputHandler(void* unused_arg) {
	int presses = 0;

	for (;;) {
		printf("Encoder Rotation: %d\n", encoderRotation);
		// Check inputs
		if (encoderButton == 1) {
			presses += 1;
			printf("Encoder Button Pressed %d times\n", presses);
			vTaskDelay(500);
			encoderButton = 0;
		} else {
		vTaskDelay(250);
		}
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
	display.setContrast(255);

	drawText(&display, font_8x8, "LED TEMP:", 0, 0);
	drawText(&display, font_8x8, "20.0C", 88, 0);

	drawText(&display, font_8x8, "LED STATE:", 0, 10);
	drawText(&display, font_8x8, "ON", 112, 10);

	drawText(&display, font_8x8, "FAN SPEED:", 0, 20);
	drawText(&display, font_8x8, "100%", 96, 20);

	drawText(&display, font_8x8, "TIMER ON:", 0, 30);
	drawText(&display, font_8x8, "07:00", 88, 30);

	drawText(&display, font_8x8, "TIMER OFF:", 0, 40);
	drawText(&display, font_8x8, "20:00", 88, 40); 

	drawText(&display, font_8x8, "TIME NOW:", 0, 50);
	drawText(&display, font_8x8, "13:07", 88, 50);


	display.sendBuffer(); 

	/*
	//create a vertical line on x: 64 y:0-63
	for (int y = 0; y < 64; y++){
	    display.setPixel(64, y);
	}
        */

	// FLASH THE DISPLAY BRIGHTNESS
	/*
	bool flash = 0;

	for (int i = 0; i < 256; i++) {

		display.clear();

		drawText(&display, font_8x8, "TEST text", 0 , 0);

		if (flash==0) {display.setContrast(1); flash = 1;} else {display.setContrast(255); flash = 0;}
 
		display.sendBuffer(); //Send buffer to device and show on screen
		
		vTaskDelay(500);
	}
	*/
}

int main() {
	stdio_init_all();

	// Interrupt for ENC A
	gpio_init(9);
	gpio_set_dir(9, GPIO_IN);
	gpio_pull_up(9);
	gpio_set_irq_enabled_with_callback(9, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
	gpio_set_irq_enabled_with_callback(9, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

	// Interrupt for ENC B
	gpio_init(10);
	gpio_set_dir(10, GPIO_IN);
	gpio_pull_up(10);
	gpio_set_irq_enabled_with_callback(10, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
	gpio_set_irq_enabled_with_callback(10, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

	// Interrupt for rotary encoder button
	gpio_init(11);
	gpio_set_dir(11, GPIO_IN);
	gpio_pull_up(11);
	gpio_set_irq_enabled_with_callback(11, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
	

	xTaskCreate(vInputHandler, "Read rotary encoder", 8192, NULL, 1, NULL);
	xTaskCreate(vDisplayTask, "Initialise and print to the display", 8192, NULL, 1, NULL);
	//xTaskCreate(vI2cScanTask, "Scan I2C Ports", 8192, NULL, 1, NULL);

	vTaskStartScheduler();
}
