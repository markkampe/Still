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
	printf("\tHEAT[%d]: %s\n", pin, value ? "on" : "off");
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
 * @param	Still object
 * @param	Brew object
 * @param	ambient temperature (sensor units)
 * @param	relative humidity (percent)
 */
Simulator::Simulator(Still *still, Brew* brew, short Tambient, short Hambient) {
	sim = this;
	T_ambient = Tambient;
	power = still->Pheat;

	// initialize the sensor values
	values[kettle1] = Tambient + 2;
	values[kettle2] = Tambient - 2;
	values[ambient1] = Tambient + 1;
	values[ambient2] = Tambient - 1;
	values[output1] = Tambient;
	values[alcohol] = ALC_MIN;
	heating = 0.0;

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

#ifdef DEBUG
	printf("SIMULATE \ttemp=%6d S,  H2O=%d%%\n", Tambient, Hambient);
	printf("kettle   \tmass=%6.2f Kg, C=%8.2f J/degS\n", still->Mkettle, C_kettle);
	printf("condensor\tmass=%6.2f Kg, C=%8.2f J/degS\n", still->Mcond, C_condensor);
	printf("brew     \tmass=%6.2f Kg, C=%8.2f J/degS\n", M_H2O + M_malt, C_brew);
	printf("local air\tmass=%6.2f Kg, C=%8.2f J/degS\n", m_air + m_wet, C_local);
	printf("k->cond  \tarea=%6.2f M2, dQ/dt = %f W/degS\n", still->Atubing, dQ_kc);
	printf("k->air   \tarea=%6.2f M2, dQ/dt = %f W/degS\n", still->Akettle, dQ_ka);
	printf("cond->air\tarea=%6.2f M2, dQ/dt = %f W/degS\n", still->Acond, dQ_ca);
	printf("air->env \tarea=%6.2f M2, dQ/dt = %f W/degS\n", a_air, dQ_ae);
	printf("enthalpy \t%6.2f J/Kg\n", enthalpy);
#endif
}

void Simulator::heat(int percent) {
	heating = percent;
}

short Simulator::readPin(int pin) {
	return( (short) values[pin] );
}

void Simulator::simulate(int seconds) {

	// start with the current temperatures
	float k = (values[kettle1] + values[kettle2])/2;
	float a = (values[ambient1] + values[ambient2])/2;
	float c = values[output1];

	// coil to kettle heat flow based on power
	float J_ck = heating * power * seconds;

	// conductive heat transfer from kettle to condensor
	float J_kc = dQ_kc * (k - c) * seconds;

	// convective heat transfer from kettle to air
	float J_ka = dQ_ka * (k - a) * seconds;

	// convective heat transfer from condensor to air
	float J_ca = dQ_ca * (((k + c)/2) - a) * seconds;

	// conductive heat transfer from local air to the world
	float J_aa = dQ_ae * (a - T_ambient) * seconds;

	// see if we are boiling alcohol
	float J_boil = 0;
	float M_boil = 0;
	if (J_ck > 0 && k >= degCtoSensor(Tb_C2H6O-1)) {
		// excess energy goes into phase change
		J_boil = (J_ck - (J_kc + J_ka));
		M_boil = J_boil / enthalpy;
		// FIX - do real evaporation
		values[alcohol] += ALC_LEAK;
	} else {
		// FIX - do real diffusion
		if (values[alcohol] > ALC_MIN)
			values[alcohol] -= ALC_DIFFUSE;
	}
	
	// compute the resulting brew temperature
	float deltaK = (J_ck - (J_ka + J_kc + J_boil)) / C_brew;
	values[kettle1] += deltaK;
	values[kettle2] += deltaK;

	// compute the resulting air temperature
	float deltaA = (J_ka + J_ca - J_aa) / C_local;
	if ((a - deltaA) < T_ambient)
		deltaA = T_ambient - a;
	values[ambient1] += deltaA;
	values[ambient2] += deltaA;

	// compute the output temperature
	float deltaC = (J_kc + J_boil - J_ca) / C_condensor;
	if ((c + deltaC) < T_ambient)
		deltaC = T_ambient - c;
	// values[output1] += deltaC;
	// FIX - compute temperature at the end

#ifdef DEBUG
	printf("k = %f, a = %f, c = %f\n", k, a, c);
	printf("Tk + %9.2fS: J_ck = %9.2fJ, J_ka = %9.2fJ, J_kc = %9.2fJ\n", 
		deltaK, J_ck, J_ka, J_aa);
	printf("At + %9.2fS: J_ka = %9.2fJ, J_ca = %9.2fJ, J_aa = %9.2fJ\n", 
		deltaA, J_ka, J_ca, J_aa);
	printf("Ct + %9.2fS: J_kc = %9.2fJ, J_ca = %9.2fJ, J_bl = %9.2fJ\n", 
		deltaC, J_kc, J_ca, J_boil);
	if (M_boil > 0) {
		float V_boil = M_boil * 1000 / Rho_C2H6O;
		printf("distil: %8.5fKg (%8.5fL)\n", M_boil, V_boil);
	}
#endif
}
