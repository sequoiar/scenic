// pyInterpreter.h
// Copyright 2008 Koya Charles & Tristan Matthews
//
// This file is part of [propulse]ART.
//
// [propulse]ART is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// [propulse]ART is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
//
//

/** \file
 *      Main python loop
 *
 *      _ground_loop() called on every return\n
 *      See python_import.cpp
 */

#include "pyInterpreter.h"

#include <iostream>

int ground_init(int argc, char *argv[]);
int ground_loop(int result);

int main(int argc, char *argv[])
{
	static int count;
	std::string result;
	pyInterpreter py;

	py.init(argc, argv);

	ground_init(argc, argv);

	std::cout << std::endl << "Welcome to the Console" << std::endl;
	std::cout << "Brought to you by The Hawfullpuf Industrial Complex." << std::endl;
	while (1)
	{
		if (ground_loop(count)) {
			if (!py.run_input().empty())
				count = -1;
		}
		else
			break;
	}

	std::cout << std::endl << "going down - CLEANUP ----" << std::endl;
}
