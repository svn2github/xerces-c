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

#if !defined(XSIDCDEFINITION_HPP)
#define XSIDCDEFINITION_HPP

#include <xercesc/framework/psvi/XSObject.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
 * This class describes all properties of a Schema Identity Constraint
 * Definition component.
 * This is *always* owned by the validator /parser object from which
 * it is obtained.  
 */

// forward declarations
class XSAnnotation;

class IdentityConstraint;
class XMLStringPool;

class XMLPARSER_EXPORT XSIDCDefinition : public XSObject
{
public:

    // Identity Constraints
    enum IC_CATEGORY {
	    /**
	     * 
	     */
	    IC_KEY                    = 1,
	    /**
	     * 
	     */
	    IC_KEYREF                 = 2,
	    /**
	     * 
	     */
	    IC_UNIQUE                 = 3
    };

    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    /** @name Constructors */
    //@{

    /**
      * The default constructor 
      *
      * @param  manager     The configurable memory manager
      */
    XSIDCDefinition(IdentityConstraint* identityConstraint,
                XSModel*                xsModel,
                MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    //@};

    /** @name Destructor */
    //@{
    ~XSIDCDefinition();
    //@}

    //---------------------
    /** @name overridden XSXSObject methods */

    //@{

    /**
     * The name of type <code>NCName</code> of this declaration as defined in 
     * XML Namespaces.
     */
    const XMLCh* getName();

    /**
     *  The [target namespace] of this object, or <code>null</code> if it is 
     * unspecified. 
     */
    const XMLCh* getNamespace();

    /**
     * A namespace schema information item corresponding to the target 
     * namespace of the component, if it's globally declared; or null 
     * otherwise.
     */
    XSNamespaceItem *getNamespaceItem();

    //@}

    //---------------------
    /** @name XSIDCDefinition methods */

    //@{

    /**
     * [identity-constraint category]: one of IC_KEY, IC_KEYREF or IC_UNIQUE. 
     */
    IC_CATEGORY getCategory() const;

    /**
     * [selector]: a restricted XPath expression. 
     */
    const XMLCh *getSelectorStr();

    /**
     * [fields]: a non-empty list of restricted XPath ([XPath]) expressions. 
     */
    StringList *getFieldStrs();

    /**
     * [referenced key]: required if [identity-constraint category] is IC_KEYREF, 
     * forbidden otherwise (when an identity-constraint definition with [
     * identity-constraint category] equal to IC_KEY or IC_UNIQUE). 
     */
    XSIDCDefinition *getRefKey();

    /**
     * A set of [annotations]. 
     */
    XSAnnotationList *getAnnotations();

    //@}

    //----------------------------------
    /** methods needed by implementation */

    //@{

    //@}
private:

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XSIDCDefinition(const XSIDCDefinition&);
    XSIDCDefinition & operator=(const XSIDCDefinition &);

protected:

    // -----------------------------------------------------------------------
    //  data members
    // -----------------------------------------------------------------------
    IdentityConstraint*     fIdentityConstraint;
    XSIDCDefinition*        fKey;
    StringList*             fStringList;
    XSAnnotationList*       fXSAnnotationList;
};

inline XSIDCDefinition::~XSIDCDefinition() {
    if (fStringList)
    {
        delete fStringList;
    }
    // don't delete fKey - deleted by XSModel
    if (fXSAnnotationList)
    {
        delete fXSAnnotationList;
    }
}

XERCES_CPP_NAMESPACE_END

#endif
