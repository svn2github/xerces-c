/*
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 1999-2003 The Apache Software Foundation.  All rights
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
 * $Id$
 * $Log$
 * Revision 1.2  2004/08/24 16:00:15  peiyongz
 * To build on AIX/Win2003-ecl
 *
 * Revision 1.1  2004/08/19 17:17:21  peiyongz
 * XSValueTest
 *
 *
 */

#if !defined(XSVALUE_TEST_HPP)
#define XSVALUE_TEST_HPP

// ---------------------------------------------------------------------------
//  Includes for all the program files to see
// ---------------------------------------------------------------------------
#include <xercesc/util/XercesDefs.hpp>

#if defined(XERCES_NEW_IOSTREAMS)
#include <iostream>
#else
#include <iostream.h>
#endif

#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_USE

// ---------------------------------------------------------------------------
//  This is a simple class that lets us do easy (though not terribly efficient)
//  trancoding of XMLCh data to local code page for display.
// ---------------------------------------------------------------------------
class StrX
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    StrX(const XMLCh* const toTranscode)
    {
        fUnicodeForm = XMLString::replicate(toTranscode);
        fLocalForm   = XMLString::transcode(toTranscode);
    }

    StrX(const char* const toTranscode)
    {
        // Call the private transcoding method
        fLocalForm   = XMLString::replicate(toTranscode);
        fUnicodeForm = XMLString::transcode(toTranscode);
    }

    ~StrX()
    {
        XMLString::release(&fLocalForm);
        XMLString::release(&fUnicodeForm);
    }

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    const char* localForm() const
    {
        return fLocalForm;
    }

    const XMLCh* unicodeForm() const
    {
        return fUnicodeForm;
    }

private :

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    StrX(const StrX&);
    StrX & operator=(const StrX &);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fLocalForm
    //      This is the local code page form of the string.
    //
    //  fUnicodeForm
    //      This is the Unicode XMLCh format of the string.
    // -----------------------------------------------------------------------
    XMLCh*   fUnicodeForm;
    char*    fLocalForm;
};

#define UniForm(str) StrX(str).unicodeForm()
#define LocForm(str) StrX(str).localForm()

inline XERCES_STD_QUALIFIER ostream& operator<<(XERCES_STD_QUALIFIER ostream& target, const StrX& toDump)
{
    target << toDump.localForm();
    return target;
}

#endif
