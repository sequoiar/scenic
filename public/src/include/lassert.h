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
 *      Detailed description
 */

#ifndef __LASSERT_H__
#define __LASSERT_H__

#define ASSERT_THROWS   

#ifndef ASSERT_THROWS
#include <assert.h>
#else //ASSERT_THROWS

#include <assert.h>
#include "logWriter.h"

#ifdef assert
#undef assert
#endif

#define assert(expr)                                                        \
    ((expr)                             \
     ?__ASSERT_VOID_CAST (0)            \
     :  assert_throw(__STRING(expr), __FILE__, __LINE__, __ASSERT_FUNCTION))


extern void assert_throw(__const char *__assertion, __const char *__file,
                           unsigned int __line, __const char *__function);

#endif  //ASSERT_THROWS




#endif  //__LASSERT_H__
