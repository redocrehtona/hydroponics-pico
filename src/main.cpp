#include "pico/stdlib.h"

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "hardware/i2c.h"

void gpio_callback(uint gpio, uint32_t event_mask) {
	printf("INTERRUPT\n");
}
 
void vInputHandler(void* unused_arg) {

}

int main() {
	stdio_init_all();

	gpio_init(PICO_DEFAULT_LED_PIN);
	gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

	gpio_init(16);
	gpio_set_dir(16, GPIO_IN);

	gpio_set_irq_enabled_with_callback(16, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

	xTaskCreate(vInputHandler, "Read rotary encoder", 8192, NULL, 1, NULL);

	vTaskStartScheduler();
}
