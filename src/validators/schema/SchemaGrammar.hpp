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
 * Revision 1.3  2001/05/03 20:34:43  tng
 * Schema: SchemaValidator update
 *
 * Revision 1.2  2001/04/19 17:43:19  knoaman
 * More schema implementation classes.
 *
 * Revision 1.1  2001/03/21 21:56:33  tng
 * Schema: Add Schema Grammar, Schema Validator, and split the DTDValidator into DTDValidator, DTDScanner, and DTDGrammar.
 *
 */



#if !defined(SCHEMAGRAMMAR_HPP)
#define SCHEMAGRAMMAR_HPP

#include <dom/DOM_Document.hpp>
#include <framework/XMLNotationDecl.hpp>
#include <util/RefHash3KeysIdPool.hpp>
#include <util/NameIdPool.hpp>
#include <util/StringPool.hpp>
#include <validators/common/Grammar.hpp>
#include <validators/schema/SchemaElementDecl.hpp>

//
// This class stores the Schema information
//  NOTE: Schemas are not namespace aware, so we just use regular NameIdPool
//  data structures to store element and attribute decls. They are all set
//  to be in the global namespace and the full QName is used as the base name
//  of the decl. This means that all the URI parameters below are expected
//  to be null pointers (and anything else will cause an exception.)
//

// ---------------------------------------------------------------------------
//  Forward Declarations
// ---------------------------------------------------------------------------
class DatatypeValidatorFactory;
class ComplexTypeInfo;
class NamespaceScope;


class VALIDATORS_EXPORT SchemaGrammar : public Grammar
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    SchemaGrammar();
    virtual ~SchemaGrammar();

    // -----------------------------------------------------------------------
    //  Implementation of Virtual Interface
    // -----------------------------------------------------------------------
    virtual Grammar::GrammarType getGrammarType();

    virtual XMLElementDecl* findOrAddElemDecl
    (
        const   unsigned int    uriId
        , const XMLCh* const    baseName
        , const XMLCh* const    prefixName
        , const XMLCh* const    qName
        , unsigned int          scope
        ,       bool&           wasAdded
    ) ;

    virtual unsigned int getElemId
    (
        const   unsigned int    uriId
        , const XMLCh* const    baseName
        , const XMLCh* const    qName
        , unsigned int          scope
    )   const ;

    virtual const XMLElementDecl* getElemDecl
    (
        const   unsigned int    uriId
        , const XMLCh* const    baseName
        , const XMLCh* const    qName
        , unsigned int          scope
    )   const ;

    virtual XMLElementDecl* getElemDecl
    (
        const   unsigned int    uriId
        , const XMLCh* const    baseName
        , const XMLCh* const    qName
        , unsigned int          scope
    );

    virtual const XMLElementDecl* getElemDecl
    (
        const   unsigned int    elemId
    )   const;

    virtual XMLElementDecl* getElemDecl
    (
        const   unsigned int    elemId
    );

    virtual const XMLNotationDecl* getNotationDecl
    (
        const   XMLCh* const    notName
    )   const;

    virtual XMLNotationDecl* getNotationDecl
    (
        const   XMLCh* const    notName
    );

    virtual unsigned int putElemDecl
    (
        XMLElementDecl* const elemDecl
    )   const;

    virtual unsigned int putNotationDecl
    (
        XMLNotationDecl* const notationDecl
    )   const;

    virtual void reset();

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    RefHash3KeysIdPoolEnumerator<SchemaElementDecl> getElemEnumerator() const;
    const XMLCh* getTargetNamespace() const;
    XMLCh* getTargetNamespace() ;
    RefHashTableOf<XMLAttDef>* getAttributeDeclRegistry() const;
    RefHashTableOf<ComplexTypeInfo>* getComplexTypeRegistry() const;
    DatatypeValidatorFactory* getDatatypeRegistry() const;
    NamespaceScope* getNamespaceScope() const;

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    void setTargetNamespace(const XMLCh* const targetNamespace);
    void setAttributeDeclRegistry(RefHashTableOf<XMLAttDef>* const attReg);
    void setComplexTypeRegistry(RefHashTableOf<ComplexTypeInfo>* const other);
    void setDatatypeRegistry(DatatypeValidatorFactory* const dvRegistry);
    void setNamespaceScope(NamespaceScope* const nsScope);

private:

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fElemDeclPool
    //      This is the element decl pool. It contains all of the elements
    //      declared in the Schema (and their associated attributes.) When in
    //      non-validating mode, its just populated as new elements are seen
    //      and they are given default characteristics.
    //
    //  fNotationDeclPool
    //      This is a pool of NotationDecl objects, which contains all of the
    //      notations declared in the Schema.
    //
    //  fTargetNamespace
    //      Target name space for this grammar.
    //
    //  fAttributeDeclRegistry
    //      Global attribute declarations
    //
    //  fComplexTypeRegistry
    //      Stores complexType declaration info
    //
    //  fDatatypeRegistry
    //      Datatype validator factory
    //
    //  fNamespaceScope
    //      Prefix to Namespace map
    // -----------------------------------------------------------------------
    RefHash3KeysIdPool<SchemaElementDecl>* fElemDeclPool;
    NameIdPool<XMLNotationDecl>*           fNotationDeclPool;
    XMLCh*                                 fTargetNamespace;
    RefHashTableOf<XMLAttDef>*             fAttributeDeclRegistry;
    RefHashTableOf<ComplexTypeInfo>*       fComplexTypeRegistry;
    DatatypeValidatorFactory*              fDatatypeRegistry;
    NamespaceScope*                        fNamespaceScope;
};


// ---------------------------------------------------------------------------
//  SchemaGrammar: Getter methods
// ---------------------------------------------------------------------------
inline RefHash3KeysIdPoolEnumerator<SchemaElementDecl>
SchemaGrammar::getElemEnumerator() const
{
    return RefHash3KeysIdPoolEnumerator<SchemaElementDecl>(fElemDeclPool);
}

inline XMLCh* SchemaGrammar::getTargetNamespace() {
    return fTargetNamespace;
}

inline const XMLCh* SchemaGrammar::getTargetNamespace() const {
    return fTargetNamespace;
}

inline RefHashTableOf<XMLAttDef>* SchemaGrammar::getAttributeDeclRegistry() const {

    return fAttributeDeclRegistry;
}

inline RefHashTableOf<ComplexTypeInfo>*
SchemaGrammar::getComplexTypeRegistry() const {

    return fComplexTypeRegistry;
}

inline DatatypeValidatorFactory* SchemaGrammar::getDatatypeRegistry() const {

    return fDatatypeRegistry;
}

inline NamespaceScope* SchemaGrammar::getNamespaceScope() const {

    return fNamespaceScope;
}

// -----------------------------------------------------------------------
//  Setter methods
// -----------------------------------------------------------------------
inline void SchemaGrammar::setTargetNamespace(const XMLCh* const targetNamespace) {
    fTargetNamespace = XMLString::replicate(targetNamespace);
}

inline void
SchemaGrammar::setAttributeDeclRegistry(RefHashTableOf<XMLAttDef>* const attReg) {

    fAttributeDeclRegistry = attReg;
}

inline void
SchemaGrammar::setComplexTypeRegistry(RefHashTableOf<ComplexTypeInfo>* const other) {

    fComplexTypeRegistry = other;
}

inline void
SchemaGrammar::setDatatypeRegistry(DatatypeValidatorFactory* const dvRegistry) {

    fDatatypeRegistry = dvRegistry;
}

inline void SchemaGrammar::setNamespaceScope(NamespaceScope* const nsScope) {

    fNamespaceScope = nsScope;
}

// ---------------------------------------------------------------------------
//  SchemaGrammar: Virtual methods
// ---------------------------------------------------------------------------
inline Grammar::GrammarType SchemaGrammar::getGrammarType() {
    return Grammar::SchemaGrammarType;
}

// Element Decl
inline unsigned int SchemaGrammar::getElemId (const   unsigned int  uriId
                                              , const XMLCh* const    baseName
                                              , const XMLCh* const    qName
                                              , unsigned int          scope ) const
{
    //
    //  In this case, we don't return zero to mean 'not found', so we have to
    //  map it to the official not found value if we don't find it.
    //
    const SchemaElementDecl* decl = fElemDeclPool->getByKey(baseName, uriId, scope);
    if (!decl)
        return XMLElementDecl::fgInvalidElemId;
    return decl->getId();
}

inline const XMLElementDecl* SchemaGrammar::getElemDecl( const   unsigned int  uriId
                                              , const XMLCh* const    baseName
                                              , const XMLCh* const    qName
                                              , unsigned int          scope )   const
{
    return fElemDeclPool->getByKey(baseName, uriId, scope);
}

inline XMLElementDecl* SchemaGrammar::getElemDecl (const   unsigned int  uriId
                                              , const XMLCh* const    baseName
                                              , const XMLCh* const    qName
                                              , unsigned int          scope )
{
    return fElemDeclPool->getByKey(baseName, uriId, scope);
}

inline const XMLElementDecl* SchemaGrammar::getElemDecl(const unsigned int elemId) const
{
    // Look up this element decl by id
    return fElemDeclPool->getById(elemId);
}

inline XMLElementDecl* SchemaGrammar::getElemDecl(const unsigned int elemId)
{
    // Look up this element decl by id
    return fElemDeclPool->getById(elemId);
}

inline unsigned int SchemaGrammar::putElemDecl (XMLElementDecl* const elemDecl)   const
{
    return fElemDeclPool->put(elemDecl->getBaseName(), elemDecl->getURI(), ((SchemaElementDecl* )elemDecl)->getEnclosingScope(), (SchemaElementDecl*) elemDecl);
}

// Notation Decl
inline const XMLNotationDecl* SchemaGrammar::getNotationDecl(const XMLCh* const notName) const
{
    return fNotationDeclPool->getByKey(notName);
}

inline XMLNotationDecl* SchemaGrammar::getNotationDecl(const XMLCh* const notName)
{
    return fNotationDeclPool->getByKey(notName);
}

inline unsigned int SchemaGrammar::putNotationDecl(XMLNotationDecl* const notationDecl)   const
{
    return fNotationDeclPool->put(notationDecl);
}

#endif
