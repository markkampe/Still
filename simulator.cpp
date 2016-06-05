#include <stdio.h>
#include "sensors.h"
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

/*
 * program a pin for input or output
 *
 *	use first call to initialize sensor simulations
 */
void pinMode(int pin, int direction) {
#define ROOM_TEMP 68
#define MIN_ALC	5
	if (values[KETTLE1] == 0) {
		values[KETTLE1] = degFtoSensor(ROOM_TEMP + 1);
		values[KETTLE2] = degFtoSensor(ROOM_TEMP - 1);
		values[AMBIENT1] = degFtoSensor(ROOM_TEMP + 2);
		values[AMBIENT2] = degFtoSensor(ROOM_TEMP - 2);
		values[CONDENSOR1] = degFtoSensor(ROOM_TEMP + 0);
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
 * physics simulation parameters
 */
#define	POWER		1500	// power of the heater (Watts)
#define	SPEC_H2O	4.2	// specific heat of water (Watts/g/degC)
#define	SPEC_MALT	1.7	// specific heat of malt (Watts/g/degC)
#define BOIL_WATER	100	// boiling point of water (degC)
#define	BOIL_ETHANOL	78	// boiling point of methanol (degC)
#define	TERM_CU		400	// resistance (watt-meters/square meter degC)

#define	COND_PATH	(.03*.001)/.5	// path to condensor (meters)
#define CAP_H2O		20000	// water in kettle (grams)
#define	FRAC_MALT	0.35	// malt/H2O

// 
// add convective cooling
// add radiative cooling
// add heating of ambient air

/* 
 * increment the clock that drives the simulation
 */
void tick(int seconds) {

// FIX: just move to a watts/mass/specific heat model
// temperature model is based on heat-flow computations
// which are, in turn, based on heat flow resistance 
// and relative heat mass constants.
#define COILTEMP 1000
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
		dh_ck = (COILTEMP - k) * dTdt_K * seconds;
		// boiling carries away a lot of energy
		if (k >= degCtoSensor(BOIL_WATER))	
			dh_ck /= 2;
	}

	// compute the kettle/ambient heat flow
	int dh_ka = (k-a) * dTdt_A * seconds;

	// compute the kettle to condensor heat flow
	int dh_kc = (k-c) * dTdt_C * seconds;

#ifdef DEBUG
	printf("K=%f, A=%f, h=%d, dh_ck = %d, dh_ka=%d\n", k, a, heating, dh_ck, dh_ka);
#endif
	// update the kettle temperatures
	values[KETTLE1] += dh_ck - dh_ka;	// coil heats, ambient cools
	values[KETTLE2] += dh_ck - dh_ka;	// coil heats, ambient cools
	k = (values[KETTLE1] + values[KETTLE2])/2;

	// update the ambient temperatures
	values[AMBIENT1] += dh_ka / AoverK;	// kettle heats
	values[AMBIENT2] += dh_ka / AoverK;	// kettle heats
	a = (values[AMBIENT1] + values[AMBIENT2])/2;

	// update the condensor and alcohol sensors
	if (k >= degCtoSensor(BOIL_ETHANOL)) {
		values[CONDENSOR1] = degCtoSensor(BOIL_ETHANOL);
		values[ALCOHOL] += dAdt;
	} else {
		values[CONDENSOR1] += dh_kc;
		if (values[ALCOHOL] > MIN_ALC)
			values[ALCOHOL] -= dAdt;
	}
}
