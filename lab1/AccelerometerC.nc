#include "printfZ1.h"

#define INTERVAL  100
#define THRESHOLD 100

module AccelerometerC {
	uses interface Boot;
	uses interface Leds;
	uses interface Timer<TMilli> as TimerAccel;
	uses interface Read<uint16_t> as Zaxis;
	uses interface Read<uint16_t> as Yaxis;
	uses interface Read<uint16_t> as Xaxis;
	uses interface SplitControl as AccelControl;
}

implementation {
	int16_t x, y, z;

	event void Boot.booted() {
		printfz1_init();
		printfz1("  +  Booted\n");
		// start the accelerometer interface
		call AccelControl.start();
	}

	event void AccelControl.startDone(error_t err) {
		printfz1("  +  Accelerometer Started\n");
		// starting a timer with sampling rate of 10 Hz
		call TimerAccel.startPeriodic( INTERVAL );
	}

	event void AccelControl.stopDone(error_t err) {}

	event void TimerAccel.fired() {
		// every INTERVAL ms, read the X-axis first
		// the other axes are chained in the readDone callbacks
		call Xaxis.read();
	}

	/*
	 * Check if the difference between the old and new value
	 * is bigger than THRESHOLD.
	 * Additionally stores the new value for the next round.
	 */
	int checkDiff(int16_t *old_val, uint16_t new_val) {
		int res = abs(*old_val - (int16_t)new_val) > THRESHOLD;
		*old_val = (int16_t)new_val;
		return res;
	}

	event void Xaxis.readDone(error_t result, uint16_t data){
		printfz1("  +  X (%d) ", data);
		// toggle the red LED depending on the sensor reading
		if (checkDiff(&x, data)) call Leds.led0On();
		else call Leds.led0Off();
		call Yaxis.read();
	}
 
	event void Yaxis.readDone(error_t result, uint16_t data){
		printfz1(" Y (%d) ", data);
		// toggle the green LED depending on the sensor reading
		if (checkDiff(&y, data)) call Leds.led1On();
		else call Leds.led1Off();
		call Zaxis.read();
	}

	event void Zaxis.readDone(error_t result, uint16_t data){
		printfz1(" Z (%d) \n", data);
		// toggle the blue LED depending on the sensor reading
		if (checkDiff(&z, data)) call Leds.led2On();
		else call Leds.led2Off();
	}
}
