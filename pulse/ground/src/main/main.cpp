// headerGPL.c
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

/** \file
 *      Just the License GPL 3+
 *
 *      Detailed description here.
 *      Continues here.
 *      And more.
 *      And more.
 */
#include "config.h"
#include <glib.h>
#include <vector>

class OptionArgs
{
	typedef std::vector<GOptionEntry> Options;
	Options options;
public:
	void add(GOptionEntry& o){

		options.push_back(o);
	}

	GOptionEntry* getArray()
	{
		GOptionEntry *pA;
		static GOptionEntry n = { NULL };
		int count = 0;
		if(options.empty())
			return 0;

		pA = new GOptionEntry[options.size()+1];
		for(Options::iterator it= options.begin(); it != options.end(); ++it)
		{
			pA[count++] = *it;
		}
		pA[count] =  n ;

		return pA;
	}

};

int main (int argc, char **argv)
{
	GError *error = NULL;
	GOptionContext *context;
	GOptionEntry *pGOptions;
	OptionArgs options;

	int result;
	GOptionEntry o = { "try", 't', 0, G_OPTION_ARG_INT, &result,"try it out", "arg desc" };
	options.add(o);

	context = g_option_context_new (PACKAGE_VERSION PACKAGE);

	pGOptions = options.getArray();
	g_option_context_add_main_entries(context, pGOptions,NULL);

	if (!g_option_context_parse (context, &argc, &argv, &error)) {
		g_print ("option parsing failed: %s\n", error->message);
		exit (1);
	}



}



