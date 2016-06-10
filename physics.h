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

enum material {H2O, C2H6O};
float vaporPressure(enum material, short temp);

// vapor pressure coefficients (Antoine equation)
const float VA_H2O	= 8.07131;
const float VA_H2O_B	= 8.14109;
const float VB_H2O	= 1730.63;
const float VB_H2O_B	= 1810.94;
const float VC_H2O	= 233.426;	// degK
const float VC_H2O_B	= 244.485;	// degK

const float VA_C2H6O	= 8.20417;
const float VA_C2H6O_B	= 7.68117;
const float VB_C2H6O	= 1642.89;
const float VB_C2H6O_B	= 1332.04;
const float VC_C2H6O	= 230.3;	// degK
const float VC_C2H6O_B	= 199.200;	// degK

// pressure conversion
const float mm_Hg = .0013332268;

// useful equations (I knew this stuff would be handy someday)
//	1 watt		1 joul/second
//	mass:		density * volume
//	energy:		dQ/dt * time
//	heating:	deltaT = energy * C / mass
//	conduction:	dQ/dt = k * area * deltaT / thickness
//	convection:	dQ/dt = h * area * deltaT
//	vapor pressure	10^(A-(B/(C+T))
//	steam flow
