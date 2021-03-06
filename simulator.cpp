#include <stdio.h>
#include <math.h>
#include "still.h"
#include "brew.h"
#include "sensors.h"
#include "simulator.h"
#include "physics.h"

#define DEBUG	1

// bogus alcohol sensor constants
#define	ALC_MIN		5	// minimum reading
#define ALC_LEAK	1	// leakage (ticks/min)
#define	ALC_DIFFUSE	1	// diffusion (ticks/min)

static Simulator *sim;	// static instance pointer
static long clocktime;	// seconds since start

/*
 * Arduino: program a pin for input or output
 *	(NOP in this simulation)
 */
void pinMode(int pin, int direction) { }

/*
 * Arduino: digital write
 *	the only output pins we simulate are the heater relay
 */
void digitalWrite(int pin, short value) {
	sim->heat(value);
}

/*
 * Arduino: analog read
 *	return the value from a simulated sensor
 */
short analogRead(int pin) {
	return(sim->readPin(pin));
}

long millis() {
	return(clocktime * 1000);
}

/**
 * constructor - initialize sensors, compute parameters
 *
 * @param	Still object
 * @param	Brew object
 * @param	ambient temperature (sensor units)
 * @param	relative humidity (percent)
 */
Simulator::Simulator(Still *still, Brew* brew, short Tambient, short Hambient) {
	sim = this;
	T_ambient = Tambient;
	power = still->Pheat;
	diameter = still->Dtubing;
	length = still->Ltubing;

	// initialize the sensor values
	values[kettle1] = Tambient + 2;
	values[kettle2] = Tambient - 2;
	values[ambient1] = Tambient + 1;
	values[ambient2] = Tambient - 1;
	values[output1] = Tambient;
	values[alcohol] = ALC_MIN;
	heating = 0;

	// conversion from Centigrade to sensor units
	float degSperDegC = (degCtoSensor(100) - degCtoSensor(0))/100.0;

	// specific heat of the kettle and brew
	float litersPerM3 = 1000;
	float M_H2O = brew->volume * Rho_H2O / litersPerM3;
	float M_malt = brew->Mmalt;
	C_brew = ((M_H2O * C_H2O) + (M_malt * C_malt)) / degSperDegC;

	// specific heat of the kettle and condensor
	C_condensor = (still->Mcond * C_cu) / degSperDegC;
	C_kettle = (still->Mkettle * C_cu) / degSperDegC;

#define	RADIUS	1	// assumed local air radius (M)
	// specific heat of the immediately surrounding air
	float a_air = 4 * M_PI * RADIUS * RADIUS;
	float v_air = a_air * RADIUS / 3;
	float m_air = (v_air * Rho_air);
	float m_wet = (v_air * Hambient * humidity) / 100.0;
	C_local = ((C_air * m_air) + (C_H2O * m_wet)) / degSperDegC;

	// enthalpy of boiling/condensation
	enthalpy = H_C2H6O * 1000 / gmw_C2H6O;

	// conductive heat flow resistance from kettle to condensor
	dQ_kc = k_cu * still->Atubing / still->Ltubing;

	// convective heat flow resistance from kettle to local air
	dQ_ka = h_air * still->Akettle;

	// convective heat flow resistance from condensor to local air
	dQ_ca = h_air * still->Acond;

	// conductive heat flow out resistance of the local area
	dQ_ae = k_air * a_air / RADIUS;

	// create a log file
	logfile = fopen("energy.csv", "w");
	fprintf(logfile, "# time (hhmmss),%s,%s,%s,%s,%s,%s,%s,%s\n",
		"heat->kettle", "kettle->air", "kettle->boil", "condenser->air",
		"alc->condenser", "alc->output", "H2O->condenser", "H2O output"
		);

#ifdef DEBUG
	fprintf(stdout,"SIMULATE \ttemp=%6d S,  H2O=%d%%\n", Tambient, Hambient);
	fprintf(stdout,"kettle   \tmass=%6.2f Kg, C=%8.2f J/degS\n", still->Mkettle, C_kettle);
	fprintf(stdout,"condensor\tmass=%6.2f Kg, C=%8.2f J/degS\n", still->Mcond, C_condensor);
	fprintf(stdout,"brew     \tmass=%6.2f Kg, C=%8.2f J/degS\n", M_H2O + M_malt, C_brew);
	fprintf(stdout,"local air\tmass=%6.2f Kg, C=%8.2f J/degS\n", m_air + m_wet, C_local);
	fprintf(stdout,"k->cond  \tarea=%6.2f M2, dQ/dt = %f W/degS\n", still->Atubing, dQ_kc);
	fprintf(stdout,"k->air   \tarea=%6.2f M2, dQ/dt = %f W/degS\n", still->Akettle, dQ_ka);
	fprintf(stdout,"cond->air\tarea=%6.2f M2, dQ/dt = %f W/degS\n", still->Acond, dQ_ca);
	fprintf(stdout,"air->env \tarea=%6.2f M2, dQ/dt = %f W/degS\n", a_air, dQ_ae);
	fprintf(stdout,"enthalpy \t%6.2f J/Kg\n", enthalpy);
#endif
}

/**
 * turn on the heater
 * @param percent power
 */
void Simulator::heat(int percent) {
	heating = percent;
}

/**
 * simulate the reading of an analog sensor pin
 *
 * 	values come from the simulation
 */
short Simulator::readPin(int pin) {
	return( (short) values[pin] );
}

/**
 * simulate the passage of time, at current settings
 */
void Simulator::simulate(int seconds) {

	// update the clock
	clocktime += seconds;

	// start with the current temperatures
	float k = (values[kettle1] + values[kettle2])/2;
	float a = (values[ambient1] + values[ambient2])/2;
	float c = values[output1];

	// coil to kettle heat flow based on power
	float W_ck = (heating * power)/100;

	// conductive heat transfer from kettle to condensor
	float W_kc = dQ_kc * (k - c);

	// convective heat transfer from kettle to air
	float W_ka = dQ_ka * (k - a);

	// convective heat transfer from condensor to air
	float W_ca = dQ_ca * (((k + c)/2) - a);

	// conductive heat transfer from local air to the world
	float W_aa = dQ_ae * (a - T_ambient);

	// see if we are boiling alcohol
	float W_boil = 0;
	float M_boil = 0;
	if (W_ck > 0 && k >= degCtoSensor(Tb_C2H6O-1)) {
		// excess energy goes into phase change
		W_boil = (W_ck - (W_kc + W_ka));
		M_boil = W_boil * seconds / enthalpy;
		// FIX - do real evaporation
		values[alcohol] += ALC_LEAK;
	} else {
		// FIX - do real diffusion
		if (values[alcohol] > ALC_MIN)
			values[alcohol] -= ALC_DIFFUSE;
	}
	
	// compute the updated brew temperature
	float deltaK = (W_ck - (W_ka + W_kc + W_boil)) * seconds / C_brew;
	values[kettle1] += deltaK;
	values[kettle2] += deltaK;
	k += deltaK;

	// compute the vapor presssure of water and ethanol
	float P_H2O = vaporPressure(H2O, sensorToDegC((short) k));
	float P_C2H6O = vaporPressure(C2H6O, sensorToDegC((short) k));
	// FIX Delta-P != vapor pressure

	// compute the flow through the condenser of each (ml/s)
	float flow_H2O = flowRate(H2O, P_H2O, diameter, length) * 1000000;
	float flow_C2H6O = flowRate(C2H6O, P_C2H6O, diameter, length) * 1000000;

	printf("Palc=%f, Falc=%f\n", P_C2H6O, flow_C2H6O);

	// TODO
	//	we now know the evaporation over-pressure
	//	how much phase change is there
	//		a) n = PV/RT, use dP to estimate dN
	//		b) compute the flow
	//	this amount of phase change is subtracted from kettle heat equation
	//	how much (or each) will flow out the condensor (delta P * ? * diameter/len)
	//	how much latent heat of evaporation do they carry
	//	compute convective loss from the condensor
	//	compare that with the heat carried by the flowing water/enthanol vapor
	//	figure out what fraction of the ethanol condenses
	//	figure out how much alcohol vapor escapes
	//	add that to the local 2M cube
	//	compute long term diffusion of alcohol out of the local cube

	// compute the updated air temperature
	float deltaA = (W_ka + W_ca - W_aa) * seconds / C_local;
	if ((a - deltaA) < T_ambient)
		deltaA = T_ambient - a;
	values[ambient1] += deltaA;
	values[ambient2] += deltaA;

	// compute the output temperature
	float deltaC = (W_kc + W_boil - W_ca) * seconds / C_condensor;
	if ((c + deltaC) < T_ambient)
		deltaC = T_ambient - c;
	// values[output1] += deltaC;

	float l_alc = 0;	// alcohol liquid condensed
	float l_H2O = 0;	// water liquid condensed

	if (logfile != NULL) {
		fprintf(logfile,"%02d%02d%02d,%f,%f,%f,%f,%f,%f,%f,%f\n",
			(int) clocktime/3600, (int) (clocktime/60)%60, (int) clocktime % 60,
			W_ck, W_ka, W_boil, W_ca,
			P_C2H6O, flow_C2H6O, P_H2O, flow_H2O
			);
	}
}
