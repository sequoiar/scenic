/* headerGPL.c
 * Copyright 2008 Koya Charles & Tristan Matthews 
 *
 * This file is part of [propulse]ART.
 *
 * [propulse]ART is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * [propulse]ART is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <glib.h>
#include "config.h"
#include "optionArgs.h"
#include "logWriter.h"

OptionArgs::~OptionArgs()
{
    if(pA_)
        delete[] pA_;
}

void OptionArgs::add(BaseArg *ba)
{
    if(ba->type == 'i') {
        IntArg* arg = static_cast<IntArg*>(ba);
        GOptionEntry e = {arg->l_arg.c_str(), arg->s_arg, 0,
                          G_OPTION_ARG_INT, arg->arg, arg->desc.c_str(),
                          arg->arg_desc.c_str()};
        options_.push_back(e);
    }
    else if (ba->type == 'b') {
        BoolArg* arg = static_cast<BoolArg*>(ba);
        GOptionEntry e = {arg->l_arg.c_str(), arg->s_arg, 0,
                          G_OPTION_ARG_NONE, arg->arg, arg->desc.c_str(), ""};
        options_.push_back(e);
    }
    else if (ba->type == 's') {
        StringArg* arg = static_cast<StringArg*>(ba);
        GOptionEntry e = {arg->l_arg.c_str(), arg->s_arg, 0,
                          G_OPTION_ARG_STRING, arg->arg, arg->desc.c_str(),
                          arg->arg_desc.c_str()};
        options_.push_back(e);
    }
    else {
        LOG("Bad BaseArg type", ERROR);
    }
}


GOptionEntry* OptionArgs::getArray()
{
    GOptionEntry n = { NULL, 0, 0, G_OPTION_ARG_NONE, 0, 0, 0 };
    unsigned int count = 0;
    if(options_.empty())
        return 0;
    if(pA_)
    {
        delete[] pA_;
        pA_ = 0;
    }
    pA_ = new GOptionEntry[options_.size() + 1];

    for(Options::iterator it = options_.begin(); it != options_.end(); ++it)
        pA_[count++] = *it;

    pA_[count] =  n;

    return pA_;
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

    if(pA_)
    {
        delete[] pA_;
        pA_ = 0;
    }
    return ret;
}


