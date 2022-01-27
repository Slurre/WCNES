#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/adxl345.h"
#include "dev/leds.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#define ACCM_READ_INTERVAL    CLOCK_SECOND/100
#define LED_INT_ONTIME        CLOCK_SECOND/2
#define MOVE_THRESHOLD 100

PROCESS(init_process, "Main proc");
PROCESS(shaker_process, "Shaker proc");
PROCESS(btn_process, "Button proc");

AUTOSTART_PROCESSES(&init_process);

static void recv(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest) {
}

static struct etimer et_acc_read; // controls read freq. for acc-meter

/* Our main process, initializes devices */
PROCESS_THREAD(init_process, ev, data) {
	PROCESS_BEGIN();

	//SENSORS_ACTIVATE(adxl345);
	SENSORS_ACTIVATE(button_sensor);
	
	/* Activate accelerometer */
	accm_init();

	/* Activate leds */
	leds_init();

	/* Initialize NullNet */
	//nullnet_buf = (uint8_t *)&payload_acc;
	//nullnet_len = sizeof(payload_acc);
	nullnet_set_input_callback(recv);
	
	process_start(&btn_process, NULL);
	process_start(&shaker_process, NULL);
	
	PROCESS_END();
}

PROCESS_THREAD(shaker_process, ev, data) {
	static int16_t x, oldx = 0;
	static uint8_t msg_acc = 1;

	PROCESS_BEGIN();
	
	/* Loop forever. */
	while (1) {
		x = accm_read_axis(X_AXIS);

		if(abs(x - oldx) > MOVE_THRESHOLD) {
			leds_toggle(LEDS_RED);
			
			nullnet_buf = &msg_acc;
    			nullnet_len = 1;

			/* Send the content of the packet buffer using the
			 * broadcast handle. */
			NETSTACK_NETWORK.output(NULL);
		}
		oldx = x;
		etimer_set(&et_acc_read, ACCM_READ_INTERVAL);
      		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_acc_read));
	}

	PROCESS_END();	
}

PROCESS_THREAD(btn_process, ev, data) {	

	static uint8_t msg_btn = 2;

	PROCESS_BEGIN();
	
	/* Loop forever. */
	while (1) {
		PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event &&
			data == &button_sensor);

		leds_toggle(LEDS_GREEN);

		nullnet_buf = &msg_btn;
    		nullnet_len = 1;

		/* Send the content of the packet buffer using the
		 * broadcast handle. */
		NETSTACK_NETWORK.output(NULL);
	}

	PROCESS_END();	
}

