/*
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2001 The Apache Software Foundation.  All rights
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
 * originally based on software copyright (c) 2001, International
 * Business Machines, Inc., http://www.ibm.com .  For more information
 * on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

/*
 * $Id$
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <util/PlatformUtils.hpp>
#include <sax/SAXException.hpp>
#include <sax/SAXParseException.hpp>
#include <parsers/IDOMParser.hpp>
#include <idom/IDOM_DOMException.hpp>
#include <idom/IDOM_Document.hpp>
#include "IDOMCount.hpp"
#include <string.h>
#include <stdlib.h>




// ---------------------------------------------------------------------------
//  This is a simple program which invokes the DOMParser to build a DOM
//  tree for the specified input file. It then walks the tree and counts
//  the number of elements. The element count is then printed.
// ---------------------------------------------------------------------------
static void usage()
{
    cout << "\nUsage:\n"
            "    IDOMCount [-v -n] {XML file}\n\n"
            "This program invokes the XML4C DOM parser, builds\n"
            "the DOM tree, and then prints the number of elements\n"
            "found in the input XML file.\n\n"
            "Options:\n"
            "    -v=xxx      Validation scheme [always | never | auto*]\n"
            "    -n          Enable namespace processing. Defaults to off.\n\n"
            "    -s          Enable schema processing. Defaults to off.\n\n"
            "  * = Default if not provided explicitly\n\n"
         << endl;
}



// ---------------------------------------------------------------------------
//
//  Recursively Count up the total number of child Elements under the specified Node.
//
// ---------------------------------------------------------------------------
static int countChildElements(IDOM_Node *n)
{
    IDOM_Node *child;
    int count = 0;
    if (n) {
        if (n->getNodeType() == IDOM_Node::ELEMENT_NODE)
        {
            count++;
            for (child = n->getFirstChild(); child != 0; child=child->getNextSibling())
            {
                if (child->getNodeType() == IDOM_Node::ELEMENT_NODE)
                {
                    count += countChildElements(child);
                }
            }
        }
    }
    return count;
}


// ---------------------------------------------------------------------------
//
//   main
//
// ---------------------------------------------------------------------------
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

    const char*               xmlFile = 0;
    IDOMParser::ValSchemes    valScheme = IDOMParser::Val_Auto;
    bool                      doNamespaces    = false;
    bool                      doSchema        = false;

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
                valScheme = IDOMParser::Val_Never;
            else if (!strcmp(parm, "auto"))
                valScheme = IDOMParser::Val_Auto;
            else if (!strcmp(parm, "always"))
                valScheme = IDOMParser::Val_Always;
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
         else if (!strcmp(argV[argInd], "-s")
              ||  !strcmp(argV[argInd], "-S"))
        {
            doSchema = true;
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
    IDOMParser parser;
    parser.setValidationScheme(valScheme);
    parser.setDoNamespaces(doNamespaces);
    parser.setDoSchema(doSchema);

    // And create our error handler and install it
    DOMCountErrorHandler errorHandler;
    parser.setErrorHandler(&errorHandler);

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
    catch (const IDOM_DOMException& toCatch)
    {
        cerr << "\nDOM Error during parsing: '" << xmlFile << "'\n"
             << "DOMException code is:  \n"
             << toCatch.code << "\n" << endl;
        XMLPlatformUtils::Terminate();
        return 4;
    }
    catch (...)
    {
       cerr << "\nUnexpected exception during parsing: '" << xmlFile << "'\n";
        XMLPlatformUtils::Terminate();
        return 4;
    }

    //
    //  Extract the DOM tree, get the list of all the elements and report the
    //  length as the count of elements.
    //
    if (errorHandler.getSawErrors())
    {
        cout << "\nErrors occured, no output available\n" << endl;
    }
     else
    {
        IDOM_Document *doc = parser.getDocument();
        unsigned int elementCount = 0;
        if (doc)
            elementCount = countChildElements((IDOM_Node*)doc->getDocumentElement());

        // Print out the stats that we collected and time taken.
        cout << xmlFile << ": " << duration << " ms ("
             << elementCount << " elems)." << endl;
    }

    // And call the termination method
    XMLPlatformUtils::Terminate();

    return 0;
}





DOMCountErrorHandler::DOMCountErrorHandler() :

    fSawErrors(false)
{
}

DOMCountErrorHandler::~DOMCountErrorHandler()
{
}


// ---------------------------------------------------------------------------
//  DOMCountHandlers: Overrides of the SAX ErrorHandler interface
// ---------------------------------------------------------------------------
void DOMCountErrorHandler::error(const SAXParseException& e)
{
    fSawErrors = true;
    cerr << "\nError at file " << StrX(e.getSystemId())
         << ", line " << e.getLineNumber()
         << ", char " << e.getColumnNumber()
         << "\n  Message: " << StrX(e.getMessage()) << endl;
}

void DOMCountErrorHandler::fatalError(const SAXParseException& e)
{
    fSawErrors = true;
    cerr << "\nFatal Error at file " << StrX(e.getSystemId())
         << ", line " << e.getLineNumber()
         << ", char " << e.getColumnNumber()
         << "\n  Message: " << StrX(e.getMessage()) << endl;
}

void DOMCountErrorHandler::warning(const SAXParseException& e)
{
    cerr << "\nWarning at file " << StrX(e.getSystemId())
         << ", line " << e.getLineNumber()
         << ", char " << e.getColumnNumber()
         << "\n  Message: " << StrX(e.getMessage()) << endl;
}

void DOMCountErrorHandler::resetErrors()
{
}
