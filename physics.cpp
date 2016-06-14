#include <math.h>
#include "physics.h"
#include "sensors.h"

/**
 * use Antoine's equation to compute the vapor pressure
 *
 * @param material (H2O, C2H6O)
 * @param temperature (in DegC)
 *
 * @return vapor pressure (bar)
 */
float vaporPressure(enum material stuff , short t) {
	float l = 0;
	if (stuff == H2O) {
		l = (t > Tb_H2O) ? 
			VA_H2O_B - (VB_H2O_B / (VC_H2O_B + t)) :
			VA_H2O - (VB_H2O / (VC_H2O + t));
	} else if (stuff == C2H6O) {
		l = (t > Tb_C2H6O) ? 
			VA_C2H6O_B - (VB_C2H6O_B / (VC_C2H6O_B + t)) :
			VA_C2H6O - (VB_C2H6O / (VC_C2H6O + t));
	}

	return( mm_Hg * pow(10, l));	// convert to Pascals
}

/**
 * use Hagen-Poiseuille relation to compute flow rate
 *
 * @param material (H2O, C2H6O)
 * @param pressure head (Pascals)
 * @param diameter (meters)
 * @param length (meters)
 *
 * @return flow rate (M^3/s)
 */
float flowRate(enum material stuff, float head, float diameter, float length) {
	float mu = (stuff == H2O) ? mu_H2O : mu_C2H6O;
	return (head * 3.14159 * pow(diameter,4) / (128 * mu * length));
}
