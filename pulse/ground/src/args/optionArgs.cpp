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
#include "optionArgs.h"
#include <glib.h>

OptionArgs::~OptionArgs()
{
	for(; str_dump.begin() != str_dump.end();)
	{
		free(**str_dump.begin());
		str_dump.erase(str_dump.begin());
	}

}
void OptionArgs::add(bool* ret, const char* l_flag, char s_flag, const char* desc)
{
	GOptionEntry e = {l_flag,s_flag,0,G_OPTION_ARG_NONE,ret,desc};
	options.push_back(e);

}

void OptionArgs::add(char** ret, const char* l_flag, char s_flag, const char* desc, const char* arg_desc)
{
	GOptionEntry e = {l_flag,s_flag,0,G_OPTION_ARG_STRING,ret,desc,arg_desc};
	str_dump.push_back(ret);
	options.push_back(e);
}

void OptionArgs::add(int * ret,const char* l_flag, char s_flag,  const char* desc, const char* arg_desc)
{
	GOptionEntry e = {l_flag,s_flag,0,G_OPTION_ARG_INT,ret,desc,arg_desc};
	options.push_back(e);
}


GOptionEntry* OptionArgs::getArray()
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
int OptionArgs::parse(int argc,char **argv)
{

	GError *error = NULL;
	GOptionContext *context;
	GOptionEntry *pGOptions;



	context = g_option_context_new (0);
	g_option_context_set_summary(context, PACKAGE " ver. " PACKAGE_VERSION);

	pGOptions = getArray();
	g_option_context_add_main_entries(context, pGOptions,NULL);

	free(pGOptions);

	if (!g_option_context_parse (context, &argc, &argv, &error)) {
		g_print ("option parsing failed: %s\n", error->message);
		return 0;
	}

	return 1;
}



