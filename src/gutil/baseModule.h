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

#ifndef __BASE_MODULE_H__
#define __BASE_MODULE_H__

#include <string>
#include <list>

class BaseArg
{
public:
	BaseArg(char t,std::string l, char s, std::string d, std::string a)
		: type(t),l_arg(l),desc(d),arg_desc(a),s_arg(s){
	}
	char type;
	std::string l_arg, desc, arg_desc;
	char s_arg;
	virtual ~BaseArg(){
	}
};

class IntArg : public BaseArg
{
public:
	int* arg;
	IntArg(int* i,std::string l, char s, std::string d, std::string a) :
		BaseArg('i',l,s,d,a),arg(i){
	}


};

class BoolArg : public BaseArg
{
public:
	bool* arg;
	BoolArg(bool* b,std::string l, char s, std::string d, std::string a) :
		BaseArg('b',l,s,d, std::string()),arg(b){
	}
};

class StringArg : public BaseArg
{
public:
	char** arg ;
	StringArg(char** ppc,std::string l, char s, std::string d, std::string a) :
		BaseArg('s',l, s, d, a),arg(ppc){
	}
};





class BaseModule
{
public:
	typedef std::list<BaseArg*> ArgList;
	typedef std::list<BaseArg*>::iterator iterator;
	virtual ArgList args(){
		return ArgList();
	}
	virtual bool run(){
		return 1;
	}
	virtual ~BaseModule(){
	}
};

#endif
