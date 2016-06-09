class Brew {
    public:
    	float	volume;		// water volume (liters)
	float	Mmalt;		// malt (Kg)

	/**
	 * brew parameters
	 *
	 * @param	volume of water (liters)
	 * @param	mass of malt (Kg)
	 */
	Brew( int vol, int mass ) {
		volume = vol;
		Mmalt = mass;
	}
};
