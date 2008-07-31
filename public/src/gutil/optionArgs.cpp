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
#include <glib.h>
#include "config.h"
#include "optionArgs.h"
#include "logWriter.h"
OptionArgs::~OptionArgs()
{
    if(pA)
        delete[] pA;
}


void OptionArgs::add(BaseModule::ArgList args)
{
    for(BaseModule::iterator it= args.begin(); it != args.end(); ++it){
        add(*it);
    }
}


void OptionArgs::add(BaseArg *ba)
{
    if(ba->type == 'i') {
        IntArg* arg = dynamic_cast<IntArg*>(ba);
        GOptionEntry e = {arg->l_arg.c_str(), arg->s_arg, 0,
                          G_OPTION_ARG_INT, arg->arg, arg->desc.c_str(),
                          arg->arg_desc.c_str()};
        options.push_back(e);
    }
    else if (ba->type == 'b') {
        BoolArg* arg = dynamic_cast<BoolArg*>(ba);
        GOptionEntry e = {arg->l_arg.c_str(), arg->s_arg, 0,
                          G_OPTION_ARG_NONE, arg->arg, arg->desc.c_str()};
        options.push_back(e);
    }
    else if (ba->type == 's') {
        StringArg* arg = dynamic_cast<StringArg*>(ba);
        GOptionEntry e = {arg->l_arg.c_str(), arg->s_arg, 0,
                          G_OPTION_ARG_STRING, arg->arg, arg->desc.c_str(),
                          arg->arg_desc.c_str()};
        options.push_back(e);
    }
    else {
        LOG("Bad BaseArg type");
    }
}


GOptionEntry* OptionArgs::getArray()
{
    GOptionEntry n = { NULL };
    int count = 0;
    if(options.empty())
        return 0;
    if(pA)
        delete[] pA;
    pA = new GOptionEntry[options.size()+1];
    for(Options::iterator it= options.begin(); it != options.end(); ++it)
    {
        pA[count++] = *it;
    }

    pA[count] =  n ;

    return pA;
}


int OptionArgs::parse(int argc, char **argv)
{
    int ret = 1;
    GError *error = NULL;
    GOptionContext *context;
    GOptionEntry *pGOptions;


    context = g_option_context_new (0);
    g_option_context_set_summary(context, PACKAGE " ver. " PACKAGE_VERSION);

    pGOptions = getArray();
    g_option_context_add_main_entries(context, pGOptions, NULL);

    //afteradd delete?
    //delele(pGOptions);

    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_print ("option parsing failed: %s\n", error->message);
        ret = 0;
    }
    g_option_context_free(context);
    delete[] pA;


    return ret;
}


