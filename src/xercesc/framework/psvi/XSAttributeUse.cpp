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
 * Revision 1.3  2003/11/14 22:33:30  neilg
 * ./src/xercesc/framework/psvi/XSAnnotation.cpp
 * ./src/xercesc/framework/psvi/XSAnnotation.hpp
 * ./src/xercesc/framework/psvi/XSAttributeDeclaration.cpp
 * ./src/xercesc/framework/psvi/XSAttributeDeclaration.hpp
 * ./src/xercesc/framework/psvi/XSAttributeGroupDefinition.cpp
 * ./src/xercesc/framework/psvi/XSAttributeGroupDefinition.hpp
 * ./src/xercesc/framework/psvi/XSAttributeUse.cpp
 * ./src/xercesc/framework/psvi/XSAttributeUse.hpp
 * ./src/xercesc/framework/psvi/XSComplexTypeDefinition.cpp
 * ./src/xercesc/framework/psvi/XSComplexTypeDefinition.hpp
 * ./src/xercesc/framework/psvi/XSElementDeclaration.cpp
 * ./src/xercesc/framework/psvi/XSElementDeclaration.hpp
 * ./src/xercesc/framework/psvi/XSFacet.cpp
 * ./src/xercesc/framework/psvi/XSFacet.hpp
 * ./src/xercesc/framework/psvi/XSIDCDefinition.cpp
 * ./src/xercesc/framework/psvi/XSIDCDefinition.hpp
 * ./src/xercesc/framework/psvi/XSModel.cpp
 * ./src/xercesc/framework/psvi/XSModel.hpp
 * ./src/xercesc/framework/psvi/XSModelGroup.cpp
 * ./src/xercesc/framework/psvi/XSModelGroup.hpp
 * ./src/xercesc/framework/psvi/XSModelGroupDefinition.cpp
 * ./src/xercesc/framework/psvi/XSModelGroupDefinition.hpp
 * ./src/xercesc/framework/psvi/XSMultiValueFacet.cpp
 * ./src/xercesc/framework/psvi/XSMultiValueFacet.hpp
 * ./src/xercesc/framework/psvi/XSNamespaceItem.cpp
 * ./src/xercesc/framework/psvi/XSNamespaceItem.hpp
 * ./src/xercesc/framework/psvi/XSNotationDeclaration.cpp
 * ./src/xercesc/framework/psvi/XSNotationDeclaration.hpp
 * ./src/xercesc/framework/psvi/XSObject.cpp
 * ./src/xercesc/framework/psvi/XSObject.hpp
 * ./src/xercesc/framework/psvi/XSParticle.cpp
 * ./src/xercesc/framework/psvi/XSParticle.hpp
 * ./src/xercesc/framework/psvi/XSSimpleTypeDefinition.cpp
 * ./src/xercesc/framework/psvi/XSSimpleTypeDefinition.hpp
 * ./src/xercesc/framework/psvi/XSTypeDefinition.cpp
 * ./src/xercesc/framework/psvi/XSTypeDefinition.hpp
 * ./src/xercesc/framework/psvi/XSWildcard.cpp
 * ./src/xercesc/framework/psvi/XSWildcard.hpp
 * ./src/xercesc/internal/XMLGrammarPoolImpl.cpp
 * ./src/xercesc/internal/XMLGrammarPoolImpl.hpp
 * ./src/xercesc/validators/schema/identity/IdentityConstraint.cpp
 * ./src/xercesc/validators/schema/identity/IdentityConstraint.hpp
 * ./src/xercesc/validators/schema/SchemaGrammar.hpp
 * ./src/xercesc/validators/schema/TraverseSchema.cpp
 *
 * Revision 1.2  2003/11/06 15:30:04  neilg
 * first part of PSVI/schema component model implementation, thanks to David Cargill.  This covers setting the PSVIHandler on parser objects, as well as implementing XSNotation, XSSimpleTypeDefinition, XSIDCDefinition, and most of XSWildcard, XSComplexTypeDefinition, XSElementDeclaration, XSAttributeDeclaration and XSAttributeUse.
 *
 * Revision 1.1  2003/09/16 14:33:36  neilg
 * PSVI/schema component model classes, with Makefile/configuration changes necessary to build them
 *
 */

#include <xercesc/framework/psvi/XSAttributeUse.hpp>
#include <xercesc/framework/psvi/XSAttributeDeclaration.hpp>
#include <xercesc/validators/schema/SchemaAttDef.hpp>

XERCES_CPP_NAMESPACE_BEGIN

XSAttributeUse::XSAttributeUse(SchemaAttDef*            attDef,
                               XSModel*                 xsModel,
                               MemoryManager * const    manager):
    fAttDef(attDef),
    XSObject(XSConstants::ATTRIBUTE_USE, xsModel, manager )
{
    fXSAttributeDeclaration = (XSAttributeDeclaration*) getObjectFromMap(fAttDef);
    if (!fXSAttributeDeclaration)
    {
        fXSAttributeDeclaration = new (manager) XSAttributeDeclaration(fAttDef, xsModel, manager);
        putObjectInMap((void*)fAttDef, fXSAttributeDeclaration);
    }
}


XSAttributeUse::~XSAttributeUse() 
{
    // don't delete fXSAttributeDeclaration - deleted by XSModel
}
// XSAttributeUse methods


/**
 * [required]: determines whether this use of an attribute declaration 
 * requires an appropriate attribute information item to be present, or 
 * merely allows it. 
 */
bool XSAttributeUse::getRequired() const
{
    if (fAttDef->getDefaultType() == XMLAttDef::Required ||
        fAttDef->getDefaultType() == XMLAttDef::Required_And_Fixed)
    {
        return true;
    }
    return false;
}

/**
 * [attribute declaration]: provides the attribute declaration itself, 
 * which will in turn determine the simple type definition used. 
 */
XSAttributeDeclaration *XSAttributeUse::getAttrDeclaration()
{
    return fXSAttributeDeclaration;
}

/**
 * Value Constraint: one of default, fixed. 
 */
XSConstants::VALUE_CONSTRAINT XSAttributeUse::getConstraintType() const
{
    // REVISIT: same as XSAttributeDeclaration????
    if (fAttDef->getDefaultType() & XMLAttDef::Default)
    {
        return XSConstants::VC_DEFAULT;
    }
    if (fAttDef->getDefaultType() & XMLAttDef::Fixed ||
        fAttDef->getDefaultType() & XMLAttDef::Required_And_Fixed)
    {
        return XSConstants::VC_FIXED;
    }
    return XSConstants::VC_NONE;
}

/**
 * Value Constraint: The actual value. 
 */
const XMLCh *XSAttributeUse::getConstraintValue()
{
    // REVISIT: same as XSAttributeDeclaration????
    return fAttDef->getValue();
}

XERCES_CPP_NAMESPACE_END


