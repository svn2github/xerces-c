/*
 * Copyright 1999-2000,2004 The Apache Software Foundation.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * $Id$
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include "CoreTests.hpp"
#include <xercesc/util/ArrayIndexOutOfBoundsException.hpp>
#include <xercesc/util/ValueArrayOf.hpp>
#include <string.h>


// ---------------------------------------------------------------------------
//  Local test functions
// ---------------------------------------------------------------------------



// ---------------------------------------------------------------------------
//  Test entry point
// ---------------------------------------------------------------------------
bool testString()
{
    XERCES_STD_QUALIFIER wcout  << L"----------------------------------\n"
                << L"Testing String class\n"
                << L"----------------------------------" << XERCES_STD_QUALIFIER endl;

    bool retVal = true;

    try
    {
    }

    catch(const XMLException& toCatch)
    {
        XERCES_STD_QUALIFIER wcout  << L"  ERROR: Unexpected exception!\n   Msg: "
                    << toCatch.getMessage() << XERCES_STD_QUALIFIER endl;
        return false;
    }
    return retVal;
}
