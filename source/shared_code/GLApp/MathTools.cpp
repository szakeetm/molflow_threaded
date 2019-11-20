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
#include "MathTools.h"
#include "GLTypes.h" //bool
#include <math.h>
#include <cstdio>
#include <algorithm> //std::Lower_bound
#include <sstream>
#include <iterator>
#include "Random.h"
#include <chrono>
#include <string.h> //strdup

bool IsEqual(const double &a, const double &b, double toleranceRatio) {
	return fabs(a - b) < Max(1E-99, fabs(a)*toleranceRatio);
}

size_t  IDX(int i, size_t nb) {
	return (i < 0) ? (nb + i) : (i%nb);
}

size_t IDX(size_t i, size_t nb) {
	return i%nb;
}


size_t Next(int i, size_t nb) {
	return (i+1)%nb;
}

size_t Next(size_t i, size_t nb) {
	return (i+1)%nb;
}

size_t Previous(int i, size_t nb) {
	return IDX(i - 1, nb);
}

size_t Previous(size_t i, size_t nb) {
	return IDX((int)i - 1, nb);
}

size_t GetPower2(size_t n) {
// Return a power of 2 which is greater or equal than n
  if((n & (n-1))==0) {
    // already a power of 2
    return n;
  } else {
    // Get the power of 2 above
    int p = 0;
    while(n!=0) { n = n >> 1; p++; }
    return (size_t)1 << p;
  }

}

double RoundAngle(double a) {
// Return a in [-PI,PI]
  double r=a;
  while(r<-PI) r+=2.0*PI;
  while(r> PI) r-=2.0*PI;
  return r;

}

char* FormatMemory(size_t size) {
	return FormatMemoryLL((long long)size);
}

char* FormatMemoryLL(long long size) {

	static char ret[256];
	const char *suffixStr[] = { "KB", "MB", "GB", "TB", "PB" };
	double dSize = (double)size;
	int suffix = 0;

	while (dSize >= 1024.0 && suffix < 4) {
		dSize /= 1024.0;
		suffix++;
	}

	if (suffix == 0) {
		sprintf(ret, "%u bytes", (unsigned int)size);
	}
	else {
		if (fabs(dSize - floor(dSize)) < 1e-3)
			sprintf(ret, "%.0f%s", dSize, suffixStr[suffix - 1]);
		else
			sprintf(ret, "%.2f%s", dSize, suffixStr[suffix - 1]);
	}
	return ret;

}

double my_erf(double x)
{
	// constants
	double a1 = 0.254829592;
	double a2 = -0.284496736;
	double a3 = 1.421413741;
	double a4 = -1.453152027;
	double a5 = 1.061405429;
	double p = 0.3275911;

	// Save the sign of x
	int sign = 1;
	if (x < 0)
		sign = -1;
	x = fabs(x);

	// A&S formula 7.1.26
	double t = 1.0 / (1.0 + p*x);
	double y = 1.0 - (((((a5*t + a4)*t) + a3)*t + a2)*t + a1)*t*exp(-x*x);

	return sign*y;
}

double Weigh(const double & a, const double & b, const double & weigh)
{
	return a + (b - a)*weigh;
}

double InterpolateY(const double& x, const std::vector<std::pair<double, double>>& table, const bool& logarithmic, const bool& allowExtrapolate) {
	return InterpolateXY(x, table, true, logarithmic, allowExtrapolate);
}

double InterpolateX(const double& y, const std::vector<std::pair<double, double>>& table, const bool& logarithmic, const bool& allowExtrapolate) {
	return InterpolateXY(y, table, false, logarithmic, allowExtrapolate);
}

double InterpolateVectorX(const double& y, const std::vector < std::pair<double, std::vector<double>>> & table, const size_t& elementIndex, const bool& logarithmic, const bool& allowExtrapolate) {
	//InterpolateX and InterpolateY
	//Avoids repeated code with minor changes only
	//returns double

	//firstToSecond: either unused or selector between first and second element of pairs to search for lookupValue
	//param2: either size of pointer A or index of element to search lookupValue in the second element of the pairs
	bool first = false;
	double lookupValue = y;

	if (table.size() == 1) return GetElement(table[0], !first, elementIndex);

	int lowerIndex = my_lower_bound(lookupValue, table, first, elementIndex);

	if (lowerIndex == -1) {
		lowerIndex = 0;
		if (!allowExtrapolate) return GetElement(table[lowerIndex], !first, elementIndex); //return first element
	}
	else if (lowerIndex == (table.size() - 1)) {
		if (allowExtrapolate) {
			lowerIndex = (int)table.size() - 2;
		}
		else return GetElement(table[lowerIndex], !first, elementIndex); //return last element
	}

	double delta = (logarithmic) ? log10(GetElement(table[lowerIndex + 1], first, elementIndex)) - log10(GetElement(table[lowerIndex], first, elementIndex)) : GetElement(table[lowerIndex + 1], first, elementIndex) - GetElement(table[lowerIndex], first, elementIndex);
	double overshoot = (logarithmic) ? log10(lookupValue) - log10(GetElement(table[lowerIndex], first, elementIndex)) : lookupValue - GetElement(table[lowerIndex], first, elementIndex);

	if (logarithmic) return Pow10(Weigh(log10(GetElement(table[lowerIndex], !first, elementIndex)),log10(GetElement(table[lowerIndex + 1], !first, elementIndex)),overshoot / delta)); //log-log interpolation
	else return Weigh(GetElement(table[lowerIndex], !first, elementIndex),GetElement(table[lowerIndex + 1], !first, elementIndex),overshoot / delta);
}

std::vector<double> InterpolateVectorY(const double& x, const std::vector<std::pair<double, std::vector<double>>>& table, const bool& logarithmic, const bool& allowExtrapolate) {
	//Same as InterpolateY but returns a vector.
	//Must repeat most of code because C++ doesn't allow runtime evaluated return-type (and only 'bool first' decides what to return)
	if (table.size() == 1) return table[0].second;

	int lowerIndex = my_lower_bound(x, table, true, 0);

	if (lowerIndex == -1) {
		lowerIndex = 0;
		if (!allowExtrapolate) return table[lowerIndex].second; //return first element
	}
	else if (lowerIndex == (table.size() - 1)) {
		if (allowExtrapolate) {
			lowerIndex = (int)table.size() - 2;
		}
		else return table[lowerIndex].second; //return last element
	}

	double delta = (logarithmic) ? log10(table[lowerIndex + 1].first) - log10(table[lowerIndex].first) : table[lowerIndex + 1].first - table[lowerIndex].first;
	double overshoot = (logarithmic) ? log10(x) - log10(table[lowerIndex].first) : x - table[lowerIndex].first;

	size_t distrYsize = table[0].second.size();
	std::vector<double> result; result.resize(distrYsize);
	for (size_t e = 0; e < distrYsize; e++)
	{
		if (logarithmic) result[e]=Pow10(Weigh(log10(table[lowerIndex].second[e]),log10(table[lowerIndex + 1].second[e]), overshoot / delta)); //log-log interpolation
		else result[e]=Weigh(table[lowerIndex].second[e],table[lowerIndex + 1].second[e],overshoot / delta);
	}
	return result;
}




/*
double FastLookupY(const double& x, const std::vector<std::pair<double, double>>& table, const bool& limitToBounds) {
	//Function inspired by http://stackoverflow.com/questions/11396860/better-way-than-if-else-if-else-for-linear-interpolation
	_ASSERTE(table.size());
	if (table.size() == 1) return table[0].second; //constant value

												   // Assumes that table .first is SORTED AND EQUIDISTANT
												   // Check if x is out of bound
	std::vector<std::pair<double, double> >::const_iterator lower, upper;
	bool outOfLimits = false;

	if (x >= table.back().first) {
		if (limitToBounds) return table.back().second;
		else {
			outOfLimits = true;
			lower = upper = table.end() - 1;
			lower--;
		}
	}
	else if (x < table[0].first) {
		if (limitToBounds) return table[0].second;
		else {
			outOfLimits = true;
			lower = upper = table.begin();
			upper++;
		}
	}

	if (!outOfLimits) {
		double distanceX = table[1].first - table[0].first;
		size_t lowerIndex = (int)((x - table[0].first) / distanceX);
		lower = upper = table.begin() + (lowerIndex + 1);
		// Corner case
		if (upper == table.begin()) return upper->second;
		lower--;
	}
	double result = lower->second + (upper->second - lower->second)*(x - lower->first) / (upper->first - lower->first);
	return result;
}
*/

std::vector<std::string> SplitString(std::string const &input) {
	//Split string by whitespaces
	std::istringstream buffer(input);
	std::vector<std::string> ret;

	std::copy(std::istream_iterator<std::string>(buffer),
		std::istream_iterator<std::string>(),
		std::back_inserter(ret));
	return ret;
}

std::vector<std::string> SplitString(std::string const & input, const char & delimiter)
{
		std::vector<std::string> result;
		const char* str = strdup(input.c_str());
		do
		{
			const char *begin = str;
			while (*str != delimiter && *str)
				str++;

			result.push_back(std::string(begin, str));
		} while (0 != *str++);
		return result;
}

bool endsWith(std::string const &fullString, std::string const &ending) {
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	}
	else {
		return false;
	}
}

bool beginsWith(std::string const &fullString, std::string const &beginning) {
	return (fullString.compare(0, beginning.length(), beginning) == 0);
}

std::string space2underscore(std::string text) {
	for (std::string::iterator it = text.begin(); it != text.end(); ++it) {
		if (*it == ' ') {
			*it = '_';
		}
	}
	return text;
}

bool iequals(std::string a, std::string b)
{
	return std::equal(a.begin(), a.end(),
		b.begin(), b.end(),
		[](char a, char b) {
		return tolower(a) == tolower(b);
	});
}

int my_lower_bound(const double & key, double* A,const size_t& size)
//"iterative" version of algorithm, modified from https://en.wikipedia.org/wiki/Binary_search_algorithm
//key: searched value
//A: the lookup table
//first: either unused or selector between first and second element of pairs
//returns index of last lower value, or -1 if key not found

// GetElement: chooses first or second member of a pair (avoids writing this function twice)

{
	int l = 0;
	int h = (int)size; // Not n - 1
	while (l < h) {
		int mid = (l + h) / 2;
		if (key <= A[mid]) {
			h = mid;
		}
		else {
			l = mid + 1;
		}
	}
	return l - 1;
}

int my_lower_bound(const double & key, const std::vector<double>& A)
//"iterative" version of algorithm, modified from https://en.wikipedia.org/wiki/Binary_search_algorithm
//key: searched value
//A: the lookup table
//first: either unused or selector between first and second element of pairs
//returns index of last lower value, or -1 if key not found

// GetElement: chooses first or second member of a pair (avoids writing this function twice)

{
	int l = 0;
	int h = (int)A.size(); // Not n - 1
	while (l < h) {
		int mid = (l + h) / 2;
		if (key <= A[mid]) {
			h = mid;
		}
		else {
			l = mid + 1;
		}
	}
	return l - 1;
}

int my_lower_bound(const double & key, const std::vector<std::pair<double, double>>& A, const bool & first)
//"iterative" version of algorithm, modified from https://en.wikipedia.org/wiki/Binary_search_algorithm
//key: searched value
//A: the lookup table
//first: either unused or selector between first and second element of pairs
//returns index of last lower value, or -1 if key not found

// GetElement: chooses first or second member of a pair (avoids writing this function twice)

{
	int l = 0;
	int h = (int)A.size(); // Not n - 1
	while (l < h) {
		int mid = (l + h) / 2;
		if (key <= GetElement(A[mid], first)) {
			h = mid;
		}
		else {
			l = mid + 1;
		}
	}
	return l - 1;
}

int my_lower_bound(const double & key, const std::vector<std::pair<double, std::vector<double>>>& A, const bool & first, const size_t & elementIndex)
//"iterative" version of algorithm, modified from https://en.wikipedia.org/wiki/Binary_search_algorithm
//key: searched value
//A: the lookup table
//first: either unused or selector between first and second element of pairs
//returns index of last lower value, or -1 if key not found

// GetElement: chooses first or second member of a pair (avoids writing this function twice)

{
	int l = 0;
	int h = (int)A.size(); // Not n - 1
	while (l < h) {
		int mid = (l + h) / 2;
		if (key <= GetElement(A[mid],first,elementIndex)) {
			h = mid;
		}
		else {
			l = mid + 1;
		}
	}
	return l-1;
}

double InterpolateXY(const double & lookupValue, const std::vector<std::pair<double, double>>& table, const bool & first, const bool & logarithmic, const bool & allowExtrapolate) {
	//InterpolateX and InterpolateY
	//Avoids repeated code with minor changes only
	//returns double

	//firstToSecond: either unused or selector between first and second element of pairs to search for lookupValue
	//param2: either size of pointer A or index of element to search lookupValue in the second element of the pairs

	if (table.size() == 1) return GetElement(table[0], !first);

	int lowerIndex = my_lower_bound(lookupValue, table, first);

	if (lowerIndex == -1) {
		lowerIndex = 0;
		if (!allowExtrapolate) return GetElement(table[lowerIndex], !first); //return first element
	}
	else if (lowerIndex == (table.size() - 1)) {
		if (allowExtrapolate) {
			lowerIndex = (int)table.size() - 2;
		}
		else return GetElement(table[lowerIndex], !first); //return last element
	}

	double delta = (logarithmic) ? log10(GetElement(table[lowerIndex + 1], first)) - log10(GetElement(table[lowerIndex], first)) : GetElement(table[lowerIndex + 1], first) - GetElement(table[lowerIndex], first);
	double overshoot = (logarithmic) ? log10(lookupValue) - log10(GetElement(table[lowerIndex], first)) : lookupValue - GetElement(table[lowerIndex], first);

	if (logarithmic) return Pow10(Weigh(log10(GetElement(table[lowerIndex], !first)),log10(GetElement(table[lowerIndex + 1], !first)),overshoot / delta)); //log-log interpolation
	else return Weigh(GetElement(table[lowerIndex], !first),GetElement(table[lowerIndex + 1], !first),overshoot / delta);

}

double QuadraticInterpolateX(const double& y,
	const double& a, const double& b, const double& c,
	const double& FA, const double& FB, const double& FC, MersenneTwister& randomGenerator) {
	double amb = a - b;
	double amc = a - c;
	double bmc = b - c;
	double amb_amc = amb*amc;
	double amc_bmc = amc*bmc;
	double divisor = (2 * (-(FA / (amb_amc)) + FB / (amb_amc)+FB / (amc_bmc)-FC / (amc_bmc)));
	                 
	if (fabs(divisor) < 1e-30) {
		//Divisor is 0 when the slope is 1st order (a straight line) i.e. (FC-FB)/(c-b) == (FB-FA)/(b-a)
		if ((FB - FA) < 1e-30) {
			//FA==FB, shouldn't happen
			return a + randomGenerator.rnd()*(b - a);
		}
		else {
			//Inverse linear interpolation
			return a + (y - FA) * (b - a)/ (FB - FA);
		}
	}
	else {
		//(reverse interpolate y on a 2nd order polynomial fitted on {a,FA},{b,FB},{c,FC} where FA<y<FB):
		//Root of Lagrangian polynomials, solved by Mathematica
		return (FA / (amb)-(a*FA) / (amb_amc)-(b*FA) / (amb_amc)-FB / (amb)+(a*FB) / (amb_amc)+(b*FB) / (amb_amc)+(a*FB) / (amc_bmc)+(b*	FB) / (amc_bmc)-(a*FC) / (amc_bmc)
			-(b*FC) / (amc_bmc)-sqrt(Sqr(-(FA / (amb)) + (a*FA) / (amb_amc)+(b*FA) / (amb_amc)+FB / (amb)-(a*FB) / (amb_amc)-(b*FB) / (amb_amc)-(a*FB) / (amc_bmc)-(b*FB)
				/ (amc_bmc)+(a*FC) / (amc_bmc)+(b*FC) / (amc_bmc)) - 4 * (-(FA / (amb_amc)) + FB / (amb_amc)+FB / (amc_bmc)-FC / (amc_bmc))*(-FA + (a*FA) / (amb)-(a*b*FA)
					/ (amb_amc)-(a*FB) / (amb)+(a*b*FB) / (amb_amc)+(a*b*FB) / (amc_bmc)-(a*b*FC) / (amc_bmc)+y))) / divisor;
	}
}

int weighed_lower_bound_X(const double & key, const double & weigh, double * A, double * B, const size_t & size)
{
	//interpolates among two lines of a cumulative distribution
	//all elements of line 1 and line 2 must be monotonously increasing (except equal consecutive values)
	//key: lookup value
	//weigh: between 0 and 1 (0: only first distribution, 1: only second distribution)
	//A* and B* : pointers to arrays of 'size' number of CDF values. The first value (not included) is assumed to be 0, the last (not included) is assumed to be 1
	//return value: lower index. If -1, then key is smaller than first element, if 'size-1', then key is larger than last element
	
		if (size == 0) return -1;
		if (size == 1) {
			double weighed = Weigh(A[0], B[0], weigh);
			if (key < weighed) return -1;
			else return 0;
		}
		int L = 0;
		int R = (int)(size - 1);
		// continue searching while [imin,imax] is not empty
		int M; double weighed,nextWeighed;
		while (L<=R)
		{
			M = (L + R) / 2;
			weighed = Weigh(A[M], B[M], weigh);
			nextWeighed = Weigh(A[M + 1], B[M + 1], weigh);
			if (weighed <= key && key < nextWeighed) {
				// key found at index M
				return M;
			}
			else if (weighed < key) {
				L = M + 1;
			}
			else  {
				R = M - 1;
			}
		}
		//Not found
		if (M == 0)
			return -1; //key lower than first element
		else
			return (int)size - 1; //key larger than last element
	
}

double GetElement(const std::pair<double, double>& pair, const bool & first) {
	return first ? pair.first : pair.second;
}

double GetElement(const std::pair<double, std::vector<double>>& pair, const bool & first, const size_t & elementIndex) {
	return first ? pair.first : pair.second[elementIndex];
}

size_t GetSysTimeMs()
{
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
		);
	return ms.count();
}

double Pow10(const double& a) {
	return pow(10,a);
}
/*
int isinf(double x)
{
#if defined _MSC_VER
	typedef unsigned __int64 uint64;
#else
	typedef uint64_t uint64;
#endif
	union { uint64 u; double f; } ieee754;
	ieee754.f = x;
	return ((unsigned)(ieee754.u >> 32) & 0x7fffffff) == 0x7ff00000 &&
		((unsigned)ieee754.u == 0);
}

int isnan(double x)
{
#if defined _MSC_VER
	typedef unsigned __int64 uint64;
#else
	typedef uint64_t uint64;
#endif
	union { uint64 u; double f; } ieee754;
	ieee754.f = x;
	return ((unsigned)(ieee754.u >> 32) & 0x7fffffff) +
		((unsigned)ieee754.u != 0) > 0x7ff00000;
}*/