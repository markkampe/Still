#include <stdio.h>
#include <math.h>
#include "sensors.h"
#include "simulator.h"

//define DEBUG	1

static Simulator *sim;	// static instance pointer

/*
 * Arduino: program a pin for input or output
 */
void pinMode(int pin, int direction) { }

/*
 * Arduino: digital write
 *	the only output pin we simulate is the heater relay
 */
void digitalWrite(int pin, short value) {
	printf("\tHEAT: %s\n", value ? "on" : "off");
	sim->heat(value);
}


/*
 * Arduino: analog read
 *	return the value from a simulated sensor
 */
short analogRead(int pin) {
	return(sim->readPin(pin));
}

/**
 * constructor - initialize sensors, compute parameters
 *
 * @param	heater power (watts)
 * @param	kettle volume (ml)
 * @param	ambient temperature (sensor units)
 */
Simulator::Simulator(int watts, int ml, short degS) {
	sim = this;
	power = watts;
	ambient = degS;
	

	// initialize the sensor values
	values[kettle1] = degS + 2;
	values[kettle2] = degS - 2;
	values[ambient1] = degS + 1;
	values[ambient2] = degS - 1;
	values[condensor1] = degS;
#define	MIN_ALC		5	// minimum likely reading
	values[alcohol] = MIN_ALC;
	heating = 0;

	// compute the rate of kettle heating per watt
#define	RHO_H2O		1.0	// density of water (grams/ml)
#define	FRAC_MALT	0.35	// malt/H2O ratio (by weight)
#define	SPEC_H2O	4.2	// specific heat of water (Watts/g/degC)
#define	SPEC_MALT	1.7	// specific heat of malt (Watts/g/degC)
	float mass = ml * (RHO_H2O  + (RHO_H2O * FRAC_MALT));
	float specHeat = SPEC_H2O + (FRAC_MALT * SPEC_MALT);
	float wattsPerDegC = mass * specHeat;
	float degSperDegC = (degCtoSensor(100) - degCtoSensor(0))/100.0;
	C_heating = wattsPerDegC / degSperDegC;

	// compute the convective heat loss per degree
#define	CONV_AIR	.002	// convective air transfer (W/cm^2 degC)
	// assume kettle volume is twice the liquid volume, area = vol^2/3:x
	float kettleArea = exp( log(2 * ml) * 2 / 3 );
	C_cooling = kettleArea * CONV_AIR / degSperDegC;

	// I consider the heat to flow into a 2M cube around the still
#define	RHO_AIR		.001225	// density of air (grams/cc)
#define SPEC_AIR	4.2	// specific heat (watts/g/degC)
	float airVol = 2.0E2 * 2.0E2 * 2.0E2;
	float airMass = airVol * RHO_AIR;
	C_air = airMass * SPEC_AIR / degSperDegC;

	// the 2M cube conducts heat (through air) into an infinite ambient sink
#define	COND_AIR_AIR	.01	// thermal conductivity (watts/m-degC)
	float area = 5 * 2.0E2;
	C_drain = area * COND_AIR_AIR / degSperDegC;
	
#ifdef DEBUG
	printf("mass = %f, specHeat = %f\n", mass, specHeat);
	printf("W/degC = %f, degS/degC=%f, W/degS=%f\n", wattsPerDegC, degSperDegC, C_heating);

	printf("area = %f cm^2, W/degS = %f\n", kettleArea, C_cooling);

	printf("air mass = %fg, W/degS = %f\n", airMass, C_air);

	printf("area = %fcm^2, W/degS = %f\n", area, C_drain);
#endif
}

void Simulator::heat(bool on) {
	heating = on;
}

short Simulator::readPin(int pin) {
	return( (short) values[pin] );
}

void Simulator::simulate(int seconds) {
/*
 * physics simulation parameters
 */
#define BOIL_WATER	100	// boiling point of water (degC)
#define	BOIL_ETHANOL	78	// boiling point of methanol (degC)


	// start with the current temperatures
	float k = (values[kettle1] + values[kettle2])/2;
	float a = (values[ambient1] + values[ambient2])/2;
	float c = values[condensor1];

	// compute the coil to kettle heat flow
	float W_ck = heating ? power : 0;
	if (k >= degCtoSensor(BOIL_WATER))	
		W_ck /= 2;

	// compute the kettle to air heat flow
	float W_ka = C_cooling * (k - a);

	// compute the air-to-air heat flow
	float W_aa = (a - ambient) * C_drain;

#ifdef DEBUG
	printf("W_ck = %f, W_ka = %f, w_aa = %f\n", W_ck, W_ka, W_aa);
#endif

	// compute the resulting temperature changes
	float deltaK = (W_ck - W_ka) * seconds / C_heating;
	values[kettle1] += deltaK;
	values[kettle2] += deltaK;

	// air is heated by kettle, cooled by surrounding air
	float deltaA = (W_ka - W_aa) * seconds / C_air;
	values[ambient1] += deltaA;
	values[ambient2] += deltaA;

	// FIX: model conductive heating of condensor before boil
	// FIX: model continuous convective cooling of condensor
#define	TERM_CU		400	// resistance (watt-meters/square meter degC)
#define	COND_PATH	(.03*.001)/.5	// path to condensor (meters)
	// update the condensor and alcohol sensors
	if (k >= degCtoSensor(BOIL_ETHANOL)) {
		values[condensor1] = degCtoSensor(BOIL_ETHANOL);
		values[alcohol] += 1;		// FIX
	} else {
		values[condensor1] += 1;	// FIX
		if (values[alcohol] > MIN_ALC)
			values[alcohol] -= 1;	// FIX
	}
}
