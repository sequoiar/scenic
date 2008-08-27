/*
 * Copyright (C) 2008 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is free software: you can redistribute it and*or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Sropulpof is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Sropulpof.  If not, see <http:*www.gnu.org*licenses*>.
 */

#include "sdpmedia.h"
#include <string.h>
#include <sstream>
#include <iostream>

sdpMedia::sdpMedia( int type )
    : _type( type ), _mediaStr(""), _codecList(0), _port( 0 ){
    switch( type )
    {
        case MIME_TYPE_AUDIO:
            _mediaStr = "audio";
            break;
        case MIME_TYPE_VIDEO:
            _mediaStr = "video";
            break;
        default:
            _mediaStr = "unknown";
            break;
    }
}


sdpMedia::sdpMedia( std::string media, int port )
    : _type( -1 ), _mediaStr(media), _codecList(0), _port( port ){
    if( media == "audio" )
        _type = MIME_TYPE_AUDIO;
    else if ( media == "video" )
        _type = MIME_TYPE_VIDEO;
    else
        _type = MIME_TYPE_UNKNOWN;
}


sdpMedia::~sdpMedia()
{
    int i;
    for(i=0; i<(int)_codecList.size(); i++)
        delete _codecList[i];
}


void sdpMedia::addCodec( sdpCodec *codec ){
    _codecList.push_back(codec);
}


void sdpMedia::addCodec( std::string codecName ){
    // We have to build the codec from its encoding name
    sdpCodec *codec;

    codec = new sdpCodec( _type, codecName );
    _codecList.push_back(codec);
}


void sdpMedia::removeCodec( std::string codecName )
{
    // Look for the codec by its encoding name
    int i;
    int size;
    std::string enc_name;
    std::vector<sdpCodec*>::iterator iter;

    size = _codecList.size();
    std::cout << "vector size: " << size << std::endl;

    for( i=0 ; i<size ; i++ ){
        std::cout << _codecList[i]->_name.c_str() << std::endl;
        if( strcmp(_codecList[i]->_name.c_str(), codecName.c_str()) == 0 ){
            std::cout << "erase " <<_codecList[i]->_name << std::endl;
            iter = _codecList.begin()+i;
            _codecList.erase(iter);
            break;
        }
    }
}


void sdpMedia::clearCodecList( void ) {
    // Erase every codecs from the list
    _codecList.clear();
}


std::string sdpMedia::toString( void ){
    std::ostringstream display;
    int size, i;

    size = _codecList.size();

    display << "TYPE = " << _mediaStr << std::endl;
    display << "PORT = " << getPort() << std::endl;
    display << "CODECS = ";
    for(i=0; i<size; i++){
        display << _codecList[i]->_name << " ";
    }

    display << std::endl;

    return display.str();
}


