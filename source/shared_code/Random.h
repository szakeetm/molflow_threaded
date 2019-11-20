/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY / Pascal BAEHR
Copyright:   E.S.R.F / CERN
Website:     https://cern.ch/molflow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Full license text: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
*/
#pragma once

#include "TruncatedGaussian/rtnorm.hpp"

/* Maximum generated random value */
#define RK_STATE_LEN 624
#define RK_MAX 0xFFFFFFFFUL
/* Magic Mersenne Twister constants */
#define MERSENNE_N 624
#define MERSENNE_M 397
#define MATRIX_A 0x9908b0dfUL
#define UPPER_MASK 0x80000000UL
#define LOWER_MASK 0x7fffffffUL

#ifdef WIN
// Disable "unary minus operator applied to unsigned type, result still unsigned" warning.
#pragma warning(disable : 4146)
#endif

class MersenneTwister {	
	
/* State of the RNG */
	struct rk_state
	{
		unsigned long key[RK_STATE_LEN];
		int pos;
	};

public:
	MersenneTwister();
	void   SetSeed(unsigned long seed); // Initialise the random generator with the specified seed
	double rnd(); // Returns a uniform distributed double value in the interval ]0,1[

	double Gaussian(const double &sigma);

	unsigned long GetSeed();

private:
	rk_state localState;

	/* Slightly optimised reference implementation of the Mersenne Twister */
	unsigned long rk_random();

	double rk_double();
};

class TruncatedGaussian {
public:
	double GetGaussian(gsl_rng *gen, const double &mean, const double &sigma, const double &lowerBound, const double &upperBound);
};

