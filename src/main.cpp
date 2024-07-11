#include "pico/stdlib.h"

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "hardware/i2c.h"

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

int main() {
	stdio_init_all();

	
	gpio_init(11);
	gpio_set_dir(11, GPIO_IN);
	gpio_pull_up(11);

	gpio_set_irq_enabled_with_callback(11, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
	

	xTaskCreate(vInputHandler, "Read rotary encoder", 8192, NULL, 1, NULL);

	vTaskStartScheduler();
}
