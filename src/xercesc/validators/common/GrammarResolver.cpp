/*
 * The Apache Software License, Version 1.1
 *
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
 *    permission, please contact apache@apache.org.
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
 * individuals on behalf of the Apache Software Foundation and was
 * originally based on software copyright (c) 2001, International
 * Business Machines, Inc., http://www.apache.org.  For more
 * information on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

/*
 * $Log$
 * Revision 1.8  2003/05/18 14:02:06  knoaman
 * Memory manager implementation: pass per instance manager.
 *
 * Revision 1.7  2003/05/16 21:43:20  knoaman
 * Memory manager implementation: Modify constructors to pass in the memory manager.
 *
 * Revision 1.6  2003/05/15 18:48:27  knoaman
 * Partial implementation of the configurable memory manager.
 *
 * Revision 1.5  2002/11/04 14:54:58  tng
 * C++ Namespace Support.
 *
 * Revision 1.4  2002/09/24 19:48:39  tng
 * Performance: use XMLString::equals instead of XMLString::compareString
 *
 * Revision 1.3  2002/07/12 14:35:37  knoaman
 * Add an error message and use it in the scanner.
 *
 * Revision 1.2  2002/07/11 18:17:43  knoaman
 * Grammar caching/preparsing - initial implementation.
 *
 * Revision 1.1.1.1  2002/02/01 22:22:38  peiyongz
 * sane_include
 *
 * Revision 1.5  2001/08/28 19:20:54  tng
 * Schema: xsi:type support
 *
 * Revision 1.4  2001/07/24 18:33:13  knoaman
 * Added support for <group> + extra constraint checking for complexType
 *
 * Revision 1.3  2001/06/07 20:58:39  tng
 * Fix no newline at the end warning.  By Pei Yong Zhang.
 *
 * Revision 1.2  2001/05/11 13:27:19  tng
 * Copyright update.
 *
 * Revision 1.1  2001/03/21 21:56:28  tng
 * Schema: Add Schema Grammar, Schema Validator, and split the DTDValidator into DTDValidator, DTDScanner, and DTDGrammar.
 *
 */

#include <xercesc/validators/common/GrammarResolver.hpp>
#include <xercesc/framework/XMLBuffer.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/validators/schema/SchemaGrammar.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  GrammarResolver: Constructor and Destructor
// ---------------------------------------------------------------------------
GrammarResolver::GrammarResolver(MemoryManager* const manager) :
    fCacheGrammar(false)
    , fUseCachedGrammar(false)
    , fStringPool(109, manager)
    , fGrammarRegistry(0)
    , fCachedGrammarRegistry(0)
    , fDataTypeReg(0)
    , fMemoryManager(manager)	 
{
    fGrammarRegistry = new (manager) RefHashTableOf<Grammar>(29, true,  manager);
}

GrammarResolver::~GrammarResolver()
{
   delete fGrammarRegistry;
   delete fCachedGrammarRegistry;
   if (fDataTypeReg)
      delete fDataTypeReg;
}

// ---------------------------------------------------------------------------
//  GrammarResolver: Getter methods
// ---------------------------------------------------------------------------
DatatypeValidator*
GrammarResolver::getDatatypeValidator(const XMLCh* const uriStr,
                                      const XMLCh* const localPartStr) {

    DatatypeValidator* dv = 0;

    if (XMLString::equals(uriStr, SchemaSymbols::fgURI_SCHEMAFORSCHEMA)) {

        if (!fDataTypeReg) {

            fDataTypeReg = new (fMemoryManager) DatatypeValidatorFactory(fMemoryManager);
            fDataTypeReg->expandRegistryToFullSchemaSet();
        }

        dv = fDataTypeReg->getDatatypeValidator(localPartStr);
    }
    else {

        Grammar* grammar = getGrammar(uriStr);

        if (grammar && grammar->getGrammarType() == Grammar::SchemaGrammarType) {

            XMLBuffer nameBuf(128, fMemoryManager);

            nameBuf.set(uriStr);
            nameBuf.append(chComma);
            nameBuf.append(localPartStr);

            dv = ((SchemaGrammar*) grammar)->getDatatypeRegistry()->getDatatypeValidator(nameBuf.getRawBuffer());
        }
    }

    return dv;
}

Grammar* GrammarResolver::getGrammar( const XMLCh* const nameSpaceKey )
{
    if (!nameSpaceKey) {
        return 0;
    }

    Grammar* aGrammar = fGrammarRegistry->get(nameSpaceKey);

    if (!aGrammar && fUseCachedGrammar && fCachedGrammarRegistry)
        aGrammar = fCachedGrammarRegistry->get(nameSpaceKey);

    return aGrammar;
}

RefHashTableOfEnumerator<Grammar>
GrammarResolver::getGrammarEnumerator() const
{
    return RefHashTableOfEnumerator<Grammar>(fGrammarRegistry);
}

bool GrammarResolver::containsNameSpace( const XMLCh* const nameSpaceKey )
{
   return fGrammarRegistry->containsKey( nameSpaceKey );
}

void GrammarResolver::putGrammar( const XMLCh* const nameSpaceKey, Grammar* const grammarToAdopt ){

   if (fCacheGrammar)
       fCachedGrammarRegistry->put((void*) nameSpaceKey, grammarToAdopt);

   fGrammarRegistry->put( (void*) nameSpaceKey, grammarToAdopt );
}


// ---------------------------------------------------------------------------
//  GrammarResolver: methods
// ---------------------------------------------------------------------------
void GrammarResolver::reset() {
   fGrammarRegistry->removeAll();
}

void GrammarResolver::resetCachedGrammar()
{
    if (fCachedGrammarRegistry)
        fCachedGrammarRegistry->removeAll();
}

void GrammarResolver::cacheGrammars()
{
    RefHashTableOfEnumerator<Grammar> grammarEnum(fGrammarRegistry);
    ValueVectorOf<XMLCh*> keys(8, fMemoryManager);
    unsigned int keyCount = 0;

    //Check if a grammar has already been cached.
    while (grammarEnum.hasMoreElements()) {

        XMLCh* grammarKey = (XMLCh*) grammarEnum.nextElementKey();

        if (fCachedGrammarRegistry && fCachedGrammarRegistry->containsKey(grammarKey)) {
            ThrowXML(RuntimeException, XMLExcepts::GC_ExistingGrammar);
        }

        keys.addElement(grammarKey);
        keyCount++;
    }

    if (!fCachedGrammarRegistry)
        fCachedGrammarRegistry = new (fMemoryManager) RefHashTableOf<Grammar>(29, true, fMemoryManager);

    // Cache
    for (unsigned int i=0; i<keyCount; i++) {

        XMLCh* grammarKey = keys.elementAt(i);
        Grammar* grammar = fGrammarRegistry->orphanKey(grammarKey);
        fCachedGrammarRegistry->put((void*) grammarKey, grammar);
    }
}


// ---------------------------------------------------------------------------
//  GrammarResolver: Setter methods
// ---------------------------------------------------------------------------
void GrammarResolver::cacheGrammarFromParse(const bool aValue) {

    if (aValue && !fCachedGrammarRegistry) {
        fCachedGrammarRegistry = new (fMemoryManager) RefHashTableOf<Grammar>(29, true, fMemoryManager);
    }

    fCacheGrammar = aValue;
    fGrammarRegistry->removeAll();
    fGrammarRegistry->setAdoptElements(!fCacheGrammar);
}

XERCES_CPP_NAMESPACE_END
