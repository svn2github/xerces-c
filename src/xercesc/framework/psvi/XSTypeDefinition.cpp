/*
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2003 The Apache Software Foundation.  All rights
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
 * Revision 1.1  2003/09/16 14:33:36  neilg
 * PSVI/schema component model classes, with Makefile/configuration changes necessary to build them
 *
 */

#include <xercesc/framework/psvi/XSTypeDefinition.hpp>

XERCES_CPP_NAMESPACE_BEGIN

XSTypeDefinition::XSTypeDefinition( TYPE_CATEGORY typeCategory,
            MemoryManager* const manager ):  
        XSObject(XSConstants::TYPE_DEFINITION, manager),
        fTypeCategory(typeCategory)
{
}

// overloaded XSObject methods
inline const XMLCh *XSTypeDefinition::getName() 
{
    // REVISIT
    return 0;
}

inline const XMLCh *XSTypeDefinition::getNamespace() 
{
    // REVISIT
    return 0;
}

inline XSNamespaceItem *XSTypeDefinition::getNamespaceItem() 
{
    //REVISIT
    return 0;
}

// XSTypeDefinition implementation
inline XSTypeDefinition::TYPE_CATEGORY XSTypeDefinition::getTypeCategory() const
{
    return fTypeCategory;
}

XSTypeDefinition *XSTypeDefinition::getBaseType() 
{
    // REVISIT
    return 0;
}

bool XSTypeDefinition::isFinal(short toTest)
{
    // REVISIT
    return false;
}

short XSTypeDefinition::getFinal() const
{
    // REVISIT
    return 0;
}

bool XSTypeDefinition::getAnonymous() const
{
    // REVISIT
    return false; 
}

bool XSTypeDefinition::derivedFromType(const XSTypeDefinition * const ancestorType, 
                               short derivationMethod)
{
    // REVISIT
    return false;
}

bool XSTypeDefinition::derivedFrom(const XMLCh *typeNamespace, 
                           const XMLCh *name, 
                           short derivationMethod)
{
    // REVISIT
    return false;
}

XERCES_CPP_NAMESPACE_END


