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

#include "Util.h"
#include <fstream>
#include <stdarg.h>
#include <sys/stat.h>
char buffer[1024];
void log(const char* text, ...)
{
	const int BUFFER_SIZE = 1024;
	char buffer[BUFFER_SIZE];

	va_list ap;
	va_start(ap, text);
	vsnprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE, text, ap);
	va_end(ap);

	FILE *outfile;
	if (fopen_s(&outfile, "bwapi-data\\logs\\BWSAL.log", "a+")==0)
	{
		fprintf_s(outfile, buffer);
		fclose(outfile);
	}
}