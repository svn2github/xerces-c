/*
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 1999-2000 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Xerces" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache\@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation, and was
 * originally based on software copyright (c) 1999, International
 * Business Machines, Inc., http://www.ibm.com .  For more information
 * on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

/*
 * $Log$
 * Revision 1.8  2000/06/16 20:25:23  rahulj
 * Add the -v=always option to force validation checking. Need this
 * option for running the conformance tests.
 *
 * Revision 1.7  2000/05/31 18:42:31  rahulj
 * 'Auto' validation is the default processing mode.
 *
 * Revision 1.6  2000/03/03 01:29:29  roddey
 * Added a scanReset()/parseReset() method to the scanner and
 * parsers, to allow for reset after early exit from a progressive parse.
 * Added calls to new Terminate() call to all of the samples. Improved
 * documentation in SAX and DOM parsers.
 *
 * Revision 1.5  2000/03/02 19:53:39  roddey
 * This checkin includes many changes done while waiting for the
 * 1.1.0 code to be finished. I can't list them all here, but a list is
 * available elsewhere.
 *
 * Revision 1.4  2000/02/11 02:32:46  abagchi
 * Removed StrX::transcode
 *
 * Revision 1.3  2000/02/06 07:47:17  rahulj
 * Year 2K copyright swat.
 *
 * Revision 1.2  1999/11/12 02:13:40  rahulj
 * It now validates when the -v option is specified.
 *
 * Revision 1.1.1.1  1999/11/09 01:09:52  twl
 * Initial checkin
 *
 * Revision 1.8  1999/11/08 20:43:34  rahul
 * Swat for adding in Product name and CVS comment log variable.
 *
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <util/PlatformUtils.hpp>
#include <sax/SAXException.hpp>
#include <sax/SAXParseException.hpp>
#include <parsers/DOMParser.hpp>
#include "DOMCount.hpp"
#include <string.h>
#include <stdlib.h>




// ---------------------------------------------------------------------------
//  This is a simple program which invokes the DOMParser to build a DOM
//  tree for the specified input file. It then walks the tree and counts
//  the number of elements. The element count is then printed.
// ---------------------------------------------------------------------------
void usage()
{
    cout << "\nUsage:\n"
            "    DOMCount [-v -n] {XML file}\n\n"
            "This program invokes the XML4C DOM parser, builds\n"
            "the DOM tree, and then prints the number of elements\n"
            "found in the input XML file.\n\n"
            "Options:\n"
            "    -v=xxx      Validation scheme [always | never | auto*]\n"
            "    -n          Enable namespace processing. Defaults to off.\n\n"
            "  * = Default if not provided explicitly\n\n"
         << endl;
}


int main(int argC, char* argV[])
{
    // Initialize the XML4C system
    try
    {
        XMLPlatformUtils::Initialize();
    }

    catch (const XMLException& toCatch)
    {
         cerr << "Error during initialization! :\n"
              << StrX(toCatch.getMessage()) << endl;
         return 1;
    }

    // Check command line and extract arguments.
    if (argC < 2)
    {
        usage();
        return 1;
    }

    const char*              xmlFile = 0;
    DOMParser::ValSchemes    valScheme = DOMParser::Val_Auto;
    bool                     doNamespaces    = false;

    // See if non validating dom parser configuration is requested.
    if ((argC == 2) && !strcmp(argV[1], "-?"))
    {
        usage();
        return 2;
    }

    int argInd;
    for (argInd = 1; argInd < argC; argInd++)
    {
        // Break out on first non-dash parameter
        if (argV[argInd][0] != '-')
            break;

        if (!strncmp(argV[argInd], "-v=", 3)
        ||  !strncmp(argV[argInd], "-V=", 3))
        {
            const char* const parm = &argV[argInd][3];

            if (!strcmp(parm, "never"))
                valScheme = DOMParser::Val_Never;
            else if (!strcmp(parm, "auto"))
                valScheme = DOMParser::Val_Auto;
            else if (!strcmp(parm, "always"))
                valScheme = DOMParser::Val_Always;
            else
            {
                cerr << "Unknown -v= value: " << parm << endl;
                return 2;
            }
        }
         else if (!strcmp(argV[argInd], "-n")
              ||  !strcmp(argV[argInd], "-N"))
        {
            doNamespaces = true;
        }
         else
        {
            cerr << "Unknown option '" << argV[argInd]
                 << "', ignoring it\n" << endl;
        }
    }

    //
    //  There should be only one and only one parameter left, and that
    //  should be the file name.
    //
    if (argInd != argC - 1)
    {
        usage();
        return 1;
    }
    xmlFile = argV[argInd];

    // Instantiate the DOM parser.
    DOMParser parser;
    parser.setValidationScheme(valScheme);
    parser.setDoNamespaces(doNamespaces);

    // And create our error handler and install it
    DOMCount elementCounter;
    parser.setErrorHandler(&elementCounter);

    //
    //  Get the starting time and kick off the parse of the indicated
    //  file. Catch any exceptions that might propogate out of it.
    //
    unsigned long duration;
    try
    {
        const unsigned long startMillis = XMLPlatformUtils::getCurrentMillis();
        parser.parse(xmlFile);
        const unsigned long endMillis = XMLPlatformUtils::getCurrentMillis();
        duration = endMillis - startMillis;
    }

    catch (const XMLException& toCatch)
    {
        cerr << "\nError during parsing: '" << xmlFile << "'\n"
             << "Exception message is:  \n"
             << StrX(toCatch.getMessage()) << "\n" << endl;
        return -1;
    }

    //
    //  Extract the DOM tree, get the list of all the elements and report the
    //  length as the count of elements.
    //
    if (elementCounter.getSawErrors())
    {
        cout << "\nErrors occured, no output available\n" << endl;
    }
     else
    {
        DOM_Document doc = parser.getDocument();
        unsigned int elementCount = doc.getElementsByTagName("*").getLength();

        // Print out the stats that we collected and time taken.
        cout << xmlFile << ": " << duration << " ms ("
             << elementCount << " elems)." << endl;
    }

    // And call the termination method
    XMLPlatformUtils::Terminate();

    return 0;
}


DOMCount::DOMCount() :

    fSawErrors(false)
{
}

DOMCount::~DOMCount()
{
}


// ---------------------------------------------------------------------------
//  DOMCountHandlers: Overrides of the SAX ErrorHandler interface
// ---------------------------------------------------------------------------
void DOMCount::error(const SAXParseException& e)
{
    fSawErrors = true;
    cerr << "\nError at file " << StrX(e.getSystemId())
         << ", line " << e.getLineNumber()
         << ", char " << e.getColumnNumber()
         << "\n  Message: " << StrX(e.getMessage()) << endl;
}

void DOMCount::fatalError(const SAXParseException& e)
{
    fSawErrors = true;
    cerr << "\nFatal Error at file " << StrX(e.getSystemId())
         << ", line " << e.getLineNumber()
         << ", char " << e.getColumnNumber()
         << "\n  Message: " << StrX(e.getMessage()) << endl;
}

void DOMCount::warning(const SAXParseException& e)
{
    cerr << "\nWarning at file " << StrX(e.getSystemId())
         << ", line " << e.getLineNumber()
         << ", char " << e.getColumnNumber()
         << "\n  Message: " << StrX(e.getMessage()) << endl;
}

void DOMCount::resetErrors()
{
}
