#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "dev/leds.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#include "config.h"

#define LED_INT_ONTIME CLOCK_SECOND*10

/* Declare two processes, the main one and an additional one
   that turns off the LEDs after a timeout */
PROCESS(basestation_process, "Shaker basestation");
PROCESS(led_process, "LED handling process");
/* Both processes should be started automatically when the node has booted. */
AUTOSTART_PROCESSES(&basestation_process, &led_process);

/* Boolean variables to remember which alarms got triggered */
static uint8_t alarm_shaker = 0, alarm_clicker = 0;

/* Callback function for received packets.
 *
 * The basestation will react to payload that contains
 * the common message ids by setting the corresponding boolean variables.
 * On unexpected payload, it will turn on LED4.
 */
static void recv(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest) {
	switch (*((uint8_t*)data)) {
		case MSG_ID_SHAKER:
			alarm_shaker = 1;
			break;
		case MSG_ID_CLICKER:
			alarm_clicker = 1;
			break;
		default:
			leds_single_on(3);
	}
	/* Poll the LED process to restart the timer. */
	process_poll(&led_process);
}

static struct etimer ledETimer;
/* This process is responsible for turning the LEDs off after a timeout,
   if no new alarm has been received. */
PROCESS_THREAD(led_process, ev, data) {
	PROCESS_BEGIN();
	while (1) {
		/* Wait until a poll or timer expired event */
		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL || etimer_expired(&ledETimer));
		if (ev == PROCESS_EVENT_POLL) {
			/* turn on correct LEDs and reset timer */
			if (alarm_shaker) leds_single_on(0);
			if (alarm_clicker) leds_single_on(1);
			if (alarm_shaker && alarm_clicker) leds_single_on(2);
			etimer_set(&ledETimer, LED_INT_ONTIME);
		}
		else {
			/* reset state and turn off all LEDs */
			alarm_shaker = 0;
			alarm_clicker = 0;
			leds_off(LEDS_ALL);
		}
	}
	PROCESS_END();
}

/* This process does nothing but initializing the LEDs and NullNet. */
PROCESS_THREAD(basestation_process, ev, data) {
	PROCESS_BEGIN();

	leds_init();

	/* Initialize NullNet */
	nullnet_set_input_callback(recv);

	PROCESS_END();
}
