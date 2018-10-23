/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY
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
#include "MolflowTypes.h"

ProfileSlice& ProfileSlice::operator+=(const ProfileSlice& rhs)
{
	this->countEquiv += rhs.countEquiv;
	this->sum_v_ort += rhs.sum_v_ort;
	this->sum_1_per_ort_velocity += rhs.sum_1_per_ort_velocity;
	return *this;
}
ProfileSlice& ProfileSlice::operator+(const ProfileSlice& rhs)
{
	*this += rhs;
	return *this;
}

TextureCell& TextureCell::operator+=(const TextureCell& rhs)
{
	this->countEquiv += rhs.countEquiv;
	this->sum_v_ort_per_area += rhs.sum_v_ort_per_area;
	this->sum_1_per_ort_velocity += rhs.sum_1_per_ort_velocity;
	return *this;
}

TextureCell& TextureCell::operator+(const TextureCell& rhs)
{
	*this += rhs;
	return *this;
}