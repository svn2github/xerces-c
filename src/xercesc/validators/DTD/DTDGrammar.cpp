/*
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 1999-2001 The Apache Software Foundation.  All rights
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
 * Revision 1.5  2003/05/15 18:54:50  knoaman
 * Partial implementation of the configurable memory manager.
 *
 * Revision 1.4  2002/12/04 02:47:25  knoaman
 * scanner re-organization.
 *
 * Revision 1.3  2002/11/04 14:50:40  tng
 * C++ Namespace Support.
 *
 * Revision 1.2  2002/07/11 18:19:28  knoaman
 * Grammar caching/preparsing - initial implementation.
 *
 * Revision 1.1.1.1  2002/02/01 22:22:43  peiyongz
 * sane_include
 *
 * Revision 1.4  2001/09/14 14:50:22  tng
 * Schema: Fix some wildcard bugs, and some retrieving qualified/unqualified element decl problems.
 *
 * Revision 1.3  2001/05/11 13:27:08  tng
 * Copyright update.
 *
 * Revision 1.2  2001/04/19 18:17:20  tng
 * Schema: SchemaValidator update, and use QName in Content Model
 *
 * Revision 1.1  2001/03/21 21:56:20  tng
 * Schema: Add Schema Grammar, Schema Validator, and split the DTDValidator into DTDValidator, DTDScanner, and DTDGrammar.
 *
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/validators/DTD/DTDGrammar.hpp>

XERCES_CPP_NAMESPACE_BEGIN

//---------------------------------------------------------------------------
//  DTDGrammar: Constructors and Destructor
// ---------------------------------------------------------------------------
DTDGrammar::DTDGrammar(MemoryManager* const manager) :
    fMemoryManager(manager)
    , fElemDeclPool(0)
    , fElemNonDeclPool(0)
    , fEntityDeclPool(0)
    , fNotationDeclPool(0)
    , fValidated(false)
{
    //
    //  Init all the pool members.
    //
    //  <TBD> Investigate what the optimum values would be for the various
    //  pools.
    //
    fElemDeclPool = new (fMemoryManager) NameIdPool<DTDElementDecl>(109);
    fElemNonDeclPool = new (fMemoryManager) NameIdPool<DTDElementDecl>(29);
    fEntityDeclPool = new (fMemoryManager) NameIdPool<DTDEntityDecl>(109);
    fNotationDeclPool = new (fMemoryManager) NameIdPool<XMLNotationDecl>(109);

    //
    //  Call our own reset method. This lets us have the pool setup stuff
    //  done in just one place (because this stame setup stuff has to be
    //  done every time we are reset.)
    //
    reset();
}

DTDGrammar::~DTDGrammar()
{
    delete fElemDeclPool;
    delete fElemNonDeclPool;
    delete fEntityDeclPool;
    delete fNotationDeclPool;
}

// -----------------------------------------------------------------------
//  Virtual methods
// -----------------------------------------------------------------------
XMLElementDecl* DTDGrammar::findOrAddElemDecl (const   unsigned int    uriId
        , const XMLCh* const    baseName
        , const XMLCh* const    prefixName
        , const XMLCh* const    qName
        , unsigned int          scope
        ,       bool&           wasAdded )
{
    // See it it exists
    DTDElementDecl* retVal = (DTDElementDecl*) getElemDecl(uriId, baseName, qName, scope);

    // if not, then add this in
    if (!retVal)
    {
        retVal = new (fMemoryManager) DTDElementDecl
        (
            qName
            , uriId
            , DTDElementDecl::Any
            , fMemoryManager
        );
        const unsigned int elemId = fElemNonDeclPool->put(retVal);
        retVal->setId(elemId);
        wasAdded = true;
    }
     else
    {
        wasAdded = false;
    }
    return retVal;
}

XMLElementDecl* DTDGrammar::putElemDecl (const   unsigned int    uriId
        , const XMLCh* const    baseName
        , const XMLCh* const    prefixName
        , const XMLCh* const    qName
        , unsigned int          scope
        , const bool            notDeclared)
{
    DTDElementDecl* retVal = new (fMemoryManager) DTDElementDecl
    (
        qName
        , uriId
        , DTDElementDecl::Any
        , fMemoryManager
    );
    const unsigned int elemId = (notDeclared) ? fElemNonDeclPool->put(retVal)
                                              : fElemDeclPool->put(retVal);
    retVal->setId(elemId);
    return retVal;
}

void DTDGrammar::reset()
{
    //
    //  We need to reset all of the pools.
    //
    fElemDeclPool->removeAll();
    fElemNonDeclPool->removeAll();
    fNotationDeclPool->removeAll();
    resetEntityDeclPool();
    fValidated = false;
}

void DTDGrammar::resetEntityDeclPool() {

    fEntityDeclPool->removeAll();
    //
    //  Add the default entity entries for the character refs that must always
    //  be present. We indicate that they are from the internal subset. They
    //  aren't really, but they have to look that way so that they are still
    //  valid for use within a standalone document.
    //
    //  We also mark them as special char entities, which allows them to be
    //  used in places whether other non-numeric general entities cannot.
    //
    fEntityDeclPool->put(new DTDEntityDecl(XMLUni::fgAmp, chAmpersand, true, true));
    fEntityDeclPool->put(new DTDEntityDecl(XMLUni::fgLT, chOpenAngle, true, true));
    fEntityDeclPool->put(new DTDEntityDecl(XMLUni::fgGT, chCloseAngle, true, true));
    fEntityDeclPool->put(new DTDEntityDecl(XMLUni::fgQuot, chDoubleQuote, true, true));
    fEntityDeclPool->put(new DTDEntityDecl(XMLUni::fgApos, chSingleQuote, true, true));
}

XERCES_CPP_NAMESPACE_END
