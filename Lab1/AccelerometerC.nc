/* WCNES, Lab 1
*/

#include "printfZ1.h"

module AccelerometerC {
	/* Our module *uses* the following interfaces. */
	uses interface Boot;
	uses interface Leds;
	uses interface Timer<TMilli> as TimerAccel;
	uses interface Read<uint16_t> as Zaxis;
	uses interface Read<uint16_t> as Yaxis;
	uses interface Read<uint16_t> as Xaxis;
	uses interface SplitControl as AccelControl;
}

implementation {
		
	int16_t oldx, oldy, oldz;
	const int threshold = 100;

	/* Gets called when the node has booted. */
	event void Boot.booted() {
		printfz1_init();	
		call AccelControl.start();
	}
	
	event void TimerAccel.fired(){
		call Xaxis.read();
	}

	event void AccelControl.startDone(error_t err){
		printfz1("Accelerometer started\n");
		call TimerAccel.startPeriodic(100);
	}

	event void AccelControl.stopDone(error_t err) {
	}


	event void Xaxis.readDone(error_t result, uint16_t data){
		if(abs((int16_t)data - oldx) > threshold)
			call Leds.led0On();
		else
			call Leds.led0Off();
		
		printfz1(" + X (%d)", data);		
		oldx = (int16_t)data;
		call Yaxis.read();
	}

	event void Yaxis.readDone(error_t result, uint16_t data){
		if(abs((int16_t)data - oldy) > threshold)
			call Leds.led1On();
		else 
			call Leds.led1Off();
		
		printfz1(" + Y (%d)", data);
		oldy = (int16_t)data;		
		call Zaxis.read();
	}

	event void Zaxis.readDone(error_t result, uint16_t data){
		if(abs((int16_t)data - oldz) > threshold)
			call Leds.led2On();
		else
			call Leds.led2Off();			
		printfz1(" + Z (%d)\n", data);
		oldz = (int16_t)data;
	}

	

	
}
