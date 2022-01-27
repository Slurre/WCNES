#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "dev/leds.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

//#define LED_TIMEOUT CLOCK_SECOND*2 // for debuging
#define LED_TIMEOUT CLOCK_SECOND*10

/* Declare our "main" process, the basestation_process */
PROCESS(basestation_process, "Shaker basestation");
PROCESS(led_process, "Led timeout turnoff");

/* The basestation process should be started automatically when
 * the node has booted. */
AUTOSTART_PROCESSES(&basestation_process, &led_process);

static struct etimer led_timeout_timer;

static bool recent_acc = false;
static bool recent_btn = false;

static void recv(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest) {
    switch(*((uint8_t*)data)) {
	case 1:
	    leds_single_on(LEDS_LED1);
	    recent_acc = true;
	    break;
        case 2:
	    leds_single_on(LEDS_LED2);
	    recent_btn = true;
	    break;
    }
    
    //leds_on(LEDS_ALL);
    process_poll(&led_process);    
}

PROCESS_THREAD(led_process, ev, data) {
	PROCESS_BEGIN();
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&led_timeout_timer) || 
			ev == PROCESS_EVENT_POLL);
		if(ev == PROCESS_EVENT_POLL) {
			if(recent_acc && recent_btn) leds_single_on(LEDS_LED3);
			etimer_set(&led_timeout_timer, LED_TIMEOUT);
		}
		else {
			leds_off(LEDS_ALL);
			recent_btn = recent_acc = false;
		}
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
