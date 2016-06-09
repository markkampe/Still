class Still {
    public:
	/**
	 * create a still descriptor
	 *
	 * @param Volume of kettle (liters)
	 * @param Power of heater (Watts)
	 * @param Length of condensor (meters)
	 */
	Still(int, int, int);

	// basic parameters
	float	Vkettle;	// kettel volume (liters)
	float	Pheat;		// heater power (Watts)
	float	Ltubing;	// length of tubing (M)

	// assumed parameters
	float	Dtubing;	// diameter of tubing (mm)
	float	Ttubing;	// thickness of tubing (mm)
	float	Tkettle;	// thickness of the kettle (mm)

	// computed characteristics
	float	Akettle;	// kettle surface area (M^2)
	float	Atubing;	// condensor cross-sectional area (M^2)
	float	Acond;		// condensor surface area (M^2)

	float	Mcond;		// condensor mass (Kg)
	float	Mkettle;	// kettle mass (Kg)
};
