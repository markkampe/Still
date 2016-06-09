/*
 * this is a list of basic physics constants needed to do
 * temperatur computations in a still
 */
// densities
const float Rho_cu	= 8940;		// copper (kg/M^3)
const float Rho_H2O	= 1000;		// water (kg/M^3)
const float Rho_air	= 1.225;	// dry air (kg/M^3)
const float Rho_C2H6O	= 789;		// ethanol (kg/M^3)
const float humidity	= .0173;	// 100% humidity (kg/M^3)

// gram molecular weights
const float gmw_H2O	= 18.01;	// water (g/mole)
const float gmw_C2H6O	= 46.06;	// ethanol (g/mole)

// specific heats
const float C_cu	= 444;		// j/kg-DegC
const float C_H2O	= 418;		// j/kg-DegC
const float C_air	= 1005;		// j/kg-DegC
const float C_malt	= 441;		// j/kg-DegC

// thermal conductivity
const float k_cu	= 385.0;	// W-M/M^2-degC
const float k_air	= 242.8;	// W-M/M^2-degC

// convective heat transfer
const float h_air	= 20.0;		// W/M^2-degC

// boiling points
const float Tb_H2O	= 100.0;	// boiling point of water (degC)
const float Tb_C2H6O	= 78.4;		// boiling point of ethanol (degC)

// enthalpy	
const float H_H20	= 75300;	// enthalpy (J/mol)
const float H_C2H6O	= 38600;	// enthalpy (J/mol)

// useful equations (I knew this stuff would be handy someday)
//	1 watt		1 joul/second
//	mass:		density * volume
//	energy:		dQ/dt * time
//	heating:	deltaT = energy * C / mass
//	conduction:	dQ/dt = k * area * deltaT / thickness
//	convection:	dQ/dt = h * area * deltaT
