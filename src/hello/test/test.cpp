/* GTHREAD-QUEUE-PAIR - Library of Thread Queue Routines for GLIB
 * Copyright 2008  Koya Charles & Tristan Matthews 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <glib.h>
#include <iostream>
#include "baseModule.h"
#include "logWriter.h"
#include "args/optionArgs.h"

class DModule : public BaseModule
{
	bool pass;
public:
bool run();
void init(OptionArgs &);
};

void DModule::init(OptionArgs &args)
{
	pass = false;
	args.add(&pass,"pass",'p',"bypass this module");

}

bool DModule::run()
{
	if(!pass)
	while(1){ LOG("x ");	}
}

int main (int argc, char** argv) 
{ 
	DModule m;
	OptionArgs opts;

	m.init(opts);
	if(!opts.parse(argc,argv))
		return 1;

	m.run();
return 0;
}




