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

#include "MesurePrecision.h"

CMesurePrecision::CMesurePrecision()
{
	//strcpy(lastError,"Pas d'erreur");
}

CMesurePrecision::~CMesurePrecision(){}

bool CMesurePrecision::Start()
{
	if (!QueryPerformanceFrequency(&frequence))
	{
		LPVOID lpMsgBuf; 
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS, NULL, 
			::GetLastError(), 0, (LPTSTR) &lpMsgBuf, 0, NULL); 
		//strcpy(lastError,(LPCSTR)lpMsgBuf);
		LocalFree( lpMsgBuf );
		return false;

	}
	if(!QueryPerformanceCounter (&debut))
	{
		LPVOID lpMsgBuf; 
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS, 
			NULL, ::GetLastError(), 0, (LPTSTR) &lpMsgBuf, 0, NULL); 
		//strcpy(lastError,(LPCSTR)lpMsgBuf);   
		LocalFree( lpMsgBuf );    
		return false;
	}
	//strcpy(lastError,"Pas d'erreur");
	return true;
}

double CMesurePrecision::GetTimeFromStart()
{
	if (!QueryPerformanceCounter (&fin))
	{

		LPVOID lpMsgBuf; 
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS, NULL, 
			::GetLastError(), 0, (LPTSTR) &lpMsgBuf, 0, NULL);
		//strcpy(lastError,(LPCSTR)lpMsgBuf);
		LocalFree( lpMsgBuf );
		return 0;

	}
	//strcpy(lastError,"Pas d'erreur");
	return ((double)((__int64)fin.QuadPart)-((__int64)debut.QuadPart)) / 
		(double)frequence.QuadPart;
}

char * CMesurePrecision::GetLastError()
{
	return lastError;
}