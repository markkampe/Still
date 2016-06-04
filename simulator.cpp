#include <stdio.h>
static int heating = 0;

/* simulated pins	*/
#define	KETTLE1 1
#define KETTLE2 2
#define AMBIENT1 3
#define AMBIENT2 4
#define CONDENSOR1 5
#define ALCOHOL 6
#define RELAY 10

static float values[7];

int tempToSensor( int fahrenheit ) {
	return( 4 * fahrenheit );
}

int sensorToTemp( int sensor ) {
	return( sensor / 4 );
}

// reference temperatures (in sensor units)
#define	ROOMTEMP 100
#define COILTEMP 1000

// reference temperatures (in degF)
#define	BOIL_ALC 173
#define	BOIL_H2O 212

// alcohol levels (in sensor units)
#define MIN_ALC 5

/*
 * program a pin for input or output
 *
 *	use first call to initialize sensor simulations
 */
void pinMode(int pin, int direction) {
	if (values[KETTLE1] == 0) {
		values[KETTLE1] = tempToSensor(68);
		values[KETTLE2] = tempToSensor(69);
		values[AMBIENT1] = tempToSensor(67);
		values[AMBIENT2] = tempToSensor(70);
		values[CONDENSOR1] = tempToSensor(68);
		values[ALCOHOL] = MIN_ALC;
		heating = 0;
	}
}

/*
 * digital write
 *	the only output pin we simulate is the heater relay
 */
void digitalWrite(int pin, short value) {
	if (pin == RELAY && heating != value) {
		printf("\tHEAT: %s\n", value ? "on" : "off");
		heating = value;
	}
}


/*
 * analog read
 *	return the value from a simulated sensor
 */
short analogRead(int pin) {
	return( (short) values[pin] );
}

/* 
 * increment the clock that drives the simulation
 */
void tick(int minutes) {

// FIX: just move to a watts/mass/specific heat model
// temperature model is based on heat-flow computations
// which are, in turn, based on heat flow resistance 
// and relative heat mass constants.
#define	dTdt_K 0.1	// coil->kettle flow
#define	dTdt_A 0.05	// kettle->ambient flow
#define	dTdt_C 0.02	// kettle->condensor flow
#define	AoverK 20.0	// ambient/kettle thermal mass ratio
#define	dAdt  1.0	// rate of alcohol build-up

	// note the key environmental values
	float k = (values[KETTLE1] + values[KETTLE2])/2;
	float a = (values[AMBIENT1] + values[AMBIENT2])/2;
	float c = values[CONDENSOR1];

	// compute the coil to kettle heat flow
	int dh_ck = 0;
	if (heating) {
		dh_ck = (COILTEMP - k) * dTdt_K * minutes;
		// boiling carries away a lot of energy
		if (k >= tempToSensor(BOIL_H2O))	
			dh_ck /= 2;
	}

	// compute the kettle/ambient heat flow
	int dh_ka = (k-a) * dTdt_A * minutes;

	// compute the kettle to condensor heat flow
	int dh_kc = (k-c) * dTdt_C * minutes;

#ifdef DEBUG
	printf("K=%f, A=%f, h=%d, dh_ck = %d, dh_ka=%d\n", k, a, heating, dh_ck, dh_ka);
#endif
	// update the kettle temperatures
	values[KETTLE1] += dh_ck - dh_ka;	// coil heats, ambient cools
	values[KETTLE2] += dh_ck - dh_ka;	// coil heats, ambient cools

	// update the ambient temperatures
	values[AMBIENT1] += dh_ka / AoverK;	// kettle heats
	values[AMBIENT2] += dh_ka / AoverK;	// kettle heats

	// update the condensor temperatures
	if (k >= tempToSensor(BOIL_ALC))
		values[CONDENSOR1] = tempToSensor(BOIL_ALC);
	else
		values[CONDENSOR1] += dh_kc;

	// update the ambient alcohol level
	if (k >= tempToSensor(BOIL_ALC))
		values[ALCOHOL] += dAdt;
	else if (values[ALCOHOL] > MIN_ALC)
		values[ALCOHOL] -= dAdt;
}
