#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "dev/leds.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#define LED_TIMEOUT CLOCK_SECOND

/* Declare our "main" process, the basestation_process */
PROCESS(basestation_process, "Shaker basestation");
PROCESS(led_process, "Led timeout turnoff");

/* The basestation process should be started automatically when
 * the node has booted. */
AUTOSTART_PROCESSES(&basestation_process, &led_process);

static struct etimer led_timeout_timer;

/* Holds the number of packets received. */
static int count = 0;

/* Callback function for received packets.
 *
 * Whenever this node receives a packet for its broadcast handle,
 * this function will be called.
 *
 * As the client does not need to receive, the function does not do anything
 */
static void recv(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest) {
    count++;
    /* 0bxxxxx allows us to write binary values */
    /* for example, 0b10 is 2 */
    
    leds_on(LEDS_ALL);
    process_poll(&led_process);    
}

PROCESS_THREAD(led_process, ev, data) {
	PROCESS_BEGIN();
	while(1) {
		PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
		etimer_set(&led_timeout_timer, LED_TIMEOUT);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&led_timeout_timer));
 		leds_off(LEDS_ALL);
	}
	PROCESS_END();
}

/* Our main process. */
PROCESS_THREAD(basestation_process, ev, data) {
	PROCESS_BEGIN();
	leds_off(LEDS_ALL);
	/* Initialize NullNet */
	nullnet_set_input_callback(recv);

	PROCESS_END();
}
