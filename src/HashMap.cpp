/*
* The Artificial Intelligence Using Randomness (AIUR) is an AI for StarCraft: Broodwar, 
* aiming to be unpredictable thanks to some stochastic behaviors. 
* Please visit http://code.google.com/p/aiurproject/ for further information.
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

#include <HashMap.h>

using namespace std;

HashMap::HashMap()
{
	hashMap.insert(make_pair("Benzene",				"af618ea3ed8a8926ca7b17619eebcb9126f0d8b1"));
	hashMap.insert(make_pair("Destination",			"4e24f217d2fe4dbfa6799bc57f74d8dc939d425b"));
	hashMap.insert(make_pair("Heartbreak Ridge",	"6f8da3c3cc8d08d9cf882700efa049280aedca8c"));
	hashMap.insert(make_pair("Aztec",				"ba2fc0ed637e4ec91cc70424335b3c13e131b75a"));
	hashMap.insert(make_pair("Tau Cross",			"9bfc271360fa5bab3707a29e1326b84d0ff58911"));
	hashMap.insert(make_pair("Andromeda",			"1e983eb6bcfa02ef7d75bd572cb59ad3aab49285"));
	hashMap.insert(make_pair("Circuit Breaker",		"450a792de0e544b51af5de578061cb8a2f020f32"));
	hashMap.insert(make_pair("Empire of the Sun",	"a220d93efdf05a439b83546a579953c63c863ca7"));
	hashMap.insert(make_pair("Fortress",			"83320e505f35c65324e93510ce2eafbaa71c9aa1"));
	hashMap.insert(make_pair("Python",				"de2ada75fbc741cfa261ee467bf6416b10f9e301"));
}

const string HashMap::hash(const string s)
{
	return hashMap[s];
}