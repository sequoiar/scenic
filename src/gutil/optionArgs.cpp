/* optionArgs.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
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

#include "util.h"

#include "gutil.h"

#include <cstdlib>

/** RAII all resources will be cleaned here
 * since we store our int pointers inside 
 * GOptionEntrys we need to clear them out one by one 
 * our string ** is held in the second spot
 */
OptionArgs::~OptionArgs()
{
    delete[] pA_;

    while (!options_.empty())
    {
        switch(options_.back().first.arg)
        {
            case G_OPTION_ARG_STRING:
                delete options_.back().second;
                break;
            case G_OPTION_ARG_INT:
            case G_OPTION_ARG_NONE:
                delete static_cast<int*>(options_.back().first.arg_data);
                break;
            default:
                // Shouldn't have any undefined arg types 
                THROW_CRITICAL("Unhandled command line option type"); 
        }
        options_.back().first.arg_data = 0;
        options_.pop_back();    // get rid of it
    }
}

///Add a boolean value and alloc space for it
void OptionArgs::addBool(const char *l_arg, char s_arg, const char *desc)
{
    GOptionEntry e = {l_arg, s_arg, 0, G_OPTION_ARG_NONE, new gint(0), desc, 0};
    options_.push_back(OptPair(e,NULL));
}

///Add an int value and alloc space for it
void OptionArgs::addInt(const char *l_arg, char s_arg, const char *desc, const char *arg_desc)
{
    GOptionEntry e = {l_arg, s_arg, 0, G_OPTION_ARG_INT, new gint(0), desc, arg_desc};
    options_.push_back(OptPair(e,NULL));
}

///Add a string value and alloc space for address to receive the string from glib
void OptionArgs::addString(const char *l_arg, char s_arg, const char *desc, const char *arg_desc)
{
    /* glib will fill the address pointed to by p_str 
     * with the address of string argument or NULL if non specified
     * glib will still own the string 
     * we stor p_str with the GOptionEntry for later use and cleanup
     */
    char **p_str  = new (char*);  
    *p_str = NULL;

    GOptionEntry e = {l_arg, s_arg, 0, G_OPTION_ARG_STRING, p_str, desc, arg_desc};
    options_.push_back(OptPair(e,p_str));
}

///Alloc and buildout GOptionArray for glib
GOptionEntry* OptionArgs::getArray()
{
    if(options_.empty())
        return 0;
    if(pA_)
    {
        delete[] pA_;
        pA_ = 0;
    }

    int count = 0;
    pA_ = new GOptionEntry[options_.size() + 1];
    for(Options::iterator it = options_.begin(); it != options_.end(); ++it)
    {
        pA_[count++] = it->first;
    }

    //Trailing Sentinel
    GOptionEntry n = { NULL, 0, 0, G_OPTION_ARG_NONE, 0, 0, 0 };
    pA_[count] =  n;

    return pA_;
}

///Glib argument parsing action note g_option_context_free 
void OptionArgs::parse(int argc, char **argv)
{
    GError *error = NULL;
    GOptionContext *context;
    GOptionEntry *pGOptions;

    context = g_option_context_new (0);
    g_option_context_set_summary(context, PACKAGE " ver. " PACKAGE_VERSION);

    pGOptions = getArray();
    g_option_context_add_main_entries(context, pGOptions, NULL);

    bool noArgs = argc == 1;

    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_option_context_free(context);
        // FIXME: hack because the exception message never gets printed
        std::cerr << "option parsing failed: " << error->message << std::endl;
        THROW_CRITICAL("option parsing failed: " << error->message);
    }
    
    if (noArgs)      // no arguments given, print help
    {
        std::cout << g_option_context_get_help(context, TRUE, NULL);
        g_option_context_free(context);
        return;
    }

    g_option_context_free(context);

    for(Options::iterator it = options_.begin(); it != options_.end(); ++it)
    {
        switch (it->first.arg)
        {
            case G_OPTION_ARG_NONE: // boolean
                {
                    int val = (*static_cast<gboolean*>(it->first.arg_data) == TRUE) ? 1 : 0;
                    store[it->first.long_name] = val;
                    if(val)
                        LOG_DEBUG("key: " << it->first.long_name << " value: " << store[it->first.long_name] ); 
                    break;
                }
            case G_OPTION_ARG_INT: // int 
                {
                    int val(*static_cast<gint*>(it->first.arg_data));
                    store[it->first.long_name] = val;
                    if(val)
                        LOG_DEBUG("key: " << it->first.long_name << " value: " << store[it->first.long_name]);
                    break;
                }
            case G_OPTION_ARG_STRING: // string
                if(*(it->second))
                {
                    store[it->first.long_name] = std::string(*(it->second));
                    LOG_DEBUG("key: " << it->first.long_name << " value: " << store[it->first.long_name]);
                }
                break;
            default:
                THROW_CRITICAL("Shouldn't have any undefined arg types");
                break;
        }
    }
}

