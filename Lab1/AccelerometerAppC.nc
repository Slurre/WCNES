

configuration AccelerometerAppC { }

implementation {

	components AccelerometerC as App;
	components MainC, LedsC;
	components new ADXL345C();
	components new TimerMilliC() as TimerAccel;


	App.TimerAccel -> TimerAccel;

	/* MainC provides Boot, so we wire it to our module's Boot. */
	App.Boot -> MainC;

	/* LedsC provides Leds, so we wire it to our module's Leds. */
	App.Leds -> LedsC;

	App.Zaxis -> ADXL345C.Z;
	App.Yaxis -> ADXL345C.Y;
	App.Xaxis -> ADXL345C.X;
	App.AccelControl -> ADXL345C.SplitControl;
}
