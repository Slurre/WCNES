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

/* Declare our "main" process, the client process*/
PROCESS(client_process, "Shaker client");
/* The client process should be started automatically when
 * the node has booted. */
AUTOSTART_PROCESSES(&client_process);

//static struct etimer ledETimer;
static struct etimer et_acc_read;

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

void accm_shaken_cb(uint8_t reg){
	//leds_toggle(LEDS_RED);
	return;
}

/* Our main process. */
PROCESS_THREAD(client_process, ev, data) {
	int16_t x, oldx = 0;
	int16_t threshold = 100;
	static char payload[] = "hej";

	PROCESS_BEGIN();

	SENSORS_ACTIVATE(adxl345);
	
	/* Activate accelerometer */
	accm_init();

	/* Activate leds */
	leds_init();

	/* Register the callback function for the interrupt */
    	ACCM_REGISTER_INT1_CB(accm_shaken_cb);
	//ACCM_REGISTER_INT2_CB(accm_shaken_cb);
	accm_set_irq(ADXL345_INT_TAP, ADXL345_INT_DISABLE);

	/* Initialize NullNet */
	nullnet_buf = (uint8_t *)&payload;
	nullnet_len = sizeof(payload);
	nullnet_set_input_callback(recv);

	/* Loop forever. */
	while (1) {
		x = accm_read_axis(X_AXIS);

		if(abs(x - oldx) > threshold) {
			leds_toggle(LEDS_RED);
			
			/* Copy the string "hej" into the packet buffer. */
			memcpy(nullnet_buf, &payload, sizeof(payload));
	    		nullnet_len = sizeof(payload);

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
