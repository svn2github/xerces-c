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
 * originally based on software copyright (c) 1999, International
 * Business Machines, Inc., http://www.ibm.com .  For more information
 * on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

/*
 * $Log$
 * Revision 1.2  2002/09/24 20:12:48  tng
 * Performance: use XMLString::equals instead of XMLString::compareString
 *
 * Revision 1.1.1.1  2002/02/01 22:22:50  peiyongz
 * sane_include
 *
 * Revision 1.4  2001/11/20 20:21:49  knoaman
 * Add check for attribute derviation ok when redefining attribute group by restriction.
 *
 * Revision 1.3  2001/11/20 15:57:04  knoaman
 * Add check for multiple attributes with type derived from ID.
 *
 * Revision 1.2  2001/08/09 15:23:17  knoaman
 * add support for <anyAttribute> declaration.
 *
 * Revision 1.1  2001/07/31 15:26:54  knoaman
 * Added support for <attributeGroup>.
 *
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/schema/XercesAttGroupInfo.hpp>
#include <xercesc/util/QName.hpp>

// ---------------------------------------------------------------------------
//  XercesAttGroupInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
XercesAttGroupInfo::XercesAttGroupInfo()
    : fTypeWithId(false)
    , fAttributes(0)
    , fAnyAttributes(0)
    , fCompleteWildCard(0)
{

}


XercesAttGroupInfo::~XercesAttGroupInfo()
{
    delete fAttributes;
    delete fAnyAttributes;
    delete fCompleteWildCard;
}

bool XercesAttGroupInfo::containsAttribute(const XMLCh* const name,
                                           const unsigned int uri) {

    if (fAttributes) {

        unsigned int attCount = fAttributes->size();

        if (attCount) {

            for (unsigned int i=0; i < attCount; i++) {

                QName* attName = fAttributes->elementAt(i)->getAttName();

                if (attName->getURI() == uri &&
                    XMLString::equals(attName->getLocalPart(),name)) {
                    return true;
                }
            }
        }
    }

    return false;
}

// ---------------------------------------------------------------------------
//  XercesAttGroupInfo: Getter methods
// ---------------------------------------------------------------------------
const SchemaAttDef* XercesAttGroupInfo::getAttDef(const XMLCh* const baseName,
                                                  const int uriId) const {

    // If no list, then return a null
    if (!fAttributes)
        return 0;

    unsigned int attSize = fAttributes->size();

    for (unsigned int i=0; i<attSize; i++) {

        const SchemaAttDef* attDef = fAttributes->elementAt(i);
        QName* attName = attDef->getAttName();

        if (uriId == (int) attName->getURI() &&
			XMLString::equals(baseName, attName->getLocalPart())) {

            return attDef;
        }
    }

    return 0;
}

/**
  * End of file XercesAttGroupInfo.cpp
  */


