#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "dev/adxl345.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#include "config.h"

#define INTERVAL  CLOCK_SECOND/100
#define THRESHOLD 100

#define SEND(payload) \
	nullnet_buf = &payload; \
	NETSTACK_NETWORK.output(NULL);

/* Declare three processes, one for initialization
   and one for the each of the two sensors that are monitored */
PROCESS(startup_process, "Startup process");
PROCESS(shaker_process, "Shaker client");
PROCESS(clicker_process, "Clicker client");
/* Only the initialization process should start automatically. */
AUTOSTART_PROCESSES(&startup_process);

/* Callback function for received packets.
 *
 * Whenever this node receives a packet for its broadcast handle,
 * this function will be called.
 *
 * As the client does not need to receive, the function does not do anything
 */
static void recv(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest) {
}

/*
 * Check if the difference between the cached value
 * and the current read from the given axis
 * is bigger than THRESHOLD.
 * Additionally caches the new value for the next round.
 */
int checkDiff(int16_t *old_val, enum ADXL345_AXIS axis) {
	int16_t new_val = accm_read_axis(axis);
	int res = abs(*old_val - (int16_t)new_val) > THRESHOLD;
	*old_val = new_val;
	return res;
}

/* This process initializes the board and starts the two other processes. */
PROCESS_THREAD(startup_process, ev, data) {
	PROCESS_BEGIN();

	/* Initialize LEDs and turn on BLUE LED for debugging */
	leds_init();
	leds_on(LEDS_YELLOW); // ... which is in fact the *BLUE* LED -.-'

	/* Initialize NullNet */
	nullnet_set_input_callback(recv);
	nullnet_len = 1;

	/* start shaker and clicker processes */
	process_start(&shaker_process, NULL);
	process_start(&clicker_process, NULL);

	PROCESS_END();
}

static struct etimer et;

/* This process monitors the accelerometer in a pull-mode. */
PROCESS_THREAD(shaker_process, ev, data) {
	static uint8_t payload_shaker = MSG_ID_SHAKER;
	static int16_t x, y, z;

	PROCESS_BEGIN();

	/* Start and setup the accelerometer with default values, eg no interrupts enabled. */
	accm_init();

	/* Loop forever. */
	while (1) {
		int x_triggered = checkDiff(&x, X_AXIS);
		int y_triggered = checkDiff(&y, Y_AXIS);
		int z_triggered = checkDiff(&z, Z_AXIS);
		if (x_triggered || y_triggered || z_triggered) {
			/* If at least one of the axes observed a big change,
			   toggle the red LED on the Z1 and send a message via NullNet. */
			leds_toggle(LEDS_RED);

			SEND(payload_shaker);
		}
		/* check the accelerometer again after INTERVAL time expired */
		etimer_set(&et, INTERVAL);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	}

	PROCESS_END();
}

/* This process reacts on buttons in an interrupt-mode. */
PROCESS_THREAD(clicker_process, ev, data) {
	static uint8_t payload_clicker = MSG_ID_CLICKER;

	PROCESS_BEGIN();

	/* Activate the button sensor. */
	SENSORS_ACTIVATE(button_sensor);

	/* Loop forever. */
	while (1) {
		/* Wait until the button was pressed. */
		PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event &&
			data == &button_sensor);

		/* Toggle the green LED on the Z1 and send a message via NullNet. */
		leds_toggle(LEDS_GREEN);

		SEND(payload_clicker);
	}

	PROCESS_END();
}

