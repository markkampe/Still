#include "still.h"
#include "physics.h"
#include "math.h"

/*
 * instantiate a new still descriptor
 */
Still::Still(int capacity, int power, int length) {

	// note the basic parameters
	Vkettle = capacity / 1000.0; 	// L->M^3
	Pheat = power;
	Ltubing = length;

	// estimate the kettle surface area
	Akettle = exp( log(Vkettle) * 2 / 3 );

	// assume the copper diameter/thickness
	Dtubing = 0.010;	// 1cm diameter
	Ttubing = 0.001;	// 1mm thickness
	Tkettle = 0.001;	// 1mm thickness

	// compute the condensor size and mass
	Acond = 3.14 * Dtubing * Ltubing;	// surface area
	Atubing = 3.14 * Dtubing * Ttubing;	// cross section
	Mcond = Acond * Ttubing * Rho_cu;	// mass

	// compute kettle mass
	Mkettle = Akettle * Tkettle * Rho_cu;
}
