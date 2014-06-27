/*
* The Artificial Intelligence Using Randomness (AIUR) is an AI for StarCraft: Broodwar, 
* aiming to be unpredictable thanks to some stochastic behaviors. 
* Please visit https://github.com/AIUR-group/AIUR for further information.
* 
* Copyright (C) 2011 - 2014 Florian Richoux
*
* This file is part of AIUR.
* AIUR is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* AIUR is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with AIUR.  If not, see http://www.gnu.org/licenses/.
*/

#include <Random.h>

Random::Random()
{
	maxInt	= 0;
	lastInt	= 0;
	epsilon = 0.005;
	randGen.seed( static_cast<unsigned int>(std::time(NULL) + _getpid() ) );
}

Random::Random( int maxInt )
{
	for( int i = 0; i < maxInt; ++i)
		distrib.push_back(1.0 / maxInt);

	this->maxInt	= maxInt;
	lastInt			= 0;
	epsilon			= 0.005;

	boost::random::discrete_distribution<>::param_type pt( distrib );
	random.param( pt );
	randGen.seed( static_cast<unsigned int>(std::time(NULL) + _getpid() ) );
}

Random::Random( int *data, int numberMoods, bool roundRobin )
{
	maxInt	= numberMoods;
	lastInt	= 0;
	epsilon = 0.005;
	distrib = std::vector<double>( numberMoods, 0 );
	randGen.seed( static_cast<unsigned int>(std::time(NULL) + _getpid() ) );
	this->setDistribution( data, numberMoods, roundRobin );
}

Random::~Random(){}

std::vector<double> Random::getDistribution()
{
	return random.probabilities();
}

void Random::setDistribution( int *data, int numberMoods, bool roundRobin )
{
	if( roundRobin )
	{
		for( int i = 1; i < numberMoods * 2 + 1; i += 2 )
		{
			if( data[i] == 1 )
				distrib.at( (i-1)/2 ) = 1;
			else
				distrib.at( (i-1)/2 ) = 0;
		}
	}
	else
	{
		std::vector<double> empirical_means;

		for( int i = 0; i < numberMoods; ++i )
			empirical_means.push_back( (double)data[i*2+1] / (data[i*2+1] + data[i*2+2]) );

		double max = *std::max_element( empirical_means.begin(), empirical_means.end() );

		for( int i = 0; i < numberMoods; ++i )
		{
			if( empirical_means.at(i) == max )
				distrib.at(i) = 1 - epsilon + (epsilon / numberMoods);
			else
				distrib.at(i) = epsilon / numberMoods;
		}

		// old way to compute the distribution

		// int numberGames = data[0];
		// double temp;
		//for( int i = 0; i < numberMoods; ++i )
		//{
		//	temp = 3*data[i*2+1] - 2*data[i*2+2];
		//	temp /= numberGames;
		//	temp += ( 1.0 / numberMoods );
		//	if( temp < 0 )
		//		temp = 0;
		//	distrib.push_back( temp );
		//}
	}

	// boost::random::discrete_distribution auto-normalizes proba
	boost::random::discrete_distribution<>::param_type pt( distrib );
	random.param( pt );
}

void Random::setMaxInt( int maxInt )
{
	this->maxInt = maxInt;
	std::vector<double>	proba;

	for( int i = 0; i < maxInt; ++i)
		proba.push_back(1.0 / maxInt);

	boost::random::discrete_distribution<>::param_type pt( proba );
	random.param( pt );
}

/**
* generate a random integer in [0, maxInt[
*/
int Random::nextInt()
{
	lastInt = random( randGen );
	return lastInt;
}

/**
* generate a random integer in [0, maxInt[
* different from lastInt
*/
int Random::nextAnotherInt()
{
	int last = lastInt;
	double backup = distrib.at( last );
	distrib.at( last ) = 0.;
	boost::random::discrete_distribution<>::param_type pt( distrib );
	random.param( pt );

	lastInt = random( randGen );

	distrib.at( last ) = backup;
	boost::random::discrete_distribution<>::param_type pt_origin( distrib );
	random.param( pt_origin );

	return lastInt;
}