/*
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2001-2002 The Apache Software Foundation.  All rights
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
#include <xercesc/validators/schema/TraverseSchema.hpp>
#include <xercesc/framework/XMLEntityHandler.hpp>
#include <xercesc/framework/XMLValidityCodes.hpp>
#include <xercesc/validators/schema/identity/IC_Key.hpp>
#include <xercesc/validators/schema/identity/IC_KeyRef.hpp>
#include <xercesc/validators/schema/identity/IC_Unique.hpp>
#include <xercesc/validators/schema/identity/IC_Field.hpp>
#include <xercesc/validators/schema/identity/IC_Selector.hpp>
#include <xercesc/validators/schema/identity/XercesXPath.hpp>
#include <xercesc/util/XMLStringTokenizer.hpp>
#include <xercesc/validators/schema/XUtil.hpp>
#include <xercesc/validators/common/GrammarResolver.hpp>
#include <xercesc/internal/XMLReader.hpp>
#include <xercesc/validators/schema/ComplexTypeInfo.hpp>
#include <xercesc/validators/schema/NamespaceScope.hpp>
#include <xercesc/validators/schema/SchemaAttDefList.hpp>
#include <xercesc/internal/XMLScanner.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/framework/URLInputSource.hpp>
#include <xercesc/validators/schema/identity/XPathException.hpp>
#include <xercesc/validators/schema/GeneralAttributeCheck.hpp>
#include <xercesc/validators/schema/XercesGroupInfo.hpp>
#include <xercesc/validators/schema/XercesAttGroupInfo.hpp>
#include <xercesc/validators/schema/XSDLocator.hpp>
#include <xercesc/validators/schema/XSDDOMParser.hpp>
#include <xercesc/util/HashPtr.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/dom/impl/XSDElementNSImpl.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  TraverseSchema: Local declaration
// ---------------------------------------------------------------------------
typedef RefVectorOf<DatatypeValidator> DVRefVector;


// ---------------------------------------------------------------------------
//  TraverseSchema: Static member data
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
//  TraverseSchema: Local const data
// ---------------------------------------------------------------------------
const XMLCh fgXMLNS_Str[] =
{
    chLatin_x, chLatin_m, chLatin_l, chLatin_n, chLatin_s, chColon, chNull
};

const XMLCh fgAnonSNamePrefix[] =
{
    chLatin_S, chNull
};

const XMLCh fgAnonCNamePrefix[] =
{
    chLatin_C, chNull
};

const XMLCh fgUnbounded[] =
{
    chLatin_u, chLatin_n, chLatin_b, chLatin_o, chLatin_u, chLatin_n, chLatin_d,
    chLatin_e, chLatin_d, chNull
};

const XMLCh fgSkip[] =
{
    chLatin_s, chLatin_k, chLatin_i, chLatin_p, chNull
};

const XMLCh fgLax[] =
{
    chLatin_l, chLatin_a, chLatin_x, chNull
};

const XMLCh fgStrict[] =
{
    chLatin_s, chLatin_t, chLatin_r, chLatin_i, chLatin_c, chLatin_t, chNull
};

const XMLCh fgValueOne[] =
{
    chDigit_1, chNull
};

const XMLCh fgValueZero[] =
{
    chDigit_0, chNull
};

const XMLCh fgForwardSlash[] =
{
    chForwardSlash, chNull
};

const XMLCh fgDot[] =
{
    chPeriod, chNull
};

const XMLCh fgDotForwardSlash[] =
{
    chPeriod, chForwardSlash, chNull
};

const XMLCh* fgIdentityConstraints[] =
{
    SchemaSymbols::fgELT_UNIQUE,
    SchemaSymbols::fgELT_KEY,
    SchemaSymbols::fgELT_KEYREF
};

// Flags for global declaration
enum {
    ENUM_ELT_SIMPLETYPE,
    ENUM_ELT_COMPLEXTYPE,
    ENUM_ELT_ELEMENT,
    ENUM_ELT_ATTRIBUTE,
    ENUM_ELT_ATTRIBUTEGROUP,
    ENUM_ELT_GROUP,
    ENUM_ELT_SIZE
};

// ---------------------------------------------------------------------------
//  TraverseSchema: Constructors and Destructor
// ---------------------------------------------------------------------------
TraverseSchema::TraverseSchema( DOMElement* const    schemaRoot
                              , XMLStringPool* const   uriStringPool
                              , SchemaGrammar* const   schemaGrammar
                              , GrammarResolver* const grammarResolver
                              , XMLScanner* const      xmlScanner
                              , const XMLCh* const     schemaURL
                              , XMLEntityHandler* const  entityHandler
                              , XMLErrorReporter* const errorReporter
                              , MemoryManager* const    manager)
    : fFullConstraintChecking(false)
    , fTargetNSURI(-1)
    , fEmptyNamespaceURI(-1)
    , fCurrentScope(Grammar::TOP_LEVEL_SCOPE)
    , fScopeCount(0)
    , fAnonXSTypeCount(0)
    , fCircularCheckIndex(0)
    , fTargetNSURIString(0)
    , fDatatypeRegistry(0)
    , fGrammarResolver(grammarResolver)
    , fSchemaGrammar(schemaGrammar)
    , fEntityHandler(entityHandler)
    , fErrorReporter(errorReporter)
    , fURIStringPool(uriStringPool)
    , fStringPool(0)
    , fScanner(xmlScanner)
    , fNamespaceScope(0)
    , fAttributeDeclRegistry(0)
    , fComplexTypeRegistry(0)
    , fGroupRegistry(0)
    , fAttGroupRegistry(0)
    , fSchemaInfoList(0)
    , fSchemaInfo(0)
    , fCurrentGroupInfo(0)
    , fCurrentAttGroupInfo(0)
    , fCurrentComplexType(0)
    , fCurrentTypeNameStack(0)
    , fCurrentGroupStack(0)
    , fIC_NamespaceDepth(0)
    , fIC_Elements(0)
    , fDeclStack(0)
    , fGlobalDeclarations(0)
    , fNotationRegistry(0)
    , fRedefineComponents(0)
    , fIdentityConstraintNames(0)
    , fValidSubstitutionGroups(0)
    , fIC_NodeListNS(0)
    , fIC_ElementsNS(0)
    , fIC_NamespaceDepthNS(0)
    , fParser(0)
    , fPreprocessedNodes(0)
    , fLocator(0)
    , fMemoryManager(manager)
    , fAttributeCheck(manager)
{

    try {

        if (fGrammarResolver && schemaRoot && fURIStringPool) {

            init();
            preprocessSchema(schemaRoot, schemaURL);
            doTraverseSchema(schemaRoot);
        }
    }
    catch(...) {

        cleanUp();
        throw;
    }
}


TraverseSchema::~TraverseSchema()
{
   cleanUp();
}


// ---------------------------------------------------------------------------
//  TraverseSchema: Traversal methods
// ---------------------------------------------------------------------------
void TraverseSchema::doTraverseSchema(const DOMElement* const schemaRoot) {

    // process children nodes
    processChildren(schemaRoot);

    // Handle identity constraints - keyref
    if (fIC_ElementsNS && fIC_ElementsNS->containsKey(fTargetNSURIString)) {

        fIC_Elements = fIC_ElementsNS->get(fTargetNSURIString);
        fIC_NamespaceDepth = fIC_NamespaceDepthNS->get(fTargetNSURIString);

        unsigned int icListSize = fIC_Elements->size();

        for (unsigned int i=0; i < icListSize; i++) {

            SchemaElementDecl* curElem = fIC_Elements->elementAt(i);
            ValueVectorOf<DOMElement*>* icNodes =  fIC_NodeListNS->get(curElem);
            unsigned int icNodesSize = icNodes->size();
            unsigned int scopeDepth = fIC_NamespaceDepth->elementAt(i);

            for (unsigned int j = 0; j < icNodesSize; j++) {
                traverseKeyRef(icNodes->elementAt(j), curElem, scopeDepth);
            }
        }
    }

    fSchemaInfo->setProcessed();
}

void TraverseSchema::preprocessSchema(DOMElement* const schemaRoot,
                                      const XMLCh* const schemaURL) {

    // Make sure namespace binding is defaulted
    const XMLCh* rootPrefix = schemaRoot->getPrefix();

    if (rootPrefix == 0 || !*rootPrefix) {

		const XMLCh* xmlnsStr = schemaRoot->getAttribute(XMLUni::fgXMLNSString);

        if (!xmlnsStr || !*xmlnsStr) {
            schemaRoot->setAttribute(XMLUni::fgXMLNSString, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
        }
    }

    // Set schemaGrammar data and add it to GrammarResolver
    // For complex type registry, attribute decl registry , group/attGroup
    // and namespace mapping, needs to check whether the passed in
    // Grammar was a newly instantiated one.
    fComplexTypeRegistry = fSchemaGrammar->getComplexTypeRegistry();

    if (fComplexTypeRegistry == 0 ) {

        fComplexTypeRegistry = new (fMemoryManager) RefHashTableOf<ComplexTypeInfo>(29);
        fSchemaGrammar->setComplexTypeRegistry(fComplexTypeRegistry);
    }

    fGroupRegistry = fSchemaGrammar->getGroupInfoRegistry();

    if (fGroupRegistry == 0 ) {

        fGroupRegistry = new (fMemoryManager) RefHashTableOf<XercesGroupInfo>(13);
        fSchemaGrammar->setGroupInfoRegistry(fGroupRegistry);
    }

    fAttGroupRegistry = fSchemaGrammar->getAttGroupInfoRegistry();

    if (fAttGroupRegistry == 0 ) {

        fAttGroupRegistry = new (fMemoryManager) RefHashTableOf<XercesAttGroupInfo>(13);
        fSchemaGrammar->setAttGroupInfoRegistry(fAttGroupRegistry);
    }

    fAttributeDeclRegistry = fSchemaGrammar->getAttributeDeclRegistry();

    if (fAttributeDeclRegistry == 0) {

        fAttributeDeclRegistry = new (fMemoryManager) RefHashTableOf<XMLAttDef>(29);
        fSchemaGrammar->setAttributeDeclRegistry(fAttributeDeclRegistry);
    }

    fNamespaceScope = fSchemaGrammar->getNamespaceScope();

    if (fNamespaceScope == 0) {

        fNamespaceScope = new (fMemoryManager) NamespaceScope();
        fNamespaceScope->reset(fEmptyNamespaceURI);
        fSchemaGrammar->setNamespaceScope(fNamespaceScope);
    }

    fValidSubstitutionGroups = fSchemaGrammar->getValidSubstitutionGroups();

    if (!fValidSubstitutionGroups) {

        fValidSubstitutionGroups = new (fMemoryManager) RefHash2KeysTableOf<ElemVector>(29);
        fSchemaGrammar->setValidSubstitutionGroups(fValidSubstitutionGroups);
    }

    //Retrieve the targetnamespace URI information
    const XMLCh* targetNSURIStr = schemaRoot->getAttribute(SchemaSymbols::fgATT_TARGETNAMESPACE);
    fSchemaGrammar->setTargetNamespace(targetNSURIStr);

    fScopeCount = 0;
    fCurrentScope = Grammar::TOP_LEVEL_SCOPE;
    fTargetNSURIString = fSchemaGrammar->getTargetNamespace();
    fTargetNSURI = fURIStringPool->addOrFind(fTargetNSURIString);
    fGrammarResolver->putGrammar(fTargetNSURIString, fSchemaGrammar);
    fAttributeCheck.setIDRefList(fSchemaGrammar->getIDRefList());

    // Save current schema info
    SchemaInfo* currInfo = new (fMemoryManager) SchemaInfo(0, 0, 0, fTargetNSURI, fScopeCount,
                                          fNamespaceScope->increaseDepth(),
                                          XMLString::replicate(schemaURL),
                                          fTargetNSURIString, schemaRoot);

    if (fSchemaInfo) {
        fSchemaInfo->addSchemaInfo(currInfo, SchemaInfo::IMPORT);
    }

    fSchemaInfo = currInfo;
    fSchemaInfoList->put((void*) fSchemaInfo->getCurrentSchemaURL(), fSchemaInfo->getTargetNSURI(), fSchemaInfo);
    fSchemaInfo->addSchemaInfo(fSchemaInfo, SchemaInfo::INCLUDE);
    traverseSchemaHeader(schemaRoot);

    // preprocess chidren
    preprocessChildren(schemaRoot);
}


void TraverseSchema::traverseSchemaHeader(const DOMElement* const schemaRoot) {

    // Make sure that the root element is <xsd:schema>
    if (!XMLString::equals(schemaRoot->getLocalName(), SchemaSymbols::fgELT_SCHEMA)) {
        reportSchemaError(schemaRoot, XMLUni::fgXMLErrDomain, XMLErrs::InvalidXMLSchemaRoot);
    }

    // Make sure that the targetNamespace value is not empty string
    checkForEmptyTargetNamespace(schemaRoot);

    // -----------------------------------------------------------------------
    // Check Attributes
    // -----------------------------------------------------------------------
    fAttributeCheck.checkAttributes(schemaRoot, GeneralAttributeCheck::E_Schema, this, true);

    retrieveNamespaceMapping(schemaRoot);
    unsigned short elemAttrDefaultQualified = 0;

    if (XMLString::equals(schemaRoot->getAttribute(SchemaSymbols::fgATT_ELEMENTFORMDEFAULT),
                                  SchemaSymbols::fgATTVAL_QUALIFIED)) {
        elemAttrDefaultQualified |= Elem_Def_Qualified;
    }

    if (XMLString::equals(schemaRoot->getAttribute(SchemaSymbols::fgATT_ATTRIBUTEFORMDEFAULT),
                                  SchemaSymbols::fgATTVAL_QUALIFIED)) {
        elemAttrDefaultQualified |= Attr_Def_Qualified;
    }

    fSchemaInfo->setElemAttrDefaultQualified(elemAttrDefaultQualified);
    fSchemaInfo->setBlockDefault(parseBlockSet(schemaRoot, ES_Block, true));
    fSchemaInfo->setFinalDefault(parseFinalSet(schemaRoot, ECS_Final, true));
}


void TraverseSchema::traverseAnnotationDecl(const DOMElement* const annotationElem,
                                            const bool topLevel) {

    // -----------------------------------------------------------------------
    // Check Attributes
    // -----------------------------------------------------------------------
    fAttributeCheck.checkAttributes(annotationElem, GeneralAttributeCheck::E_Annotation, this, topLevel);

    for (DOMElement* child = XUtil::getFirstChildElement(annotationElem);
         child != 0;
         child = XUtil::getNextSiblingElement(child)) {

        const XMLCh* name = child->getLocalName();

        if (XMLString::equals(name, SchemaSymbols::fgELT_APPINFO)) {
            fAttributeCheck.checkAttributes(child, GeneralAttributeCheck::E_Appinfo, this);
        }
        else if (XMLString::equals(name, SchemaSymbols::fgELT_DOCUMENTATION)) {
            fAttributeCheck.checkAttributes(child, GeneralAttributeCheck::E_Documentation, this);
        }
        else {
            reportSchemaError(child, XMLUni::fgXMLErrDomain, XMLErrs::InvalidAnnotationContent);
        }
    }
}


/**
  * Traverse include
  *
  *    <include
  *        id = ID
  *        schemaLocation = anyURI
  *        {any attributes with non-schema namespace . . .}>
  *        Content: (annotation?)
  *    </include>
  */
void TraverseSchema::preprocessInclude(const DOMElement* const elem) {

    // ------------------------------------------------------------------
    // Check attributes
    // ------------------------------------------------------------------
    fAttributeCheck.checkAttributes(elem, GeneralAttributeCheck::E_Include, this, true);

    // ------------------------------------------------------------------
    // First, handle any ANNOTATION declaration
    // ------------------------------------------------------------------
    if (checkContent(elem, XUtil::getFirstChildElement(elem), true) != 0) {
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::OnlyAnnotationExpected);
    }

    // ------------------------------------------------------------------
    // Get 'schemaLocation' attribute
    // ------------------------------------------------------------------
    const XMLCh* schemaLocation = getElementAttValue(elem, SchemaSymbols::fgATT_SCHEMALOCATION);

    if (!schemaLocation || !*schemaLocation) {
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::DeclarationNoSchemaLocation, SchemaSymbols::fgELT_INCLUDE);
        return;
    }

    // ------------------------------------------------------------------
    // Resolve schema location
    // ------------------------------------------------------------------
    InputSource*         srcToFill = resolveSchemaLocation(schemaLocation);
    Janitor<InputSource> janSrc(srcToFill);

    // Nothing to do
    if (!srcToFill) {
        return;
    }

    const XMLCh* includeURL = srcToFill->getSystemId();
    SchemaInfo* includeSchemaInfo = fSchemaInfoList->get(includeURL, fTargetNSURI);

    if (includeSchemaInfo) {

        fSchemaInfo->addSchemaInfo(includeSchemaInfo, SchemaInfo::INCLUDE);
        return;
    }

    // ------------------------------------------------------------------
    // Parse input source
    // ------------------------------------------------------------------
    if (!fParser)
        fParser = new (fMemoryManager) XSDDOMParser;

    fParser->setValidationScheme(XercesDOMParser::Val_Never);
    fParser->setDoNamespaces(true);
    fParser->setUserEntityHandler(fEntityHandler);
    fParser->setUserErrorReporter(fErrorReporter);

    // Should just issue warning if the schema is not found
    const bool flag = srcToFill->getIssueFatalErrorIfNotFound();
    srcToFill->setIssueFatalErrorIfNotFound(false);

    fParser->parse(*srcToFill);

    // Reset the InputSource
    srcToFill->setIssueFatalErrorIfNotFound(flag);

    if (fParser->getSawFatal() && fScanner->getExitOnFirstFatal())
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::SchemaScanFatalError);

    // ------------------------------------------------------------------
    // Get root element
    // ------------------------------------------------------------------
    DOMDocument* document = fParser->getDocument();

    if (document) {

        DOMElement* root = document->getDocumentElement();

        if (root) {

            const XMLCh* targetNSURIString = root->getAttribute(SchemaSymbols::fgATT_TARGETNAMESPACE);

            // check to see if targetNameSpace is right
            if (*targetNSURIString
                && !XMLString::equals(targetNSURIString,fTargetNSURIString)){
                reportSchemaError(root, XMLUni::fgXMLErrDomain, XMLErrs::IncludeNamespaceDifference,
                                  schemaLocation, targetNSURIString);
                return;
            }

            // if targetNamespace is empty, change it to includ'g schema
            // targetNamespace
            if (!*targetNSURIString && root->getAttributeNode(XMLUni::fgXMLNSString) == 0
                && fTargetNSURI != fEmptyNamespaceURI) {
                root->setAttribute(XMLUni::fgXMLNSString, fTargetNSURIString);
            }

            // --------------------------------------------------------
            // Update schema information with included schema
            // --------------------------------------------------------
            SchemaInfo* saveInfo = fSchemaInfo;

            fSchemaInfo = new (fMemoryManager) SchemaInfo(0, 0, 0, fTargetNSURI, fScopeCount,
                                         fNamespaceScope->increaseDepth(),
                                         XMLString::replicate(includeURL),
                                         fTargetNSURIString, root);

            fSchemaInfoList->put((void*) fSchemaInfo->getCurrentSchemaURL(),
                                 fSchemaInfo->getTargetNSURI(), fSchemaInfo);
            fPreprocessedNodes->put((void*) elem, fSchemaInfo);
            saveInfo->addSchemaInfo(fSchemaInfo, SchemaInfo::INCLUDE);
            traverseSchemaHeader(root);
            preprocessChildren(root);
            fSchemaInfo = saveInfo;
        }
    }
}


void TraverseSchema::traverseInclude(const DOMElement* const elem) {

    SchemaInfo* includeInfo = fPreprocessedNodes->get(elem);

    if (includeInfo) {

        SchemaInfo* saveInfo = fSchemaInfo;

        fSchemaInfo = includeInfo;
        processChildren(includeInfo->getRoot());
        fSchemaInfo = saveInfo;
    }
}


/**
  * Traverse import
  *
  *    <import
  *        id = ID
  *        namespace = anyURI
  *        schemaLocation = anyURI
  *        {any attributes with non-schema namespace . . .}>
  *        Content: (annotation?)
  *    </import>
  */
void TraverseSchema::preprocessImport(const DOMElement* const elem) {

    // ------------------------------------------------------------------
    // Check attributes
    // ------------------------------------------------------------------
    fAttributeCheck.checkAttributes(elem, GeneralAttributeCheck::E_Import, this, true);

    // ------------------------------------------------------------------
    // First, handle any ANNOTATION declaration
    // ------------------------------------------------------------------
    if (checkContent(elem, XUtil::getFirstChildElement(elem), true) != 0) {
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::OnlyAnnotationExpected);
    }

    // ------------------------------------------------------------------
    // Handle 'namespace' attribute
    // ------------------------------------------------------------------
    const XMLCh* nameSpace = getElementAttValue(elem, SchemaSymbols::fgATT_NAMESPACE);

    if (XMLString::equals(nameSpace, fTargetNSURIString)) {

        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::Import_1_1);
        return;
    }

    if ((!nameSpace || !*nameSpace) && fTargetNSURI == fEmptyNamespaceURI) {

        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::Import_1_2);
        return;
    }

    // ------------------------------------------------------------------
    // Resolve namespace to a grammar
    // ------------------------------------------------------------------	
    Grammar* aGrammar = (nameSpace) ? fGrammarResolver->getGrammar(nameSpace) : 0;
    bool grammarFound = (aGrammar && (aGrammar->getGrammarType() == Grammar::SchemaGrammarType));

    if (grammarFound) {
        fSchemaInfo->addImportedNS(fURIStringPool->addOrFind(nameSpace));
    }

    // ------------------------------------------------------------------
    // Get 'schemaLocation' attribute
    // ------------------------------------------------------------------
    const XMLCh* schemaLocation = getElementAttValue(elem, SchemaSymbols::fgATT_SCHEMALOCATION);

    if (!schemaLocation || !*schemaLocation) {
        return;
    }

    // ------------------------------------------------------------------
    // Resolve schema location
    // ------------------------------------------------------------------
    InputSource*         srcToFill = resolveSchemaLocation(schemaLocation);
    Janitor<InputSource> janSrc(srcToFill);

    // Nothing to do
    if (!srcToFill) {
        return;
    }

    const XMLCh* importURL = srcToFill->getSystemId();
    SchemaInfo* importSchemaInfo = 0;

    if (nameSpace)
        importSchemaInfo = fSchemaInfoList->get(importURL, fURIStringPool->addOrFind(nameSpace));
    else
        importSchemaInfo = fSchemaInfoList->get(importURL, fEmptyNamespaceURI);

    if (importSchemaInfo) {

        fSchemaInfo->addSchemaInfo(importSchemaInfo, SchemaInfo::IMPORT);
        return;
    }

    if (grammarFound) {
        return;
    }

    // ------------------------------------------------------------------
    // Parse input source
    // ------------------------------------------------------------------
    if (!fParser)
        fParser = new (fMemoryManager) XSDDOMParser;

    fParser->setValidationScheme(XercesDOMParser::Val_Never);
    fParser->setDoNamespaces(true);
    fParser->setUserEntityHandler(fEntityHandler);
    fParser->setUserErrorReporter(fErrorReporter);

    // Should just issue warning if the schema is not found
    const bool flag = srcToFill->getIssueFatalErrorIfNotFound();
    srcToFill->setIssueFatalErrorIfNotFound(false);

    fParser->parse(*srcToFill) ;

    // Reset the InputSource
    srcToFill->setIssueFatalErrorIfNotFound(flag);

    if (fParser->getSawFatal() && fScanner->getExitOnFirstFatal())
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::SchemaScanFatalError);

    // ------------------------------------------------------------------
    // Get root element
    // ------------------------------------------------------------------
    DOMDocument* document = fParser->getDocument();

    if (document) {

        DOMElement* root = document->getDocumentElement();

        if (!root) {
            return;
        }

        const XMLCh* targetNSURIString = root->getAttribute(SchemaSymbols::fgATT_TARGETNAMESPACE);

        if (!XMLString::equals(targetNSURIString, nameSpace)) {
            reportSchemaError(root, XMLUni::fgXMLErrDomain, XMLErrs::ImportNamespaceDifference,
                              schemaLocation, targetNSURIString, nameSpace);
        }
        else {

            // --------------------------------------------------------
            // Preprocess new schema
            // --------------------------------------------------------
            SchemaInfo* saveInfo = fSchemaInfo;
            fSchemaGrammar = new (fMemoryManager) SchemaGrammar(fMemoryManager);
            preprocessSchema(root, importURL);
            fPreprocessedNodes->put((void*) elem, fSchemaInfo);

            // --------------------------------------------------------
            // Restore old schema information
            // --------------------------------------------------------
            restoreSchemaInfo(saveInfo, SchemaInfo::IMPORT);
        }
    }
}


void TraverseSchema::traverseImport(const DOMElement* const elem) {

    SchemaInfo* importInfo = fPreprocessedNodes->get(elem);

    if (importInfo) {

        // --------------------------------------------------------
        // Traverse new schema
        // --------------------------------------------------------
        SchemaInfo* saveInfo = fSchemaInfo;

        restoreSchemaInfo(importInfo, SchemaInfo::IMPORT);
        doTraverseSchema(importInfo->getRoot());

        // --------------------------------------------------------
        // Restore old schema information
        // --------------------------------------------------------
        restoreSchemaInfo(saveInfo, SchemaInfo::IMPORT);
    }
}


/**
  * Traverse redefine declaration
  *
  *    <redefine>
  *        schemaLocation = uriReference
  *        {any attributes with non-schema namespace . . .}>
  *        Content: (annotation | (
  *            attributeGroup | complexType | group | simpleType))*
  *    </redefine>
  */
void TraverseSchema::preprocessRedefine(const DOMElement* const redefineElem) {

    // ------------------------------------------------------------------
    // Check attributes
    // ------------------------------------------------------------------
    fAttributeCheck.checkAttributes(redefineElem, GeneralAttributeCheck::E_Redefine, this, true);

    // First, we look through the children of redefineElem. Each one will
    // correspond to an element of the redefined schema that we need to
    // redefine.  To do this, we rename the element of the redefined schema,
    // and rework the base or ref tag of the kid we're working on to refer to
    // the renamed group or derive the renamed type.  Once we've done this, we
    // actually go through the schema being redefined and convert it to a
    // grammar. Only then do we run through redefineDecl's kids and put them
    // in the grammar.
    SchemaInfo* redefiningInfo = fSchemaInfo;

    if (!openRedefinedSchema(redefineElem)) {

        redefiningInfo->addFailedRedefine(redefineElem);
        return;
    }

    if (!fRedefineComponents) {
        fRedefineComponents = new (fMemoryManager) RefHash2KeysTableOf<XMLCh>(13, false);
    }

    SchemaInfo* redefinedInfo = fSchemaInfo;
    renameRedefinedComponents(redefineElem, redefiningInfo, redefinedInfo);

    // Now we have to preprocess our nicely-renamed schemas.
    if (fPreprocessedNodes->containsKey(redefineElem)) {

        fSchemaInfo = redefinedInfo;
        preprocessChildren(fSchemaInfo->getRoot());
    }

    fSchemaInfo = redefiningInfo;
}

void TraverseSchema::traverseRedefine(const DOMElement* const redefineElem) {

    SchemaInfo* saveInfo = fSchemaInfo;
    SchemaInfo* redefinedInfo = fPreprocessedNodes->get(redefineElem);

    if (redefinedInfo) {

        // Now we have to march through our nicely-renamed schemas. When
        // we do these traversals other <redefine>'s may perhaps be
        // encountered; we leave recursion to sort this out.
        fSchemaInfo = redefinedInfo;
        processChildren(fSchemaInfo->getRoot());
        fSchemaInfo = saveInfo;

        // Now traverse our own <redefine>
        processChildren(redefineElem);
    }
}


/**
  * Traverse the Choice, Sequence declaration
  *
  *    <choice-sequqnce
  *        id = ID
  *        maxOccurs = (nonNegativeInteger | unbounded)  : 1
  *        minOccurs = nonNegativeInteger : 1
  *        Content: (annotation?, (element | group | choice | sequence | any)*)
  *    </choice-sequence>
  */
ContentSpecNode*
TraverseSchema::traverseChoiceSequence(const DOMElement* const elem,
                                       const int modelGroupType)
{

    // ------------------------------------------------------------------
    // Check attributes
    // ------------------------------------------------------------------
    fAttributeCheck.checkAttributes(elem, GeneralAttributeCheck::E_Sequence, this);

    // ------------------------------------------------------------------
    // Process contents
    // ------------------------------------------------------------------
    DOMElement* child = checkContent(elem, XUtil::getFirstChildElement(elem), true);
    ContentSpecNode* left = 0;
    ContentSpecNode* right = 0;
    bool hadContent = false;

    for (; child != 0; child = XUtil::getNextSiblingElement(child)) {

        ContentSpecNode* contentSpecNode = 0;
        bool seeParticle = false;
        const XMLCh* childName = child->getLocalName();

        if (XMLString::equals(childName, SchemaSymbols::fgELT_ELEMENT)) {

            QName* eltQName = traverseElementDecl(child);

            if (eltQName == 0) {
                continue;
            }

            contentSpecNode = new (fMemoryManager) ContentSpecNode(eltQName, false);
            seeParticle = true;
        }
        else if (XMLString::equals(childName, SchemaSymbols::fgELT_GROUP)) {

            XercesGroupInfo* grpInfo = traverseGroupDecl(child, false);

            if (!grpInfo) {
                continue;
            }

            contentSpecNode = grpInfo->getContentSpec();

            if (!contentSpecNode) {
                continue;
            }

            if (contentSpecNode->hasAllContent()) {

                reportSchemaError(child, XMLUni::fgXMLErrDomain, XMLErrs::AllContentLimited);
                continue;
            }

            contentSpecNode = new (fMemoryManager) ContentSpecNode(*contentSpecNode);
            seeParticle = true;
        }
        else if (XMLString::equals(childName, SchemaSymbols::fgELT_CHOICE)) {

            contentSpecNode = traverseChoiceSequence(child,ContentSpecNode::Choice);
            seeParticle = true;
        }
        else if (XMLString::equals(childName, SchemaSymbols::fgELT_SEQUENCE)) {

            contentSpecNode = traverseChoiceSequence(child,ContentSpecNode::Sequence);
            seeParticle = true;
        }
        else if (XMLString::equals(childName, SchemaSymbols::fgELT_ANY)) {

            contentSpecNode = traverseAny(child);
            seeParticle = true;
        }
        else {
            reportSchemaError(child, XMLUni::fgValidityDomain, XMLValid::GroupContentRestricted, childName);
        }

        if (contentSpecNode) {
            hadContent = true;
        }

        if (seeParticle) {
            checkMinMax(contentSpecNode, child, Not_All_Context);
        }

        if (left == 0) {
            left = contentSpecNode;
        }
        else if (right == 0) {
            right = contentSpecNode;
        }
        else {
            left = new (fMemoryManager) ContentSpecNode((ContentSpecNode::NodeTypes) modelGroupType, left, right);
            right = contentSpecNode;
        }
    }

    if (hadContent) {
        left = new (fMemoryManager) ContentSpecNode((ContentSpecNode::NodeTypes) modelGroupType, left, right);
    }

    return left;
}

/**
  * Traverse SimpleType declaration:
  * <simpleType
  *     id = ID
  *     name = NCName>
  *     Content: (annotation? , ((list | restriction | union)))
  * </simpleType>
  *
  * traverse <list>|<restriction>|<union>
  */
DatatypeValidator*
TraverseSchema::traverseSimpleTypeDecl(const DOMElement* const childElem,
                                       const bool topLevel, int baseRefContext)
{
    // ------------------------------------------------------------------
    // Process contents
    // ------------------------------------------------------------------
    const XMLCh* name = getElementAttValue(childElem,SchemaSymbols::fgATT_NAME);
    bool nameEmpty = (!name || !*name);

    if (topLevel && nameEmpty) {
        reportSchemaError(childElem, XMLUni::fgXMLErrDomain, XMLErrs::NoNameGlobalElement,
                          SchemaSymbols::fgELT_SIMPLETYPE);
        return 0;
    }

    if (nameEmpty) { // anonymous simpleType
        name = genAnonTypeName(fgAnonSNamePrefix);
    }
    else if (!XMLString::isValidNCName(name)) {

        reportSchemaError(childElem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidDeclarationName,
                          SchemaSymbols::fgELT_SIMPLETYPE, name);
        return 0;
    }

    fBuffer.set(fTargetNSURIString);
    fBuffer.append(chComma);
    fBuffer.append(name);

    unsigned int fullTypeNameId = fStringPool->addOrFind(fBuffer.getRawBuffer());
    const XMLCh* fullName = fStringPool->getValueForId(fullTypeNameId);

    //check if we have already traversed the same simpleType decl
    DatatypeValidator* dv = fDatatypeRegistry->getDatatypeValidator(fullName);

    if (!dv) {

        // ------------------------------------------------------------------
        // Check attributes
        // ------------------------------------------------------------------
        unsigned short scope = (topLevel) ? GeneralAttributeCheck::E_SimpleTypeGlobal
                                          : GeneralAttributeCheck::E_SimpleTypeLocal;

        fAttributeCheck.checkAttributes(childElem, scope, this, topLevel);

        // Circular constraint checking
        if (fCurrentTypeNameStack->containsElement(fullTypeNameId)){

            reportSchemaError(childElem, XMLUni::fgXMLErrDomain, XMLErrs::NoCircularDefinition, name);
            return 0;
        }

        fCurrentTypeNameStack->addElement(fullTypeNameId);

        // Get 'final' values
        int finalSet = parseFinalSet(childElem, S_Final);

        // annotation?,(list|restriction|union)
        DOMElement* content= checkContent(childElem,
                                          XUtil::getFirstChildElement(childElem),
                                          false);

        if (content == 0) {

            reportSchemaError(childElem, XMLUni::fgXMLErrDomain, XMLErrs::EmptySimpleTypeContent);
            popCurrentTypeNameStack();
            return 0;
        }

        const XMLCh* varietyName = content->getLocalName();

        // Remark: some code will be repeated in list|restriction| union but it
        //         is cleaner that way
        if (XMLString::equals(varietyName, SchemaSymbols::fgELT_LIST)) { //traverse List
            if ((baseRefContext & SchemaSymbols::XSD_LIST) != 0) {

                reportSchemaError(content, XMLUni::fgXMLErrDomain, XMLErrs::AtomicItemType);
                popCurrentTypeNameStack();
                return 0;
            }

            DatatypeValidator *tmpDV = traverseByList(childElem, content, name, fullName, finalSet);
            if(tmpDV && nameEmpty)
                tmpDV->setAnonymous();
            return tmpDV;
        }
        else if (XMLString::equals(varietyName, SchemaSymbols::fgELT_RESTRICTION)) { //traverse Restriction
            DatatypeValidator *tmpDV = traverseByRestriction(childElem, content, name, fullName, finalSet);
            if(tmpDV && nameEmpty)
                tmpDV->setAnonymous();
            return tmpDV;
        }
        else if (XMLString::equals(varietyName, SchemaSymbols::fgELT_UNION)) { //traverse union
            DatatypeValidator *tmpDV = traverseByUnion(childElem, content, name, fullName, finalSet, baseRefContext);
            if(tmpDV && nameEmpty)
                tmpDV->setAnonymous();
            return tmpDV;
        }
        else {
            reportSchemaError(content, XMLUni::fgXMLErrDomain, XMLErrs::FeatureUnsupported, varietyName);
            popCurrentTypeNameStack();
        }
    }

    return dv;
}

/**
  * Traverse ComplexType Declaration - CR Implementation.
  *
  *     <complexType
  *        abstract = boolean
  *        block = #all or (possibly empty) subset of {extension, restriction}
  *        final = #all or (possibly empty) subset of {extension, restriction}
  *        id = ID
  *        mixed = boolean : false
  *        name = NCName>
  *        Content: (annotation? , (simpleContent | complexContent |
  *                   ( (group | all | choice | sequence)? ,
  *                   ( (attribute | attributeGroup)* , anyAttribute?))))
  *     </complexType>
  */
int TraverseSchema::traverseComplexTypeDecl(const DOMElement* const elem,
                                            const bool topLevel,
                                            const XMLCh* const recursingTypeName) {

    // Get the attributes of the complexType
    const XMLCh* name = getElementAttValue(elem, SchemaSymbols::fgATT_NAME);
    bool isAnonymous = false;

    if (!name || !*name) {

        if (topLevel) {

            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::TopLevelNoNameComplexType);
            return -1;
        }

        if (recursingTypeName)
            name = recursingTypeName;
        else {
            name = genAnonTypeName(fgAnonCNamePrefix);
            isAnonymous = true;
        }
    }

    if (!XMLString::isValidNCName(name)) {

        //REVISIT - Should we return or continue and save type with wrong name?
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidDeclarationName,
                          SchemaSymbols::fgELT_COMPLEXTYPE, name);
        return -1;
    }

    // ------------------------------------------------------------------
    // Check if the type has already been registered
    // ------------------------------------------------------------------
    fBuffer.set(fTargetNSURIString);
    fBuffer.append(chComma);
    fBuffer.append(name);

    int typeNameIndex = fStringPool->addOrFind(fBuffer.getRawBuffer());
    const XMLCh* fullName = fStringPool->getValueForId(typeNameIndex);
    ComplexTypeInfo* typeInfo = 0;

    if (topLevel || recursingTypeName) {

        typeInfo = fComplexTypeRegistry->get(fullName);

        if (typeInfo && !typeInfo->getPreprocessed()) {
            return typeNameIndex;
        }
    }

    // -----------------------------------------------------------------------
    // Check Attributes
    // -----------------------------------------------------------------------
    unsigned short scope = (topLevel) ? GeneralAttributeCheck::E_ComplexTypeGlobal
                                      : GeneralAttributeCheck::E_ComplexTypeLocal;
    fAttributeCheck.checkAttributes(elem, scope, this, topLevel);

    // -----------------------------------------------------------------------
    // Create a new instance
    // -----------------------------------------------------------------------
    bool preProcessFlag = (typeInfo) ? typeInfo->getPreprocessed() : false;
    unsigned int previousCircularCheckIndex = fCircularCheckIndex;
    int previousScope = fCurrentScope;

    if (preProcessFlag) {

        fCurrentScope = typeInfo->getScopeDefined();
        typeInfo->setPreprocessed(false);
    }
    else {

        // ------------------------------------------------------------------
        // Register the type
        // ------------------------------------------------------------------
        typeInfo = new (fMemoryManager) ComplexTypeInfo(fMemoryManager);
        if(isAnonymous) {
            typeInfo->setAnonymous();
        }

        fCurrentScope = fScopeCount++;
        fComplexTypeRegistry->put((void*) fullName, typeInfo);
        typeInfo->setTypeName(fullName);
        typeInfo->setScopeDefined(fCurrentScope);

        if (fFullConstraintChecking) {

            XSDLocator* aLocator = new XSDLocator();
            aLocator->setValues(fStringPool->getValueForId(fStringPool->addOrFind(fSchemaInfo->getCurrentSchemaURL())),
                                0, ((XSDElementNSImpl*) elem)->getLineNo(),
                                ((XSDElementNSImpl*) elem)->getColumnNo());
            typeInfo->setLocator(aLocator);
        }
    }

    fCurrentTypeNameStack->addElement(typeNameIndex);
    ComplexTypeInfo* saveTypeInfo = fCurrentComplexType;
    fCurrentComplexType = typeInfo;

    // ------------------------------------------------------------------
    // First, handle any ANNOTATION declaration and get next child
    // ------------------------------------------------------------------
    DOMElement* child = checkContent(elem, XUtil::getFirstChildElement(elem), true);

    // ------------------------------------------------------------------
    // Process the content of the complex type declaration
    // ------------------------------------------------------------------
    try {

        const XMLCh* mixedVal = getElementAttValue(elem,SchemaSymbols::fgATT_MIXED);
        bool isMixed = false;

        if ((mixedVal && *mixedVal)
            && (XMLString::equals(SchemaSymbols::fgATTVAL_TRUE, mixedVal)
                || XMLString::equals(fgValueOne, mixedVal))) {
            isMixed = true;
        }

        if (child == 0) {
            // EMPTY complexType with complexContent
            processComplexContent(elem, name, child, typeInfo, 0,0,0, isMixed);
        }
        else {

            const XMLCh* childName = child->getLocalName();

            if (XMLString::equals(childName, SchemaSymbols::fgELT_SIMPLECONTENT)) {

                // SIMPLE CONTENT element
                traverseSimpleContentDecl(name, fullName, child, typeInfo);

                if (XUtil::getNextSiblingElement(child) != 0) {
                    reportSchemaError(child, XMLUni::fgXMLErrDomain, XMLErrs::InvalidChildFollowingSimpleContent);
                }
            }
            else if (XMLString::equals(childName, SchemaSymbols::fgELT_COMPLEXCONTENT)) {

                // COMPLEX CONTENT element
                traverseComplexContentDecl(name, child, typeInfo, isMixed);

                if (XUtil::getNextSiblingElement(child) != 0) {
                    reportSchemaError(child, XMLUni::fgXMLErrDomain, XMLErrs::InvalidChildFollowingConplexContent);
                }
            }
            else if (fCurrentGroupInfo) {
                typeInfo->setPreprocessed(true);
            }
            else {
                // We must have ....
                // GROUP, ALL, SEQUENCE or CHOICE, followed by optional attributes
                // Note that it's possible that only attributes are specified.
                processComplexContent(elem, name, child, typeInfo, 0, 0, 0, isMixed);
            }
        }
    }
    catch(const TraverseSchema::ExceptionCodes aCode) {
        if (aCode == TraverseSchema::InvalidComplexTypeInfo)
            defaultComplexTypeInfo(typeInfo);
        else if (aCode == TraverseSchema::RecursingElement)
            typeInfo->setPreprocessed();
    }

    // ------------------------------------------------------------------
    // Finish the setup of the typeInfo
    // ------------------------------------------------------------------
    if (!preProcessFlag) {

        const XMLCh* abstractAttVal = getElementAttValue(elem, SchemaSymbols::fgATT_ABSTRACT);
        int blockSet = parseBlockSet(elem, C_Block);
        int finalSet = parseFinalSet(elem, EC_Final);

        typeInfo->setBlockSet(blockSet);
        typeInfo->setFinalSet(finalSet);

        if ((abstractAttVal && *abstractAttVal)
            && (XMLString::equals(abstractAttVal, SchemaSymbols::fgATTVAL_TRUE)
                || XMLString::equals(abstractAttVal, fgValueOne))) {
            typeInfo->setAbstract(true);
        }
        else {
            typeInfo->setAbstract(false);
        }
    }

    // ------------------------------------------------------------------
    // Before exiting, restore the scope, mainly for nested anonymous types
    // ------------------------------------------------------------------
    popCurrentTypeNameStack();
    fCircularCheckIndex = previousCircularCheckIndex;
    fCurrentScope = previousScope;
    fCurrentComplexType = saveTypeInfo;

    return typeNameIndex;
}

/**
  * Traverse Group Declaration.
  *
  * <group
  *         id = ID
  *         maxOccurs = string
  *         minOccurs = nonNegativeInteger
  *         name = NCName
  *         ref = QName>
  *   Content: (annotation? , (all | choice | sequence)?)
  * <group/>
  *
  */
XercesGroupInfo*
TraverseSchema::traverseGroupDecl(const DOMElement* const elem,
                                  const bool topLevel) {

    const XMLCh* name = getElementAttValue(elem, SchemaSymbols::fgATT_NAME);
    const XMLCh* ref = getElementAttValue(elem, SchemaSymbols::fgATT_REF);
    bool         nameEmpty = (!name || !*name);
    bool         refEmpty = (!ref || !*ref);

    if (nameEmpty && topLevel) {
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::NoNameGlobalElement,
                          SchemaSymbols::fgELT_GROUP);
        return 0;
    }

    if (nameEmpty && refEmpty) {
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::NoNameRefGroup);
        return 0;
    }

    // ------------------------------------------------------------------
    // Check attributes
    // ------------------------------------------------------------------
    unsigned short scope = (topLevel) ? GeneralAttributeCheck::E_GroupGlobal
                                      : GeneralAttributeCheck::E_GroupRef;
    fAttributeCheck.checkAttributes(elem, scope, this, topLevel);

    // ------------------------------------------------------------------
    // Check for annotations
    // ------------------------------------------------------------------
    DOMElement* content = checkContent(elem, XUtil::getFirstChildElement(elem), true);

    // ------------------------------------------------------------------
    // Handle "ref="
    // ------------------------------------------------------------------
    if (!topLevel) {

        if (refEmpty) {
            return 0;
        }

        return processGroupRef(elem, ref);
    }

    // ------------------------------------------------------------------
    // Process contents of global groups
    // ------------------------------------------------------------------
    // name must be a valid NCName
    if (!XMLString::isValidNCName(name)) {
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidDeclarationName,
                          SchemaSymbols::fgELT_GROUP, name);
        return 0;
    }

    fBuffer.set(fTargetNSURIString);
    fBuffer.append(chComma);
    fBuffer.append(name);

    unsigned int nameIndex = fStringPool->addOrFind(fBuffer.getRawBuffer());
    const XMLCh* fullName = fStringPool->getValueForId(nameIndex);
    XercesGroupInfo* groupInfo = fGroupRegistry->get(fullName);

    if (groupInfo) {
        return groupInfo;
    }

    int saveScope = fCurrentScope;
    ContentSpecNode* specNode = 0;
    XercesGroupInfo* saveGroupInfo = fCurrentGroupInfo;

    groupInfo = new (fMemoryManager) XercesGroupInfo(fMemoryManager);
    fCurrentGroupStack->addElement(nameIndex);
    fCurrentGroupInfo = groupInfo;

    if (fCurrentScope == Grammar::TOP_LEVEL_SCOPE) {
        fCurrentScope = fScopeCount++;
    }

    fCurrentGroupInfo->setScope(fCurrentScope);

    if (content == 0) {
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::GroupContentError, name);
    }
    else {

        if (content->getAttributeNode(SchemaSymbols::fgATT_MINOCCURS) != 0
            || content->getAttributeNode(SchemaSymbols::fgATT_MAXOCCURS) != 0) {
            reportSchemaError(content, XMLUni::fgXMLErrDomain, XMLErrs::MinMaxOnGroupChild);
        }

        bool illegalChild = false;
        const XMLCh* childName = content->getLocalName();

        if (XMLString::equals(childName, SchemaSymbols::fgELT_SEQUENCE)) {
            specNode = traverseChoiceSequence(content, ContentSpecNode::Sequence);
        }
        else if (XMLString::equals(childName, SchemaSymbols::fgELT_CHOICE)) {
            specNode = traverseChoiceSequence(content, ContentSpecNode::Choice);
        }
        else if (XMLString::equals(childName, SchemaSymbols::fgELT_ALL)) {
            specNode = traverseAll(content);
        }
        else {
            illegalChild = true;
        }

        if (illegalChild || XUtil::getNextSiblingElement(content) != 0) {
            reportSchemaError(content, XMLUni::fgXMLErrDomain, XMLErrs::GroupContentError, name);
        }
    }

    // ------------------------------------------------------------------
    // Set groupInfo and pop group name from stack
    // ------------------------------------------------------------------
    unsigned int stackSize = fCurrentGroupStack->size();

    if (stackSize != 0) {
        fCurrentGroupStack->removeElementAt(stackSize - 1);
    }

    fCurrentGroupInfo->setContentSpec(specNode);
    fGroupRegistry->put((void*) fullName, fCurrentGroupInfo);
    fCurrentGroupInfo = saveGroupInfo;
    fCurrentScope = saveScope;

    if (fFullConstraintChecking) {

        XSDLocator* aLocator = new XSDLocator();

        groupInfo->setLocator(aLocator);
        aLocator->setValues(fStringPool->getValueForId(fStringPool->addOrFind(fSchemaInfo->getCurrentSchemaURL())),
                            0, ((XSDElementNSImpl*) elem)->getLineNo(),
                            ((XSDElementNSImpl*) elem)->getColumnNo());
		
        if (fRedefineComponents && fRedefineComponents->get(SchemaSymbols::fgELT_GROUP, nameIndex)) {

            fBuffer.set(fullName);
            fBuffer.append(SchemaSymbols::fgRedefIdentifier);
            groupInfo->setBaseGroup(fGroupRegistry->get(fBuffer.getRawBuffer()));
        }
    }

    return groupInfo;
}


/**
  * Traverse attributeGroup Declaration.
  *
  * <attributeGroup
  *         id = ID
  *         name = NCName
  *         ref = QName>
  *   Content: (annotation? , (attribute|attributeGroup)*, anyAttribute?)
  * <attributeGroup/>
  *
  */
XercesAttGroupInfo*
TraverseSchema::traverseAttributeGroupDecl(const DOMElement* const elem,
                                           ComplexTypeInfo* const typeInfo,
                                           const bool topLevel) {

    const XMLCh* name = getElementAttValue(elem, SchemaSymbols::fgATT_NAME);
    const XMLCh* ref = getElementAttValue(elem, SchemaSymbols::fgATT_REF);
    bool         nameEmpty = (!name || !*name) ? true : false;
    bool         refEmpty = (!ref || !*ref) ? true : false;

    if (nameEmpty && topLevel) {
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::NoNameGlobalElement,
			SchemaSymbols::fgELT_ATTRIBUTEGROUP);
        return 0;
    }

    if (nameEmpty && refEmpty) {
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::NoNameRefAttGroup);
        return 0;
    }

    // ------------------------------------------------------------------
    // Check attributes
    // ------------------------------------------------------------------
    unsigned short scope = (topLevel) ? GeneralAttributeCheck::E_AttributeGroupGlobal
                                      : GeneralAttributeCheck::E_AttributeGroupRef;
    fAttributeCheck.checkAttributes(elem, scope, this, topLevel);

    // ------------------------------------------------------------------
    // Handle "ref="
    // ------------------------------------------------------------------
    if (!topLevel) {

        if (refEmpty) {
            return 0;
        }

        return processAttributeGroupRef(elem, ref, typeInfo);
    }

    // ------------------------------------------------------------------
    // Handle "name="
    // ------------------------------------------------------------------
    // name must be a valid NCName
    if (!XMLString::isValidNCName(name)) {
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidDeclarationName,
                          SchemaSymbols::fgELT_ATTRIBUTEGROUP, name);
        return 0;
    }

    // ------------------------------------------------------------------
    // Check for annotations
    // ------------------------------------------------------------------
    DOMElement* content = checkContent(elem, XUtil::getFirstChildElement(elem), true);

    // ------------------------------------------------------------------
    // Process contents of global attributeGroups
    // ------------------------------------------------------------------
    XercesAttGroupInfo* saveAttGroupInfo = fCurrentAttGroupInfo;
    XercesAttGroupInfo* attGroupInfo = new (fMemoryManager) XercesAttGroupInfo(fMemoryManager);

    fDeclStack->addElement(elem);
    fCurrentAttGroupInfo = attGroupInfo;

    for (; content !=0; content = XUtil::getNextSiblingElement(content)) {

        if (XMLString::equals(content->getLocalName(), SchemaSymbols::fgELT_ATTRIBUTE)) {
            traverseAttributeDecl(content, typeInfo);
        }
        else if (XMLString::equals(content->getLocalName(), SchemaSymbols::fgELT_ATTRIBUTEGROUP)) {
            traverseAttributeGroupDecl(content, typeInfo);
        }
        else {
            break;
        }
    }

    if (content != 0) {

        if (XMLString::equals(content->getLocalName(), SchemaSymbols::fgELT_ANYATTRIBUTE)) {

            SchemaAttDef* anyAtt = traverseAnyAttribute(content);

            if (anyAtt) {
                fCurrentAttGroupInfo->addAnyAttDef(anyAtt);
            }

            if (XUtil::getNextSiblingElement(content) != 0) {
                reportSchemaError(content, XMLUni::fgXMLErrDomain, XMLErrs::AttGroupContentError, name);
            }
        }
        else {
            reportSchemaError(content, XMLUni::fgXMLErrDomain, XMLErrs::AttGroupContentError, name);
        }
    }

    // ------------------------------------------------------------------
    // Pop declaration
    // ------------------------------------------------------------------
    fDeclStack->removeElementAt(fDeclStack->size() - 1);

    // ------------------------------------------------------------------
    // Restore old attGroupInfo
    // ------------------------------------------------------------------
    fAttGroupRegistry->put((void*) fStringPool->getValueForId(fStringPool->addOrFind(name)), attGroupInfo);
    fCurrentAttGroupInfo = saveAttGroupInfo;

    // ------------------------------------------------------------------
    // Check Attribute Derivation Restriction OK
    // ------------------------------------------------------------------
    fBuffer.set(fTargetNSURIString);
    fBuffer.append(chComma);
    fBuffer.append(name);

    unsigned int nameIndex = fStringPool->addOrFind(fBuffer.getRawBuffer());

    if (fRedefineComponents && fRedefineComponents->get(SchemaSymbols::fgELT_ATTRIBUTEGROUP, nameIndex)) {

        fBuffer.set(name);
        fBuffer.append(SchemaSymbols::fgRedefIdentifier);
        XercesAttGroupInfo* baseAttGroupInfo = fAttGroupRegistry->get(fBuffer.getRawBuffer());

        if (baseAttGroupInfo) {
            checkAttDerivationOK(elem, baseAttGroupInfo, attGroupInfo);
        }
    }

    return attGroupInfo;
}


inline XercesAttGroupInfo*
TraverseSchema::traverseAttributeGroupDeclNS(const DOMElement* const elem,
                                             const XMLCh* const uriStr,
                                             const XMLCh* const name) {

    // ------------------------------------------------------------------
    // Get grammar information
    // ------------------------------------------------------------------
    Grammar* aGrammar = fGrammarResolver->getGrammar(uriStr);

    if (!aGrammar || aGrammar->getGrammarType() != Grammar::SchemaGrammarType) {

        reportSchemaError(elem, XMLUni::fgValidityDomain, XMLValid::GrammarNotFound, uriStr);
        return 0;
    }

    XercesAttGroupInfo* attGroupInfo = ((SchemaGrammar*)aGrammar)->getAttGroupInfoRegistry()->get(name);

    return attGroupInfo;
}

/**
  * Traverse Any declaration
  *
  *     <any
  *        id = ID
  *        maxOccurs = (nonNegativeInteger | unbounded)  : 1
  *        minOccurs = nonNegativeInteger : 1
  *        namespace = ((##any | ##other) | List of (anyURI |
  *                     (##targetNamespace | ##local)) )  : ##any
  *        processContents = (lax | skip | strict) : strict
  *        {any attributes with non-schema namespace . . .}>
  *        Content: (annotation?)
  *     </any>
  */
ContentSpecNode*
TraverseSchema::traverseAny(const DOMElement* const elem) {

    // -----------------------------------------------------------------------
    // Check Attributes
    // -----------------------------------------------------------------------
    fAttributeCheck.checkAttributes(elem, GeneralAttributeCheck::E_Any, this);

    // ------------------------------------------------------------------
    // First, handle any ANNOTATION declaration
    // ------------------------------------------------------------------
    if (checkContent(elem, XUtil::getFirstChildElement(elem), true) != 0) {
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::OnlyAnnotationExpected);
    }

    // ------------------------------------------------------------------
    // Get attributes
    // ------------------------------------------------------------------
    const XMLCh* const processContents = getElementAttValue(elem, SchemaSymbols::fgATT_PROCESSCONTENTS);
    const XMLCh* const nameSpace = getElementAttValue(elem, SchemaSymbols::fgATT_NAMESPACE);

    // ------------------------------------------------------------------
    // Set default node type based on 'processContents' value
    // ------------------------------------------------------------------
    ContentSpecNode::NodeTypes anyType = ContentSpecNode::Any;
    ContentSpecNode::NodeTypes anyLocalType = ContentSpecNode::Any_NS;
    ContentSpecNode::NodeTypes anyOtherType = ContentSpecNode::Any_Other;

    if ((processContents && *processContents)
        && !XMLString::equals(processContents, fgStrict)) {

        if (XMLString::equals(processContents, fgLax)) {

            anyType = ContentSpecNode::Any_Lax;
            anyOtherType = ContentSpecNode::Any_Other_Lax;
            anyLocalType = ContentSpecNode::Any_NS_Lax;
        }
        else if (XMLString::equals(processContents, fgSkip)) {

            anyType = ContentSpecNode::Any_Skip;
            anyOtherType = ContentSpecNode::Any_Other_Skip;
            anyLocalType = ContentSpecNode::Any_NS_Skip;
        }
    }

    // ------------------------------------------------------------------
    // Process 'namespace' attribute
    // ------------------------------------------------------------------
    ContentSpecNode* retSpecNode = 0;

    if ((!nameSpace || !*nameSpace)
        || XMLString::equals(nameSpace, SchemaSymbols::fgATTVAL_TWOPOUNDANY)) {
        retSpecNode = new (fMemoryManager) ContentSpecNode(new (fMemoryManager) QName(XMLUni::fgZeroLenString,
                                                    XMLUni::fgZeroLenString,
                                                    fEmptyNamespaceURI),
                                          false);
        retSpecNode->setType(anyType);
    }
    else if (XMLString::equals(nameSpace, SchemaSymbols::fgATTVAL_TWOPOUNDOTHER)) {
        retSpecNode = new (fMemoryManager) ContentSpecNode(new (fMemoryManager) QName(XMLUni::fgZeroLenString,
                                                    XMLUni::fgZeroLenString,
                                                    fTargetNSURI),
                                          false);
        retSpecNode->setType(anyOtherType);
    }
    else {

        BaseRefVectorOf<XMLCh>* nameSpaceTokens = XMLString::tokenizeString(nameSpace);
        ValueVectorOf<unsigned int> uriList(8);
        ContentSpecNode* firstNode = 0;
        ContentSpecNode* secondNode = 0;
        unsigned int tokensSize = nameSpaceTokens->size();
        DatatypeValidator* anyURIDV = fDatatypeRegistry->getDatatypeValidator(SchemaSymbols::fgDT_ANYURI);

        for (unsigned int i=0; i < tokensSize; i++) {

            const XMLCh* tokenElem = nameSpaceTokens->elementAt(i);
            int uriIndex = fEmptyNamespaceURI;

            if (!XMLString::equals(tokenElem,SchemaSymbols::fgATTVAL_TWOPOUNDLOCAL)) { // not ##local

                if (XMLString::equals(tokenElem,SchemaSymbols::fgATTVAL_TWOPOUNDTRAGETNAMESPACE)) {
                    uriIndex = fTargetNSURI;
                }
                else {
                    try {
                        anyURIDV->validate(tokenElem);
                    }
                    catch(const XMLException& excep) {
                        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::DisplayErrorMessage, excep.getMessage());
                    }
                    uriIndex = fURIStringPool->addOrFind(tokenElem);
                }
            }

            if (uriList.containsElement(uriIndex)) {
                continue;
            }

            uriList.addElement(uriIndex);

            firstNode = new (fMemoryManager) ContentSpecNode(new (fMemoryManager) QName(XMLUni::fgZeroLenString,
                                                      XMLUni::fgZeroLenString,
                                                      uriIndex),
                                            false);
            firstNode->setType(anyLocalType);

            if (secondNode == 0) {
                secondNode = firstNode;
            }
            else {
                secondNode = new (fMemoryManager) ContentSpecNode(ContentSpecNode::Choice, secondNode, firstNode);
            }
        }

        retSpecNode = secondNode;
        delete nameSpaceTokens;
    }

    return retSpecNode;
}


/**
  *  Traverse all
  *
  *     <all
  *        id = ID
  *        maxOccurs = 1 : 1
  *        minOccurs = (0 | 1) : 1
  *        {any attributes with non-schema namespace . . .}>
  *        Content: (annotation?, element*)
  *     </all>
  */
ContentSpecNode*
TraverseSchema::traverseAll(const DOMElement* const elem) {

    // ------------------------------------------------------------------
    // Check attributes
    // ------------------------------------------------------------------
    fAttributeCheck.checkAttributes(elem, GeneralAttributeCheck::E_All, this);

    // ------------------------------------------------------------------
    // Process contents
    // ------------------------------------------------------------------
    DOMElement* child = checkContent(elem, XUtil::getFirstChildElement(elem), true);

    if (child == 0) {
        return 0;
    }

    ContentSpecNode* left = 0;
    ContentSpecNode* right = 0;
    bool hadContent = false;

    for (; child != 0; child = XUtil::getNextSiblingElement(child)) {

        ContentSpecNode* contentSpecNode = 0;
        const XMLCh* childName = child->getLocalName();

        if (XMLString::equals(childName, SchemaSymbols::fgELT_ELEMENT)) {

            QName* eltQName = traverseElementDecl(child);

            if (eltQName == 0) {
                continue;
            }

            contentSpecNode = new (fMemoryManager) ContentSpecNode(eltQName, false);
            checkMinMax(contentSpecNode, child, All_Element);
        }
        else {

            reportSchemaError(child, XMLUni::fgXMLErrDomain, XMLErrs::AllContentError, childName);
            continue;
        }

        hadContent = true;

        if (!left) {
            left = contentSpecNode;
        }
        else if (!right) {
            right = contentSpecNode;
        }
        else {
            left = new (fMemoryManager) ContentSpecNode(ContentSpecNode::All, left, right);
            right = contentSpecNode;
        }
    }

    if (hadContent) {
        left = new (fMemoryManager) ContentSpecNode(ContentSpecNode::All, left, right);
    }

    return left;
}

/**
  * Traverses Schema attribute declaration.
  *
  *       <attribute
  *         fixed = string
  *         default = string
  *         form = qualified | unqualified
  *         id = ID
  *         name = NCName
  *         ref = QName
  *         type = QName
  *         use = optional | prohibited | required : optional
  >
  *         Content: (annotation? , simpleType?)
  *       <attribute/>
  *
  * @param elem:        the declaration of the attribute under consideration
  *
  * @param typeInfo:    Contains the complex type info of the element to which
  *                     the attribute declaration is attached.
  *
  */
void TraverseSchema::traverseAttributeDecl(const DOMElement* const elem,
                                           ComplexTypeInfo* const typeInfo,
                                           const bool topLevel) {

    const XMLCh*   name = getElementAttValue(elem, SchemaSymbols::fgATT_NAME);
    const XMLCh*   ref = getElementAttValue(elem, SchemaSymbols::fgATT_REF);
    bool           nameEmpty = (!name || !*name);
    bool           refEmpty = (!ref || !*ref);

    if (nameEmpty && refEmpty) {
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::NoNameRefAttribute);
        return;
    }

    if (topLevel && nameEmpty) {

        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::TopLevelNoNameAttribute);
        return;
    }

    // ------------------------------------------------------------------
    // Check attributes
    // ------------------------------------------------------------------
    unsigned short scope = (topLevel) ? GeneralAttributeCheck::E_AttributeGlobal
                                      : (refEmpty) ? GeneralAttributeCheck::E_AttributeLocal
                                                   : GeneralAttributeCheck::E_AttributeRef;
    fAttributeCheck.checkAttributes(elem, scope, this, topLevel);

    const XMLCh* defaultVal = getElementAttValue(elem, SchemaSymbols::fgATT_DEFAULT);
    const XMLCh* fixedVal = getElementAttValue(elem, SchemaSymbols::fgATT_FIXED);
    const XMLCh* useVal = getElementAttValue(elem, SchemaSymbols::fgATT_USE);
    const XMLCh* attForm = getElementAttValue(elem, SchemaSymbols::fgATT_FORM);
    const XMLCh* dvType = getElementAttValue(elem, SchemaSymbols::fgATT_TYPE);
    DOMElement* simpleType = checkContent(elem, XUtil::getFirstChildElement(elem), true);
    bool         badContent = false;

    while (simpleType != 0) {

        const XMLCh* contentName = simpleType->getLocalName();

        if (XMLString::equals(SchemaSymbols::fgELT_SIMPLETYPE, contentName)) {

            if (XUtil::getNextSiblingElement(simpleType) != 0) {
                badContent = true;
            }
            break;
        }

        badContent = true;
        simpleType = XUtil::getNextSiblingElement(simpleType);
    }

    if (badContent) {
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidAttributeContent,
                          (name) ? SchemaSymbols::fgATT_NAME : SchemaSymbols::fgATT_REF,
                          (name) ? name : ref);
    }

    if (defaultVal) {

        if (fixedVal) {

            fixedVal = 0;
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::AttributeDefaultFixedValue);
        }

        if ((useVal && *useVal)
            && !XMLString::equals(useVal, SchemaSymbols::fgATTVAL_OPTIONAL)) {

            useVal = 0;
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::NotOptionalDefaultAttValue);
        }
    }

    // processing ref
    if (!refEmpty && !topLevel) {

        // Check ref representation OK - 3.2.3::3.2
        if (attForm || dvType || (simpleType != 0)) {
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::AttributeRefContentError);
        }

        processAttributeDeclRef(elem, typeInfo, ref, useVal, defaultVal, fixedVal);
        return;
    }

    // processing 'name'
    if (!XMLString::isValidNCName(name)
        || XMLString::equals(name, XMLUni::fgXMLNSString)) {

        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidDeclarationName, SchemaSymbols::fgELT_ATTRIBUTE, name);
        return;
    }

    // Check for duplicate declaration
    const XMLCh* qualified = SchemaSymbols::fgATTVAL_QUALIFIED;
    int uriIndex = fEmptyNamespaceURI;

    if ((fTargetNSURIString && *fTargetNSURIString)
        && (topLevel || XMLString::equals(attForm, qualified)
            || ((fSchemaInfo->getElemAttrDefaultQualified() & Attr_Def_Qualified)
                && (!attForm || !*attForm)))) {
        uriIndex = fTargetNSURI;
    }

    // make sure that attribute namespace is not xsi uri
    if (XMLString::equals(fTargetNSURIString, SchemaSymbols::fgURI_XSI)) {

        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidAttTNS, name);
        return;
    }

    if (typeInfo && typeInfo->getAttDef(name, uriIndex) != 0) {

        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::DuplicateAttribute, name);
        return;
    }
    else if (fCurrentAttGroupInfo && fCurrentAttGroupInfo->containsAttribute(name, uriIndex)) {

        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::DuplicateAttribute, name);
        return;
    }

    DatatypeValidator*  dv = 0;
    XMLAttDef::AttTypes attType = XMLAttDef::Simple;
    SchemaInfo* saveInfo = fSchemaInfo;

    if (simpleType != 0) {

        if (dvType && *dvType) {
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::AttributeWithTypeAndSimpleType);
        }

        dv = traverseSimpleTypeDecl(simpleType, false);
    }
    else if (!dvType || !*dvType) {
        dv = fDatatypeRegistry->getDatatypeValidator(SchemaSymbols::fgDT_ANYSIMPLETYPE);
    }
    else {

        checkEnumerationRequiredNotation(elem, name, dvType);

        const XMLCh* localPart = getLocalPart(dvType);
        const XMLCh* prefix = getPrefix(dvType);
        const XMLCh* typeURI = resolvePrefixToURI(elem, prefix);
        DatatypeValidator*  dvBack = 0;

        if (XMLString::equals(typeURI, SchemaSymbols::fgURI_SCHEMAFORSCHEMA)) {
            dv = fDatatypeRegistry->getDatatypeValidator(localPart);
            dvBack = dv;
        }
        else { //isn't of the schema for schemas namespace...

            dv = getAttrDatatypeValidatorNS(elem, localPart, typeURI);
            dvBack = dv;

            while(dv != 0 && !XMLString::equals(dv->getTypeUri(), SchemaSymbols::fgURI_SCHEMAFORSCHEMA)) {
                dv = dv->getBaseValidator();
            }

            if(dv)
                localPart = dv->getTypeLocalName();
        }

        if(dv) {
            if (XMLString::equals(localPart,XMLUni::fgIDString)) {
                attType = XMLAttDef::ID;
            }
            else if (XMLString::equals(localPart,XMLUni::fgIDRefString)) {
                attType = XMLAttDef::IDRef;
            }
            else if (XMLString::equals(localPart,XMLUni::fgIDRefsString)) {
                attType = XMLAttDef::IDRefs;
            }
            else if (XMLString::equals(localPart,XMLUni::fgEntityString)) {
                attType = XMLAttDef::Entity;
            }
            else if (XMLString::equals(localPart,XMLUni::fgEntitiesString)) {
                attType = XMLAttDef::Entities;
            }
            else if (XMLString::equals(localPart,XMLUni::fgNmTokenString)) {
                attType = XMLAttDef::NmToken;
            }
            else if (XMLString::equals(localPart,XMLUni::fgNmTokensString)) {
                attType = XMLAttDef::NmTokens;
            }
            else if (XMLString::equals(localPart,XMLUni::fgNotationString)) {
                attType = XMLAttDef::Notation;
            }
            else {
                attType = XMLAttDef::Simple;
            }
        }
        else 
            attType = XMLAttDef::Simple;

        dv = dvBack;

        if (!dv) {
            reportSchemaError
            (
                elem
                , XMLUni::fgXMLErrDomain
                , XMLErrs::AttributeSimpleTypeNotFound
                , typeURI
                , localPart
                , name
            );
        }
    }

    // restore schema information, if necessary
    fSchemaInfo = saveInfo;

    bool required = false;
    bool prohibited = false;

    if (useVal && *useVal) {

        if (XMLString::equals(useVal, SchemaSymbols::fgATTVAL_REQUIRED)) {
            required = true;
        }
        else if (XMLString::equals(useVal, SchemaSymbols::fgATTVAL_PROHIBITED)) {
            prohibited = true;
        }
    }

    // validate fixed/default values
    const XMLCh* valueToCheck = defaultVal ? defaultVal : fixedVal;
    bool  ofTypeID = (dv && dv->getType() == DatatypeValidator::ID);

    if (attType == XMLAttDef::Simple && dv && valueToCheck) {

        try {
            dv->validate(valueToCheck);
        }
        catch (const XMLException& excep) {
            reportSchemaError(elem, XMLUni::fgValidityDomain, XMLValid::DisplayErrorMessage, excep.getMessage());
        }
        catch(...) {
            reportSchemaError(elem, XMLUni::fgValidityDomain, XMLValid::DatatypeValidationFailure, valueToCheck);
        }
    }

    if (ofTypeID && valueToCheck) {
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::AttDeclPropCorrect3,
                         SchemaSymbols::fgATT_NAME, name);
    }

    // check for multiple attributes with type derived from ID
    if (!topLevel && ofTypeID) {

        if (fCurrentAttGroupInfo) {

            if (fCurrentAttGroupInfo->containsTypeWithId()) {

                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::AttGrpPropCorrect3, name);
                return;
            }

            fCurrentAttGroupInfo->setTypeWithId(true);
        }
        else {

            if (typeInfo->containsAttWithTypeId()) {

                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::AttDeclPropCorrect5, name);
                return;
            }

            typeInfo->setAttWithTypeId(true);
        }
    }

    // create SchemaAttDef
    SchemaAttDef* attDef = new (fMemoryManager) SchemaAttDef(XMLUni::fgZeroLenString, name, uriIndex, attType);

    attDef->setDatatypeValidator(dv);

    if (prohibited) {
        attDef->setDefaultType(XMLAttDef::Prohibited);
    }
    else if (required) {

        if (fixedVal) {
            attDef->setDefaultType(XMLAttDef::Required_And_Fixed);
        }
        else {
            attDef->setDefaultType(XMLAttDef::Required);
        }
    }
    else {

        if (fixedVal) {
            attDef->setDefaultType(XMLAttDef::Fixed);
        }
        else if (defaultVal) {
            attDef->setDefaultType(XMLAttDef::Default);
        }
    }

    if (valueToCheck) {
        attDef->setValue(valueToCheck);
    }

    if (topLevel) {
        fAttributeDeclRegistry->put((void*) fStringPool->getValueForId(fStringPool->addOrFind(name)), attDef);
    }
    else {
	
        bool toClone = false;

        if (typeInfo) {

            toClone = true;
            typeInfo->addAttDef(attDef);
        }

        if (fCurrentAttGroupInfo) {
            fCurrentAttGroupInfo->addAttDef(attDef, toClone);
        }
    }
}


/**
  * Traverses Schema element declaration.
  *
  *       <element
  *            abstract = boolean : false
  *            block = (#all | List of (substitution | extension | restriction
  *                                     | list | union))
  *            default = string
  *            final = (#all | List of (extension | restriction))
  *            fixed = string
  *            form = (qualified | unqualified)
  *            id = ID
  *            maxOccurs = (nonNegativeInteger | unbounded)  : 1
  *            minOccurs = nonNegativeInteger : 1
  *            name = NCName
  *            nillable = boolean : false
  *            ref = QName
  *            substitutionGroup = QName
  *            type = QName
  *            Content: (annotation?, ((simpleType | complexType)?, (unique | key | keyref)*))
  *       </element>
  *
  * @param elem:  the declaration of the element under consideration
  */
QName* TraverseSchema::traverseElementDecl(const DOMElement* const elem,
                                           const bool topLevel) {

    const XMLCh* name = getElementAttValue(elem, SchemaSymbols::fgATT_NAME);
    const XMLCh* ref = getElementAttValue(elem, SchemaSymbols::fgATT_REF);
    bool         nameEmpty = (!name || !*name) ? true : false;
    bool         refEmpty = (!ref || !*ref) ? true : false;

    if (nameEmpty && topLevel) {
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::GlobalNoNameElement);
        return 0;
    }

    if (nameEmpty && refEmpty) {
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::NoNameRefElement);
        return 0;
    }

    if (topLevel) {

        if (fSchemaGrammar->getElemDecl(fTargetNSURI, name, 0, Grammar::TOP_LEVEL_SCOPE) != 0) {
            return new (fMemoryManager) QName(name, fTargetNSURI);
        }
    }

    // ------------------------------------------------------------------
    // Check attributes
    // ------------------------------------------------------------------
    unsigned short scope = (topLevel) ? GeneralAttributeCheck::E_ElementGlobal
		                              : (refEmpty) ? GeneralAttributeCheck::E_ElementLocal
                                                   : GeneralAttributeCheck::E_ElementRef;

    fAttributeCheck.checkAttributes(elem, scope, this, topLevel);

    // ------------------------------------------------------------------
    // Process contents
    // ------------------------------------------------------------------
    const XMLCh* fixed = getElementAttValue(elem, SchemaSymbols::fgATT_FIXED);
    const XMLCh* deflt = getElementAttValue(elem, SchemaSymbols::fgATT_DEFAULT);

    if((fixed && *fixed) && (deflt && *deflt)) {
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::ElementWithFixedAndDefault);
    }

    if (nameEmpty || (!refEmpty && !topLevel)) {

        if (!nameEmpty) {
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::DeclarationWithNameRef,
                              SchemaSymbols::fgELT_ELEMENT, name);
        }

        if (!isValidRefDeclaration(elem)) {
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::BadAttWithRef);
        }

        return processElementDeclRef(elem, ref);
    }

    // Name is notEmpty
    if (!XMLString::isValidNCName(name)) {
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidDeclarationName,
                          SchemaSymbols::fgELT_ELEMENT, name);
        return 0;
    }

    const XMLCh*                  anotherSchemaURI = 0;
    int                           scopeDefined = Grammar::UNKNOWN_SCOPE;
    bool                          noErrorFound = true;
    bool                          anonymousType = false;
    DatatypeValidator*            validator = 0;
    DatatypeValidator*            subGroupValidator = 0;
    ComplexTypeInfo*              typeInfo = 0;
    ComplexTypeInfo*              subGroupTypeInfo = 0;
    ContentSpecNode*              contentSpecNode = 0;
    SchemaElementDecl::ModelTypes contentSpecType = SchemaElementDecl::Any;

    // Create element decl
    bool isDuplicate = false;
    SchemaElementDecl* elemDecl =
       createSchemaElementDecl(elem, topLevel, contentSpecType, isDuplicate, (fixed != 0));

    if (elemDecl == 0) {
        return 0;
    }

    if (!isDuplicate) {

        fSchemaGrammar->putElemDecl(elemDecl);

        if (fCurrentGroupInfo &&
            elemDecl->getEnclosingScope() == fCurrentGroupInfo->getScope()) {
            fCurrentGroupInfo->addElement(elemDecl);
        }

        if (fCurrentComplexType &&
            elemDecl->getEnclosingScope() == fCurrentComplexType->getScopeDefined()) {
            fCurrentComplexType->addElement(elemDecl);
        }
    }

    // Resolve the type for the element
    const DOMElement*  content = checkContent(elem, XUtil::getFirstChildElement(elem), true);

    if (content != 0) {

        const XMLCh* contentName = content->getLocalName();

        if (XMLString::equals(contentName, SchemaSymbols::fgELT_COMPLEXTYPE)) {
            const XMLCh* temp = content->getAttribute(SchemaSymbols::fgATT_NAME);

            if (temp && *temp) {
                // REVISIT - we are bypassing the complex type declaration.
                reportSchemaError(content, XMLUni::fgXMLErrDomain, XMLErrs::AnonComplexTypeWithName, name);
            }
            else {
                typeInfo = checkForComplexTypeInfo(content);
            }

            if (!typeInfo) {
                noErrorFound = false;
            }
            else if (!isDuplicate) {

                typeInfo->setElementId(elemDecl->getId());

                //Recursing element
                if (typeInfo->getPreprocessed()) {

                    const XMLCh* typeInfoName = typeInfo->getTypeName();
                    fSchemaInfo->addRecursingType(content, typeInfoName + XMLString::indexOf(typeInfoName, chComma) + 1);
                }
            }

            anonymousType = true;
            content = XUtil::getNextSiblingElement(content);
        }
        else if (XMLString::equals(contentName, SchemaSymbols::fgELT_SIMPLETYPE)) {

            const XMLCh* temp = content->getAttribute(SchemaSymbols::fgATT_NAME);
            if (temp && *temp) {
                // REVISIT - we are bypassing the simple type declaration.
                reportSchemaError(content, XMLUni::fgXMLErrDomain, XMLErrs::AnonSimpleTypeWithName, name);
            }
            else {
                validator = checkForSimpleTypeValidator(content);
            }

            if (!validator) {
                noErrorFound = false;
            }

            contentSpecType = SchemaElementDecl::Simple;
            anonymousType = true;
            content = XUtil::getNextSiblingElement(content);
        }

        // Check for identity constraints
        if (content != 0) {

            content = checkIdentityConstraintContent(content);
            if (content != 0) {
                reportSchemaError(content, XMLUni::fgXMLErrDomain, XMLErrs::InvalidElementContent);
            }
        }
    }

    // Handle 'type' attribute
    const XMLCh* typeStr = getElementAttValue(elem, SchemaSymbols::fgATT_TYPE);
    if (typeStr && *typeStr) {

        if (anonymousType) {

            noErrorFound = false;
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::ElementWithTypeAndAnonType, name);
        }
        else {

            const XMLCh* typeLocalPart = getLocalPart(typeStr);
            const XMLCh* typePrefix = getPrefix(typeStr);
            const XMLCh* typeURI = resolvePrefixToURI(elem, typePrefix);

            if (!XMLString::equals(typeURI, SchemaSymbols::fgURI_SCHEMAFORSCHEMA)
                || !XMLString::equals(typeLocalPart, SchemaSymbols::fgATTVAL_ANYTYPE)) {

                checkEnumerationRequiredNotation(elem, name, typeStr);

                anotherSchemaURI = checkTypeFromAnotherSchema(elem, typeStr);

                // get complex type info
                typeInfo = getElementComplexTypeInfo(elem, typeStr, noErrorFound, anotherSchemaURI);

                // get simple type validtor - if not a complex type
                if (!typeInfo) {
                    validator = getElementTypeValidator(elem, typeStr, noErrorFound, anotherSchemaURI);
                }
            }
        }
    }

    // Set element declararion type information
    if (!isDuplicate) {
        elemDecl->setDatatypeValidator(validator);
        elemDecl->setComplexTypeInfo(typeInfo);
    }

    if (topLevel) {

        // Handle the substitutionGroup
        const XMLCh* subsGroupName = getElementAttValue(elem, SchemaSymbols::fgATT_SUBSTITUTIONGROUP);

        if (subsGroupName && *subsGroupName) {

            SchemaElementDecl* subsElemDecl = getSubstituteGroupElemDecl
            (
                elem
                , subsGroupName
                , noErrorFound
            );

            if (subsElemDecl) {

                if (isSubstitutionGroupCircular(elemDecl, subsElemDecl)) {
                    reportSchemaError
                    (
                        elem
                        , XMLUni::fgXMLErrDomain
                        , XMLErrs::CircularSubsGroup
                        , name
                    );
                }
                else {

                    // Check for substitution validity constraint
                    // Substitution allowed (block and blockDefault) && same type
                    if (isSubstitutionGroupValid(elem, subsElemDecl,typeInfo,validator,name)) {

                        if (typeInfo == 0 && validator == 0 && noErrorFound) {

                            typeInfo = subsElemDecl->getComplexTypeInfo();
                            validator = subsElemDecl->getDatatypeValidator();
                        }

                        if (!isDuplicate) {

                            XMLCh* elemBaseName = elemDecl->getBaseName();
                            XMLCh* subsElemBaseName = subsElemDecl->getBaseName();
                            int    elemURI = elemDecl->getURI();
                            int    subsElemURI = subsElemDecl->getURI();

                            elemDecl->setSubstitutionGroupElem(subsElemDecl);
                            ValueVectorOf<SchemaElementDecl*>* subsElements =
                                fValidSubstitutionGroups->get(subsElemBaseName, subsElemURI);

                            if (!subsElements && fTargetNSURI != subsElemURI) {

                                SchemaGrammar* aGrammar = (SchemaGrammar*)
                                   fGrammarResolver->getGrammar(fURIStringPool->getValueForId(subsElemURI));

                                if (aGrammar) {
                                    subsElements = aGrammar->getValidSubstitutionGroups()->get(subsElemBaseName, subsElemURI);

                                    if (subsElements) {
                                        subsElements = new (fMemoryManager) ValueVectorOf<SchemaElementDecl*>(*subsElements);
                                        fValidSubstitutionGroups->put(subsElemBaseName, subsElemURI, subsElements);
                                    }
                                    else if (fSchemaInfo->circularImportExist(subsElemURI)) {

                                        aGrammar->getValidSubstitutionGroups()->put(
                                        subsElemBaseName, subsElemURI, new (fMemoryManager) ValueVectorOf<SchemaElementDecl*>(8));
                                    }
                                }
                            }

                            if (!subsElements) {

                                subsElements = new (fMemoryManager) ValueVectorOf<SchemaElementDecl*>(8);
                                fValidSubstitutionGroups->put(subsElemBaseName, subsElemURI, subsElements);
                            }

                            subsElements->addElement(elemDecl);

                            // update related subs. info in case of circular import
                            BaseRefVectorEnumerator<SchemaInfo> importingEnum = fSchemaInfo->getImportingListEnumerator();

                            while (importingEnum.hasMoreElements()) {

                                const SchemaInfo& curRef = importingEnum.nextElement();
                                SchemaGrammar* aGrammar = (SchemaGrammar*)
                                    fGrammarResolver->getGrammar(curRef.getTargetNSURIString());
                                ValueVectorOf<SchemaElementDecl*>* subsElemList =
                                    aGrammar->getValidSubstitutionGroups()->get(subsElemBaseName, subsElemURI);

                                if (subsElemList && !subsElemList->containsElement(elemDecl))
                                    subsElemList->addElement(elemDecl);
                            }

                            buildValidSubstitutionListB(elem, elemDecl, subsElemDecl);
                            buildValidSubstitutionListF(elem, elemDecl, subsElemDecl);
                        }
                    }
                    else {
                        noErrorFound = false;
                    }
                }
            }
        }
    }

    bool isAnyType = false;

    if (typeInfo == 0 && validator == 0) {

        if (noErrorFound) { // ur type

            contentSpecType = SchemaElementDecl::Any;
            isAnyType = true;
        }
        else {
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::UntypedElement, name);
        }
    }

    // if element belongs to a compelx type
    if (typeInfo != 0) {

        contentSpecNode = typeInfo->getContentSpec();
        contentSpecType = (SchemaElementDecl::ModelTypes) typeInfo->getContentType();
        scopeDefined = typeInfo->getScopeDefined();
        validator = typeInfo->getDatatypeValidator();
    }

    // if element belongs to a simple type
    if (validator != 0) {
        contentSpecType = SchemaElementDecl::Simple;
    }

    // Now we can handle validation etc. of default and fixed attributes,
    // since we finally have all the type information.
    if(fixed && *fixed) {
        deflt = fixed;
    }

    if(deflt && *deflt) {

        try {
            if(validator == 0) { // in this case validate according to xs:string
                fDatatypeRegistry->getDatatypeValidator(SchemaSymbols::fgDT_STRING)->validate(deflt);
            } else {
                validator->validate(deflt);
            }
        }
        catch (const XMLException& excep) {
            reportSchemaError(elem, XMLUni::fgValidityDomain, XMLValid::DisplayErrorMessage, excep.getMessage());
        }
        catch(...) {
            reportSchemaError(elem, XMLUni::fgValidityDomain, XMLValid::DatatypeValidationFailure, deflt);
        }

        if(typeInfo != 0 &&
           contentSpecType != SchemaElementDecl::Simple &&
           contentSpecType != SchemaElementDecl::Mixed_Simple &&
           contentSpecType != SchemaElementDecl::Mixed_Complex) {
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::NotSimpleOrMixedElement, name);
        }

        if(typeInfo != 0 &&
           ((contentSpecType == SchemaElementDecl::Mixed_Complex
             || contentSpecType == SchemaElementDecl::Mixed_Simple)
            && !emptiableParticle(contentSpecNode))) {
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::EmptiableMixedContent, name);
        }

        if (validator && (validator->getType() == DatatypeValidator::ID)) {
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::ElemIDValueConstraint, name, deflt);
        }
    }

    // set element information, but first check for duplicate elements with
    // different types.
    if (isDuplicate) {

        DatatypeValidator* eltDV = elemDecl->getDatatypeValidator();
        ComplexTypeInfo*   eltTypeInfo = elemDecl->getComplexTypeInfo();

        if ( (eltTypeInfo != typeInfo) || (eltDV != validator))  {
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::DuplicateElementDeclaration, name);
        }
    }
    else {

        elemDecl->setDatatypeValidator(validator);
        elemDecl->setComplexTypeInfo(typeInfo);
        elemDecl->setDefaultValue(deflt);
        elemDecl->setModelType(contentSpecType);
        elemDecl->setContentSpec(contentSpecNode);

        if (isAnyType) {
            elemDecl->setAttWildCard(new (fMemoryManager) SchemaAttDef(XMLUni::fgZeroLenString,
                                                      XMLUni::fgZeroLenString,
                                                      fEmptyNamespaceURI, XMLAttDef::Any_Any,
                                                      XMLAttDef::ProcessContents_Lax));
        }

        // key/keyref/unique processing
        DOMElement* ic = XUtil::getFirstChildElementNS(elem, fgIdentityConstraints,
                                                         SchemaSymbols::fgURI_SCHEMAFORSCHEMA, 3);
        ValueVectorOf<DOMElement*>* icNodes = 0;

        while (ic != 0) {

            if (XMLString::equals(ic->getLocalName(), SchemaSymbols::fgELT_KEY)) {
                traverseKey(ic, elemDecl);
            }
            else if (XMLString::equals(ic->getLocalName(), SchemaSymbols::fgELT_UNIQUE)) {
                traverseUnique(ic, elemDecl);
            }
            else {

                if (!icNodes) {
                    icNodes = new (fMemoryManager) ValueVectorOf<DOMElement*>(8);
                }

                icNodes->addElement(ic);
            }

            ic = XUtil::getNextSiblingElementNS(ic, fgIdentityConstraints,
                                                SchemaSymbols::fgURI_SCHEMAFORSCHEMA, 3);
        }

        if (icNodes) {

            if (!fIC_ElementsNS) {

                fIC_ElementsNS = new (fMemoryManager) RefHashTableOf<ElemVector>(13);
                fIC_NamespaceDepthNS = new (fMemoryManager) RefHashTableOf<ValueVectorOf<unsigned int> >(13);
                fIC_NodeListNS = new (fMemoryManager) RefHashTableOf<ValueVectorOf<DOMElement*> >(29, true, new (fMemoryManager) HashPtr());
            }

            if (fIC_ElementsNS->containsKey(fTargetNSURIString)) {

                fIC_Elements = fIC_ElementsNS->get(fTargetNSURIString);
                fIC_NamespaceDepth = fIC_NamespaceDepthNS->get(fTargetNSURIString);
            }

            if (!fIC_Elements) {

                fIC_Elements = new (fMemoryManager) ValueVectorOf<SchemaElementDecl*>(8);
                fIC_NamespaceDepth = new (fMemoryManager) ValueVectorOf<unsigned int>(8);
                fIC_ElementsNS->put((void*) fTargetNSURIString, fIC_Elements);
                fIC_NamespaceDepthNS->put((void*) fTargetNSURIString, fIC_NamespaceDepth);
            }

            fIC_NodeListNS->put(elemDecl, icNodes);
            fIC_Elements->addElement(elemDecl);
            fIC_NamespaceDepth->addElement(fSchemaInfo->getNamespaceScopeLevel());
        }
    }

    return new (fMemoryManager) QName(*elemDecl->getElementName());
}

const XMLCh* TraverseSchema::traverseNotationDecl(const DOMElement* const elem) {

    // ------------------------------------------------------------------
    // Check attributes
    // ------------------------------------------------------------------
    fAttributeCheck.checkAttributes(elem, GeneralAttributeCheck::E_Notation, this, true);

    // ------------------------------------------------------------------
    // Process notation attributes/elements
    // ------------------------------------------------------------------
    const XMLCh* name = getElementAttValue(elem, SchemaSymbols::fgATT_NAME);
    bool         nameEmpty = (!name || !*name) ? true : false;

    if (nameEmpty) {
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::NoNameGlobalElement,
                          SchemaSymbols::fgELT_NOTATION);
        return 0;
    }

    if (fNotationRegistry->containsKey(name, fTargetNSURI)) {
        return name;
    }

    const XMLCh* publicId = getElementAttValue(elem, SchemaSymbols::fgATT_PUBLIC);
    const XMLCh* systemId = getElementAttValue(elem, SchemaSymbols::fgATT_SYSTEM);

    fNotationRegistry->put((void*) fStringPool->getValueForId(fStringPool->addOrFind(name)),
                           fTargetNSURI, 0);

    //we don't really care if something inside <notation> is wrong..
    checkContent(elem, XUtil::getFirstChildElement(elem), true);

    return name;
}

const XMLCh* TraverseSchema::traverseNotationDecl(const DOMElement* const elem,
                                                  const XMLCh* const name,
                                                  const XMLCh* const uriStr) {

    unsigned int uriId = fURIStringPool->addOrFind(uriStr);
    SchemaInfo*  saveInfo = fSchemaInfo;

    if (fTargetNSURI != (int) uriId) {

        // Make sure that we have an explicit import statement.
        // Clause 4 of Schema Representation Constraint:
        // http://www.w3.org/TR/xmlschema-1/#src-resolve
        unsigned int uriId = fURIStringPool->addOrFind(uriStr);

        if (!fSchemaInfo->isImportingNS(uriId)) {

            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidNSReference, uriStr);
            return 0;
        }

        Grammar* grammar = fGrammarResolver->getGrammar(uriStr);

        if (grammar == 0 || grammar->getGrammarType() != Grammar::SchemaGrammarType) {

            reportSchemaError(elem, XMLUni::fgValidityDomain, XMLValid::GrammarNotFound, uriStr);
            return 0;
        }

        SchemaInfo* impInfo = fSchemaInfo->getImportInfo(uriId);

        if (!impInfo || impInfo->getProcessed()) {

            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::TypeNotFound, uriStr, name);
            return 0;
        }

        fSchemaInfo = impInfo;
        fTargetNSURI = fSchemaInfo->getTargetNSURI();
    }

    DOMElement* notationElem = fSchemaInfo->getTopLevelComponent(SchemaInfo::C_Notation,
        SchemaSymbols::fgELT_NOTATION, name, &fSchemaInfo);

    if (notationElem == 0) {

        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::Notation_DeclNotFound, uriStr, name);
        return 0;
    }

    const XMLCh* notationName = traverseNotationDecl(notationElem);

    fSchemaInfo = saveInfo;
    fTargetNSURI = fSchemaInfo->getTargetNSURI();

    return notationName;
}

DatatypeValidator*
TraverseSchema::traverseByList(const DOMElement* const rootElem,
                               const DOMElement* const contentElem,
                               const XMLCh* const typeName,
                               const XMLCh* const qualifiedName,
                               const int finalSet) {

    DatatypeValidator* baseValidator = 0;
    const XMLCh*       baseTypeName = getElementAttValue(contentElem, SchemaSymbols::fgATT_ITEMTYPE);

    fAttributeCheck.checkAttributes(contentElem, GeneralAttributeCheck::E_List, this);

    if (XUtil::getNextSiblingElement(contentElem) != 0) {
        reportSchemaError(contentElem, XMLUni::fgXMLErrDomain, XMLErrs::SimpleTypeContentError);
    }

    DOMElement*      content = 0;

    if (!baseTypeName || !*baseTypeName) { // must 'see' <simpleType>

        content = checkContent(rootElem, XUtil::getFirstChildElement(contentElem), false);

        if (!content) {

            reportSchemaError(contentElem, XMLUni::fgXMLErrDomain, XMLErrs::ExpectedSimpleTypeInList, typeName);
            popCurrentTypeNameStack();
            return 0;
        }

        if (XMLString::equals(content->getLocalName(), SchemaSymbols::fgELT_SIMPLETYPE)) {
            baseValidator = checkForSimpleTypeValidator(content, SchemaSymbols::XSD_LIST);
        }
        else {

            reportSchemaError(content, XMLUni::fgXMLErrDomain, XMLErrs::ListUnionRestrictionError, typeName);
            popCurrentTypeNameStack();
            return 0;
        }

        content = XUtil::getNextSiblingElement(content);
    }
    else { // base was provided - get proper validator

        baseValidator = findDTValidator(contentElem, typeName, baseTypeName, SchemaSymbols::XSD_LIST);
        content = checkContent(rootElem, XUtil::getFirstChildElement(contentElem), true);
    }

    DatatypeValidator* newDV = 0;
	
    if (baseValidator) {

        if (!baseValidator->isAtomic()) {
            reportSchemaError(contentElem, XMLUni::fgXMLErrDomain, XMLErrs::AtomicItemType, baseTypeName);
        }
        else {

            // 'content' should be empty
            // If an annotation was encountered we have already traversed it in
            // checkContent in the case of a base provided (only allowed child is
            // an annotation).
            if (content != 0) { // report an error and continue
                reportSchemaError(content, XMLUni::fgXMLErrDomain, XMLErrs::SimpleTypeDerivationByListError, typeName);
            }

            // create & register validator for "generated" type
            try {
                newDV = fDatatypeRegistry->createDatatypeValidator(
                    qualifiedName, baseValidator, 0, 0, true, finalSet);
            }
            catch (const XMLException& excep) {
                reportSchemaError(contentElem, XMLUni::fgValidityDomain, XMLValid::DisplayErrorMessage, excep.getMessage());
            }
            catch(...) {
                reportSchemaError(contentElem, XMLUni::fgXMLErrDomain,
                                  XMLErrs::DatatypeValidatorCreationError, typeName);
            }
        }
    }

    popCurrentTypeNameStack();
    return newDV;
}

DatatypeValidator*
TraverseSchema::traverseByRestriction(const DOMElement* const rootElem,
                                      const DOMElement* const contentElem,
                                      const XMLCh* const typeName,
                                      const XMLCh* const qualifiedName,
                                      const int finalSet) {

    DatatypeValidator* baseValidator = 0;
    DatatypeValidator* newDV = 0;
    const XMLCh*       baseTypeName = getElementAttValue(contentElem, SchemaSymbols::fgATT_BASE);

    fAttributeCheck.checkAttributes(contentElem, GeneralAttributeCheck::E_Restriction, this);

    if (XUtil::getNextSiblingElement(contentElem) != 0) {
        reportSchemaError(contentElem, XMLUni::fgXMLErrDomain, XMLErrs::SimpleTypeContentError);
    }

    DOMElement* content = 0;

    if (!baseTypeName || !*baseTypeName) { // must 'see' <simpleType>

        content = checkContent(rootElem, XUtil::getFirstChildElement(contentElem), false);

        if (content == 0) {

            reportSchemaError(contentElem, XMLUni::fgXMLErrDomain, XMLErrs::ExpectedSimpleTypeInRestriction);
            popCurrentTypeNameStack();
            return 0;
        }

        if (XMLString::equals(content->getLocalName(), SchemaSymbols::fgELT_SIMPLETYPE)) {
            baseValidator = checkForSimpleTypeValidator(content);
        }
        else {

            reportSchemaError(content, XMLUni::fgXMLErrDomain, XMLErrs::ListUnionRestrictionError, typeName);
            popCurrentTypeNameStack();
            return 0;
        }

        // Check for facets
        content = XUtil::getNextSiblingElement(content);
    }
    else { // base was provided - get proper validator

        baseValidator = findDTValidator(contentElem, typeName, baseTypeName, SchemaSymbols::XSD_RESTRICTION);
        content = checkContent(rootElem, XUtil::getFirstChildElement(contentElem), true);
    }

    if (baseValidator) {

        // Get facets if any existing
        RefHashTableOf<KVStringPair>* facets = 0;
        RefArrayVectorOf<XMLCh>*      enums = 0;
        XMLBuffer                     pattern(128);
        XMLCh                         fixedFlagStr[16];
        unsigned int                  fixedFlag = 0;
        unsigned short                scope = 0;
        bool                          isFirstPattern = true;

        while (content != 0) {

            if (content->getNodeType() == DOMNode::ELEMENT_NODE) {

                const XMLCh* facetName = content->getLocalName();

                try {
                    scope = fAttributeCheck.getFacetId(facetName);
                }
                catch (...) {

                    reportSchemaError(content, XMLUni::fgXMLErrDomain, XMLErrs::InvalidFacetName, facetName);
                    content = XUtil::getNextSiblingElement(content);
                    continue;
                }

                const XMLCh* attValue = content->getAttribute(SchemaSymbols::fgATT_VALUE);
                fAttributeCheck.checkAttributes(content, scope, this);

                if (facets == 0) {
                    facets = new (fMemoryManager) RefHashTableOf<KVStringPair>(29, true);
                }

                if (XMLString::equals(facetName, SchemaSymbols::fgELT_ENUMERATION)) {

                    // REVISIT
                    // if validator is a notation datatype validator, we need
                    // to get the qualified name first before adding it to the
                    // enum buffer
                    if (!enums) {
                        enums = new (fMemoryManager) RefArrayVectorOf<XMLCh>(8, true);
                    }

                    if (baseValidator->getType() == DatatypeValidator::NOTATION) {

                        const XMLCh* localPart = getLocalPart(attValue);
                        const XMLCh* prefix = getPrefix(attValue);
                        const XMLCh* uriStr = (prefix && *prefix) ? resolvePrefixToURI(content, prefix) : fTargetNSURIString;
                        unsigned int uriId = fURIStringPool->addOrFind(uriStr);

                        if (!fNotationRegistry->containsKey(localPart, uriId)) {
                            traverseNotationDecl(content, localPart, uriStr);
                        }

                        fBuffer.set(uriStr);
                        fBuffer.append(chColon);
                        fBuffer.append(localPart);
                        enums->addElement(XMLString::replicate(fBuffer.getRawBuffer()));
                    }
                    else {
                        enums->addElement(XMLString::replicate(attValue));
                    }
                }
                else if (XMLString::equals(facetName, SchemaSymbols::fgELT_PATTERN)) {

                    if (isFirstPattern) { // fBuffer.isEmpty() - overhead call

                        isFirstPattern = false;
                        pattern.set(attValue);
                    }
                    else { //datatypes: 5.2.4 pattern

                        pattern.append(chPipe);
                        pattern.append(attValue);
                    }
                }
                else {

                    if (facets->containsKey(facetName)) {
                        reportSchemaError(content, XMLUni::fgXMLErrDomain, XMLErrs::DuplicateFacet, facetName);
                    }
                    else {

                        if (XMLString::equals(facetName, SchemaSymbols::fgELT_WHITESPACE)
                            && baseValidator->getType() != DatatypeValidator::String
                            && !XMLString::equals(attValue, SchemaSymbols::fgWS_COLLAPSE)) {
                            reportSchemaError(content, XMLUni::fgXMLErrDomain, XMLErrs::WS_CollapseExpected, attValue);
                        }
                        else {

                            const XMLCh* facetStr = fStringPool->getValueForId(fStringPool->addOrFind(facetName));
                            facets->put((void*) facetStr, new (fMemoryManager) KVStringPair(facetStr, attValue));
                            checkFixedFacet(content, facetStr, baseValidator, fixedFlag);
                        }
                    }
                }

                // REVISIT
                // check for annotation content - we are not checking whether the
                // return is empty or not. If not empty we should report an error
                checkContent(rootElem, XUtil::getFirstChildElement(content), true);
            }

            content = XUtil::getNextSiblingElement(content);
        } // end while

        if (!pattern.isEmpty()) {
            facets->put((void*) SchemaSymbols::fgELT_PATTERN,
                        new (fMemoryManager) KVStringPair(SchemaSymbols::fgELT_PATTERN, pattern.getRawBuffer()));
        }

        if (fixedFlag) {

            XMLString::binToText(fixedFlag, fixedFlagStr, 15, 10);
            facets->put((void*) SchemaSymbols::fgATT_FIXED,
                        new (fMemoryManager) KVStringPair(SchemaSymbols::fgATT_FIXED, fixedFlagStr));
        }

        try {
            newDV = fDatatypeRegistry->createDatatypeValidator(qualifiedName, baseValidator, facets, enums, false, finalSet);
        }
        catch (const XMLException& excep) {
            reportSchemaError(contentElem, XMLUni::fgValidityDomain, XMLValid::DisplayErrorMessage, excep.getMessage());
        }
        catch(...) {
            reportSchemaError(contentElem, XMLUni::fgXMLErrDomain,
                              XMLErrs::DatatypeValidatorCreationError, typeName);
        }
    }

    popCurrentTypeNameStack();
    return newDV;
}


DatatypeValidator*
TraverseSchema::traverseByUnion(const DOMElement* const rootElem,
                                const DOMElement* const contentElem,
                                const XMLCh* const typeName,
                                const XMLCh* const qualifiedName,
                                const int finalSet,
                                int baseRefContext) {

    fAttributeCheck.checkAttributes(contentElem, GeneralAttributeCheck::E_Union, this);

    if (XUtil::getNextSiblingElement(contentElem) != 0) {
        reportSchemaError(contentElem, XMLUni::fgXMLErrDomain, XMLErrs::SimpleTypeContentError);
    }

    int                             size = 1;
    const XMLCh*                    baseTypeName = getElementAttValue(contentElem, SchemaSymbols::fgATT_MEMBERTYPES);
    DatatypeValidator*              baseValidator = 0;
    RefVectorOf<DatatypeValidator>* validators = new (fMemoryManager) RefVectorOf<DatatypeValidator>(4, false);
    Janitor<DVRefVector>            janValidators(validators);
    DOMElement*                     content = 0;

    if (baseTypeName && *baseTypeName) { //base was provided - get proper validator.

        XMLStringTokenizer unionMembers(baseTypeName);
        int             tokCount = unionMembers.countTokens();

        for (int i = 0; i < tokCount; i++) {

            const XMLCh* memberTypeName = unionMembers.nextToken();

            baseValidator = findDTValidator(contentElem, typeName, memberTypeName, SchemaSymbols::XSD_UNION);

            if (baseValidator == 0) {

                popCurrentTypeNameStack();
                return 0;
            }

            validators->addElement(baseValidator);
        }

        content = checkContent(rootElem, XUtil::getFirstChildElement(contentElem), true);
    }
    else { // must 'see' <simpleType>

        content = checkContent(rootElem, XUtil::getFirstChildElement(contentElem), false);

        if (content == 0) {

            reportSchemaError(contentElem, XMLUni::fgXMLErrDomain, XMLErrs::ExpectedSimpleTypeInUnion, typeName);
            popCurrentTypeNameStack();
            return 0;
        }

        if (!XMLString::equals(content->getLocalName(), SchemaSymbols::fgELT_SIMPLETYPE)) {

            reportSchemaError(content, XMLUni::fgXMLErrDomain, XMLErrs::ListUnionRestrictionError, typeName);
            popCurrentTypeNameStack();
            return 0;
        }
    }

    // process union content of simpleType children if any
    while (content != 0) {

        if (XMLString::equals(content->getLocalName(), SchemaSymbols::fgELT_SIMPLETYPE)) {

            baseValidator = checkForSimpleTypeValidator(content, baseRefContext | SchemaSymbols::XSD_UNION);

            if (baseValidator == 0) {

                popCurrentTypeNameStack();
                return 0;
            }

            validators->addElement(baseValidator);
        }
        else {
            // REVISIT - should we break. For now, we will continue and move to
            // the next sibling
            reportSchemaError(content, XMLUni::fgXMLErrDomain, XMLErrs::ListUnionRestrictionError, typeName);
        }

        content   = XUtil::getNextSiblingElement(content);
    } // end while

    DatatypeValidator* newDV = 0;
    janValidators.orphan();

    try {
        newDV = fDatatypeRegistry->createDatatypeValidator(qualifiedName, validators, finalSet);
    }
    catch (const XMLException& excep) {
        reportSchemaError(contentElem, XMLUni::fgValidityDomain, XMLValid::DisplayErrorMessage, excep.getMessage());
    }
    catch(...) {
        reportSchemaError(contentElem, XMLUni::fgXMLErrDomain,
                          XMLErrs::DatatypeValidatorCreationError, typeName);
    }

    popCurrentTypeNameStack();
    return newDV;
}


/**
  * Traverse SimpleContent Declaration
  *
  *   <simpleContent
  *     id = ID
  *     {any attributes with non-schema namespace...}>
  *
  *     Content: (annotation? , (restriction | extension))
  *   </simpleContent>
  *
  *   <restriction
  *     base = QNAME
  *     id = ID
  *     {any attributes with non-schema namespace...}>
  *
  *     Content: (annotation?, (simpleType?, (minExclusive | minInclusive
  *               | maxExclusive | maxInclusive | totalDigits | fractionDigits
  *               | length | minLength | maxLength | enumeration | pattern
  *               | whiteSpace)*)?, ((attribute | attributeGroup)* , anyAttribute?))
  *   </restriction>
  *
  *   <extension
  *     base = QNAME
  *     id = ID
  *     {any attributes with non-schema namespace...}>
  *     Content: (annotation? , ((attribute | attributeGroup)* , anyAttribute?))
  *   </extension>
  *
  */
void TraverseSchema::traverseSimpleContentDecl(const XMLCh* const typeName,
                                               const XMLCh* const qualifiedName,
                                               const DOMElement* const contentDecl,
                                               ComplexTypeInfo* const typeInfo)
{
    // -----------------------------------------------------------------------
    // Check Attributes
    // -----------------------------------------------------------------------
    fAttributeCheck.checkAttributes(contentDecl, GeneralAttributeCheck::E_SimpleContent, this);

    // -----------------------------------------------------------------------
    // Set the content type to be simple, and initialize content spec handle
    // -----------------------------------------------------------------------
    typeInfo->setContentType(SchemaElementDecl::Simple);

    DOMElement* simpleContent = checkContent(contentDecl, XUtil::getFirstChildElement(contentDecl), false);

    // If there are no children, return
    if (simpleContent == 0) {

        reportSchemaError(contentDecl, XMLUni::fgXMLErrDomain, XMLErrs::EmptySimpleTypeContent);
        throw TraverseSchema::InvalidComplexTypeInfo;
    }

    // -----------------------------------------------------------------------
    // The content should be either "restriction" or "extension"
    // -----------------------------------------------------------------------
    const XMLCh* const contentName = simpleContent->getLocalName();

    if (XMLString::equals(contentName, SchemaSymbols::fgATTVAL_RESTRICTION)) {

        fAttributeCheck.checkAttributes(simpleContent, GeneralAttributeCheck::E_Restriction, this);
        typeInfo->setDerivedBy(SchemaSymbols::XSD_RESTRICTION);
    }
    else if (XMLString::equals(contentName, SchemaSymbols::fgATTVAL_EXTENSION)) {

        fAttributeCheck.checkAttributes(simpleContent, GeneralAttributeCheck::E_Extension, this);
        typeInfo->setDerivedBy(SchemaSymbols::XSD_EXTENSION);
    }
    else {
        reportSchemaError(simpleContent, XMLUni::fgXMLErrDomain, XMLErrs::InvalidSimpleContent);
        throw TraverseSchema::InvalidComplexTypeInfo;
    }

    // -----------------------------------------------------------------------
    // Handle the base type name
    // -----------------------------------------------------------------------
    const XMLCh* baseName = getElementAttValue(simpleContent, SchemaSymbols::fgATT_BASE);

    if (!baseName || !*baseName) {

        reportSchemaError(simpleContent, XMLUni::fgXMLErrDomain, XMLErrs::UnspecifiedBase);
        throw TraverseSchema::InvalidComplexTypeInfo;
    }

    const XMLCh* prefix = getPrefix(baseName);
    const XMLCh* localPart = getLocalPart(baseName);
    const XMLCh* uri = resolvePrefixToURI(simpleContent, prefix);
    DatatypeValidator* baseValidator = getDatatypeValidator(uri, localPart);

    if (baseValidator != 0) {

        // check that the simpleType does not preclude derivation by extension
        if ((baseValidator->getFinalSet() & SchemaSymbols::XSD_EXTENSION) == typeInfo->getDerivedBy()) {

            reportSchemaError(simpleContent, XMLUni::fgXMLErrDomain, XMLErrs::DisallowedSimpleTypeExtension,
                              baseName, typeName);
            throw TraverseSchema::InvalidComplexTypeInfo;
        }

        typeInfo->setBaseComplexTypeInfo(0);
        typeInfo->setBaseDatatypeValidator(baseValidator);
    }
    else {

        // check for 'anyType'
        if (XMLString::equals(uri, SchemaSymbols::fgURI_SCHEMAFORSCHEMA)
            && XMLString::equals(localPart, SchemaSymbols::fgATTVAL_ANYTYPE)) {

            reportSchemaError(simpleContent, XMLUni::fgXMLErrDomain, XMLErrs::InvalidSimpleContentBase, baseName);
            throw TraverseSchema::InvalidComplexTypeInfo;
        }

        processBaseTypeInfo(simpleContent, baseName, localPart, uri, typeInfo);
    }

    // check that the base isn't a complex type with complex content
    // and that derivation method is not included in 'final'
    ComplexTypeInfo* baseTypeInfo = typeInfo->getBaseComplexTypeInfo();
    bool simpleTypeRequired = false;

    if (baseTypeInfo) {

        if (baseTypeInfo->getContentType() != SchemaElementDecl::Simple) {

            // Schema Errata: E1-27
            if (typeInfo->getDerivedBy() == SchemaSymbols::XSD_RESTRICTION
                && ((baseTypeInfo->getContentType() == SchemaElementDecl::Mixed_Simple
                    || baseTypeInfo->getContentType() == SchemaElementDecl::Mixed_Complex)
                    && emptiableParticle(baseTypeInfo->getContentSpec()))) {
                simpleTypeRequired = true;
            }
            else {
                reportSchemaError(simpleContent, XMLUni::fgXMLErrDomain, XMLErrs::InvalidSimpleContentBase, baseName);
                throw TraverseSchema::InvalidComplexTypeInfo;
            }
        }

        if ((baseTypeInfo->getFinalSet() & typeInfo->getDerivedBy()) != 0) {
            reportSchemaError(simpleContent, XMLUni::fgXMLErrDomain, XMLErrs::ForbiddenDerivation, baseName);
            throw TraverseSchema::InvalidComplexTypeInfo;
        }
    }

    // -----------------------------------------------------------------------
    // Process the content of the derivation
    // -----------------------------------------------------------------------
    //Skip over any annotations in the restriction or extension elements
    DOMElement* content = checkContent(simpleContent, XUtil::getFirstChildElement(simpleContent), true);

    if (typeInfo->getDerivedBy() == SchemaSymbols::XSD_RESTRICTION) {

        //Schema Spec: 5.11: Complex Type Definition Properties Correct: 2
        if (typeInfo->getBaseDatatypeValidator() != 0) {

            reportSchemaError(simpleContent, XMLUni::fgXMLErrDomain, XMLErrs::InvalidComplexTypeBase, baseName);
            throw TraverseSchema::InvalidComplexTypeInfo;
        }
        else {
           typeInfo->setBaseDatatypeValidator(baseTypeInfo->getDatatypeValidator());
        }

        if (content != 0) {

            // ---------------------------------------------------------------
            // There may be a simple type definition in the restriction
            // element. The data type validator will be based on it, if
            // specified
            // ---------------------------------------------------------------
            if (XMLString::equals(content->getLocalName(), SchemaSymbols::fgELT_SIMPLETYPE)) {

                DatatypeValidator* simpleTypeDV = traverseSimpleTypeDecl(content, false);

                if (simpleTypeDV) {

                    // Check that the simpleType validator is validly derived
                    // from base
                    DatatypeValidator* baseDV = typeInfo->getBaseDatatypeValidator();

                    if (baseDV  && !baseDV->isSubstitutableBy(simpleTypeDV)) {

                        reportSchemaError(content, XMLUni::fgXMLErrDomain, XMLErrs::InvalidContentRestriction);
                        throw TraverseSchema::InvalidComplexTypeInfo;
                    }

                    typeInfo->setBaseDatatypeValidator(simpleTypeDV);
                    content = XUtil::getNextSiblingElement(content);
                }
                else {
                    throw TraverseSchema::InvalidComplexTypeInfo;
                }
            }
            // Schema Errata E1-27
            // Complex Type Definition Restriction OK: 2.2
            else if (simpleTypeRequired) {

                reportSchemaError(content, XMLUni::fgXMLErrDomain, XMLErrs::CT_SimpleTypeChildRequired);
                throw TraverseSchema::InvalidComplexTypeInfo;
            }

            // ---------------------------------------------------------------
            // Build up the facet info
            // ---------------------------------------------------------------
            RefHashTableOf<KVStringPair>*  facets = 0;
            RefArrayVectorOf<XMLCh>*       enums = 0;
            XMLBuffer                      pattern(128);
            XMLCh                          fixedFlagStr[16];
            unsigned int                   fixedFlag = 0;
            unsigned short                 scope = 0;
            bool                           isFirstPattern = true;

            while (content != 0) {

                const XMLCh* facetName = content->getLocalName();

                // if not a valid facet, break from the loop
                try {
                    scope = fAttributeCheck.getFacetId(facetName);
                }
                catch(...) {
                    break;
                }

                if (content->getNodeType() == DOMNode::ELEMENT_NODE) {

                    fAttributeCheck.checkAttributes(content, scope, this);

                    const XMLCh* attValue = content->getAttribute(SchemaSymbols::fgATT_VALUE);

                    if (facets == 0) {
                        facets = new (fMemoryManager) RefHashTableOf<KVStringPair>(29, true);
                    }

                    if (XMLString::equals(facetName, SchemaSymbols::fgELT_ENUMERATION)) {

                        if (!enums) {
                            enums = new (fMemoryManager) RefArrayVectorOf<XMLCh>(8, true);
                        }

                        enums->addElement(XMLString::replicate(attValue));
                    }
                    else if (XMLString::equals(facetName, SchemaSymbols::fgELT_PATTERN)) {

                        if (isFirstPattern) { // fBuffer.isEmpty() - overhead call

                            isFirstPattern = false;
                            pattern.set(attValue);
                        }
                        else { //datatypes: 5.2.4 pattern

                            pattern.append(chPipe);
                            pattern.append(attValue);
                        }
                    }
                    else {

                        if (facets->containsKey(facetName)) {
                            reportSchemaError(content, XMLUni::fgXMLErrDomain, XMLErrs::DuplicateFacet, facetName);
                        }
                        else {

                            const XMLCh* facetNameStr =
                                fStringPool->getValueForId(fStringPool->addOrFind(facetName));

                            facets->put((void*) facetNameStr, new (fMemoryManager) KVStringPair(facetNameStr, attValue));
                            checkFixedFacet(content, facetNameStr, typeInfo->getBaseDatatypeValidator(), fixedFlag);
                        }
                    }
                }

                content = XUtil::getNextSiblingElement(content);
            }

            if (facets) {

                if (!pattern.isEmpty()) {
                    facets->put
                    (
                        (void*) SchemaSymbols::fgELT_PATTERN,
                        new (fMemoryManager) KVStringPair
                            (
                                SchemaSymbols::fgELT_PATTERN,
                                pattern.getRawBuffer()
                            )
                    );
                }

                if (fixedFlag) {

                    XMLString::binToText(fixedFlag, fixedFlagStr, 15, 10);
                    facets->put((void*) SchemaSymbols::fgATT_FIXED,
                        new (fMemoryManager) KVStringPair(SchemaSymbols::fgATT_FIXED, fixedFlagStr));
                }

                try {

                    typeInfo->setDatatypeValidator
                    (
                        fDatatypeRegistry->createDatatypeValidator
                        (
                            qualifiedName,
                            typeInfo->getBaseDatatypeValidator(),
                            facets, enums, false, 0
                        )
                    );
                }
                catch (const XMLException& excep) {
                    reportSchemaError(simpleContent, XMLUni::fgValidityDomain, XMLValid::DisplayErrorMessage, excep.getMessage());
                }
                catch(...) {
                    reportSchemaError(simpleContent, XMLUni::fgXMLErrDomain, XMLErrs::DatatypeValidatorCreationError, typeName);
                }
            }
            else {
                typeInfo->setDatatypeValidator(typeInfo->getBaseDatatypeValidator());
            }
        }
        else {

            // Schema Errata E1-27
            // Complex Type Definition Restriction OK: 2.2
            if (simpleTypeRequired) {

                reportSchemaError(content, XMLUni::fgXMLErrDomain, XMLErrs::CT_SimpleTypeChildRequired);
                throw TraverseSchema::InvalidComplexTypeInfo;
            }

            typeInfo->setDatatypeValidator(typeInfo->getBaseDatatypeValidator());
        }
    } // end RESTRICTION
    else { // EXTENSION

        ComplexTypeInfo* baseTypeInfo = typeInfo->getBaseComplexTypeInfo();

        if (baseTypeInfo!= 0) {

            typeInfo->setBaseDatatypeValidator(baseTypeInfo->getDatatypeValidator());
        }

        typeInfo->setDatatypeValidator(typeInfo->getBaseDatatypeValidator());
    }

    // -----------------------------------------------------------------------
    // Process attributes if any
    // -----------------------------------------------------------------------
    processAttributes(simpleContent, content, baseName, localPart, uri, typeInfo);

    if (XUtil::getNextSiblingElement(simpleContent) != 0) {
        reportSchemaError(simpleContent, XMLUni::fgXMLErrDomain, XMLErrs::InvalidChildInSimpleContent);
    }

} // End of function traverseSimpleContentDecl

/**
  * Traverse complexContent Declaration
  *
  *   <complexContent
  *     id = ID
  *     mixed = boolean
  *     {any attributes with non-schema namespace...}>
  *
  *     Content: (annotation? , (restriction | extension))
  *   </complexContent>
  *
  *   <restriction
  *     base = QNAME
  *     id = ID
  *     {any attributes with non-schema namespace...}>
  *
  *     Content: (annotation? , (group | all | choice | sequence)?,
  *              ((attribute | attributeGroup)* , anyAttribute?))
  *   </restriction>
  *
  *   <extension
  *     base = QNAME
  *     id = ID
  *     {any attributes with non-schema namespace...}>
  *         Content: (annotation? , (group | all | choice | sequence)?,
  *                  ((attribute | attributeGroup)* , anyAttribute?))
  *   </extension>
  */
void TraverseSchema::traverseComplexContentDecl(const XMLCh* const typeName,
                                                const DOMElement* const contentDecl,
                                                ComplexTypeInfo* const typeInfo,
                                                const bool isMixed)
{
    // ------------------------------------------------------------------
    // Check attributes
    // ------------------------------------------------------------------
    fAttributeCheck.checkAttributes(contentDecl, GeneralAttributeCheck::E_ComplexContent, this);

    // -----------------------------------------------------------------------
    // Determine whether the content is mixed, or element-only
    // Setting here overrides any setting on the complex type decl
    // -----------------------------------------------------------------------
    const XMLCh* const mixed = getElementAttValue(contentDecl, SchemaSymbols::fgATT_MIXED);
    bool mixedContent = isMixed;

    if (mixed) {
        if (XMLString::equals(mixed, SchemaSymbols::fgATTVAL_TRUE)
            || XMLString::equals(mixed, fgValueOne)) {
            mixedContent = true;
        }
        else if (XMLString::equals(mixed, SchemaSymbols::fgATTVAL_FALSE)
                 || XMLString::equals(mixed, fgValueZero)) {
            mixedContent = false;
        }
    }

    // -----------------------------------------------------------------------
    // Since the type must have complex content, set the simple type validators
    // to null
    // -----------------------------------------------------------------------
    typeInfo->setDatatypeValidator(0);
    typeInfo->setBaseDatatypeValidator(0);

    DOMElement* complexContent = checkContent(contentDecl,XUtil::getFirstChildElement(contentDecl),false);

    // If there are no children, return
    if (complexContent == 0) {
       throw TraverseSchema::InvalidComplexTypeInfo;
    }

    // -----------------------------------------------------------------------
    // The content should be either "restriction" or "extension"
    // -----------------------------------------------------------------------
    const XMLCh* const complexContentName = complexContent->getLocalName();

    if (XMLString::equals(complexContentName, SchemaSymbols::fgELT_RESTRICTION)) {
        typeInfo->setDerivedBy(SchemaSymbols::XSD_RESTRICTION);
    }
    else if (XMLString::equals(complexContentName, SchemaSymbols::fgELT_EXTENSION)) {
        typeInfo->setDerivedBy(SchemaSymbols::XSD_EXTENSION);
    }
    else {

        reportSchemaError(complexContent, XMLUni::fgXMLErrDomain, XMLErrs::InvalidComplexContent);
        throw TraverseSchema::InvalidComplexTypeInfo;
    }

    // -----------------------------------------------------------------------
    // Handle the base type name
    // -----------------------------------------------------------------------
    const XMLCh* baseName = getElementAttValue(complexContent, SchemaSymbols::fgATT_BASE);

    if (!baseName || !*baseName) {

        reportSchemaError(complexContent, XMLUni::fgXMLErrDomain, XMLErrs::UnspecifiedBase);
        throw TraverseSchema::InvalidComplexTypeInfo;
    }

    const XMLCh* prefix = getPrefix(baseName);
    const XMLCh* localPart = getLocalPart(baseName);
    const XMLCh* uri = resolvePrefixToURI(complexContent, prefix);
    bool  isBaseAnyType = false;

    // -------------------------------------------------------------
    // check if the base is "anyType"
    // -------------------------------------------------------------
    if (XMLString::equals(uri, SchemaSymbols::fgURI_SCHEMAFORSCHEMA) &&
        XMLString::equals(localPart, SchemaSymbols::fgATTVAL_ANYTYPE)) {
        isBaseAnyType = true;
    }
    else {

        processBaseTypeInfo(complexContent, baseName, localPart, uri, typeInfo);

        //Check that the base is a complex type
        if (typeInfo->getBaseComplexTypeInfo() == 0)  {

            reportSchemaError(complexContent, XMLUni::fgXMLErrDomain, XMLErrs::BaseNotComplexType);
            throw TraverseSchema::InvalidComplexTypeInfo;
        }
    }

    if (fCurrentGroupInfo) // defer processing until later
        throw TraverseSchema::RecursingElement;

    // -----------------------------------------------------------------------
    // Process the content of the derivation
    // -----------------------------------------------------------------------
    //Skip over any annotations in the restriction or extension elements
    DOMElement* content = checkContent(complexContent, XUtil::getFirstChildElement(complexContent), true);

    processComplexContent(complexContent, typeName, content, typeInfo, baseName, localPart,
                          uri, mixedContent, isBaseAnyType);

    if (XUtil::getNextSiblingElement(complexContent) != 0) {
        reportSchemaError(complexContent, XMLUni::fgXMLErrDomain, XMLErrs::InvalidChildInComplexContent);
    }
}


/**
  * <anyAttribute
  *   id = ID
  *   namespace = ((##any | ##other) | list of (anyURI | (##targetNamespace | ##local)))>
  *   processContents = (lax | skip | strict) : strict
  *   Content: (annotation?)
  * </anyAttribute>
  */
SchemaAttDef* TraverseSchema::traverseAnyAttribute(const DOMElement* const elem) {

    // -----------------------------------------------------------------------
    // Check Attributes
    // -----------------------------------------------------------------------
    fAttributeCheck.checkAttributes(elem, GeneralAttributeCheck::E_AnyAttribute, this);

    // ------------------------------------------------------------------
    // First, handle any ANNOTATION declaration
    // ------------------------------------------------------------------
    if (checkContent(elem, XUtil::getFirstChildElement(elem), true) != 0) {
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::AnyAttributeContentError);
    }

    // ------------------------------------------------------------------
    // Get attributes
    // ------------------------------------------------------------------
    const XMLCh* const processContents =
            getElementAttValue(elem, SchemaSymbols::fgATT_PROCESSCONTENTS);
    const XMLCh* const nameSpace =
            getElementAttValue(elem, SchemaSymbols::fgATT_NAMESPACE);

    // ------------------------------------------------------------------
    // Set default att type based on 'processContents' value
    // ------------------------------------------------------------------
    XMLAttDef::DefAttTypes attDefType = XMLAttDef::ProcessContents_Strict;

    if ((!processContents || !*processContents)
        || XMLString::equals(processContents, SchemaSymbols::fgATTVAL_STRICT)) {
        // Do nothing - defaulted already
    }
    else if (XMLString::equals(processContents, SchemaSymbols::fgATTVAL_SKIP)) {
        attDefType = XMLAttDef::ProcessContents_Skip;
    }
    else if (XMLString::equals(processContents, SchemaSymbols::fgATTVAL_LAX)) {
        attDefType = XMLAttDef::ProcessContents_Lax;
    }

    // ------------------------------------------------------------------
    // Process 'namespace' attribute
    // ------------------------------------------------------------------
    int uriIndex = fEmptyNamespaceURI;
    XMLAttDef::AttTypes attType = XMLAttDef::Any_Any;
    ValueVectorOf<unsigned int> namespaceList(8);

    if ((!nameSpace || !*nameSpace)
        || XMLString::equals(nameSpace, SchemaSymbols::fgATTVAL_TWOPOUNDANY)) {
        // Do nothing - defaulted already
    }
    else if (XMLString::equals(nameSpace, SchemaSymbols::fgATTVAL_TWOPOUNDOTHER)) {

        attType = XMLAttDef::Any_Other;
        uriIndex = fTargetNSURI;
    }
    else {

        XMLStringTokenizer tokenizer(nameSpace);
        DatatypeValidator* anyURIDV = fDatatypeRegistry->getDatatypeValidator(SchemaSymbols::fgDT_ANYURI);

        attType = XMLAttDef::Any_List;

        while (tokenizer.hasMoreTokens()) {

            const XMLCh* token = tokenizer.nextToken();

            if (XMLString::equals(token, SchemaSymbols::fgATTVAL_TWOPOUNDLOCAL)) {
                uriIndex = fEmptyNamespaceURI;
            }
            else if (XMLString::equals(token, SchemaSymbols::fgATTVAL_TWOPOUNDTRAGETNAMESPACE)) {
                uriIndex = fTargetNSURI;
            }
            else {

                try {
                    anyURIDV->validate(token);
                }
                catch(const XMLException& excep) {
                    reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::DisplayErrorMessage, excep.getMessage());
                }
                uriIndex = fURIStringPool->addOrFind(token);
            }

            if (!namespaceList.containsElement(uriIndex)) {
                namespaceList.addElement(uriIndex);
            }
        }

        uriIndex = fEmptyNamespaceURI;
    }

    // ------------------------------------------------------------------
    // Create wildcard attribute
    // ------------------------------------------------------------------
    SchemaAttDef* attDef = new (fMemoryManager) SchemaAttDef(XMLUni::fgZeroLenString,
                                            XMLUni::fgZeroLenString,
                                            uriIndex, attType, attDefType);


    if (namespaceList.size()) {
       attDef->setNamespaceList(&namespaceList);
    }

    return attDef;
}

/**
  * <key
  *   id = ID
  *   name = NCName
  *   Content: (annotation?, (selector, field+))
  * </key>
  */
void TraverseSchema::traverseKey(const DOMElement* const icElem,
                                 SchemaElementDecl* const elemDecl) {

    // -----------------------------------------------------------------------
    // Check Attributes
    // -----------------------------------------------------------------------
    fAttributeCheck.checkAttributes(icElem, GeneralAttributeCheck::E_Key, this);

    // -----------------------------------------------------------------------
    // Create identity constraint
    // -----------------------------------------------------------------------
    const XMLCh* name = getElementAttValue(icElem, SchemaSymbols::fgATT_NAME);

    if (!name || !*name) {
        return;
    }

    if (!XMLString::isValidNCName(name)) {
        reportSchemaError(icElem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidDeclarationName,
                          SchemaSymbols::fgELT_KEY, name);
        return;
    }


    if (!fIdentityConstraintNames) {
        fIdentityConstraintNames = new (fMemoryManager) RefHash2KeysTableOf<IdentityConstraint>(29, false);
    }

    if (fIdentityConstraintNames->containsKey(name, fTargetNSURI)) {

        reportSchemaError(icElem, XMLUni::fgXMLErrDomain, XMLErrs::IC_DuplicateDecl, name);
        return;
    }

    IC_Key* icKey = new (fMemoryManager) IC_Key(name, elemDecl->getBaseName(), fMemoryManager);
    Janitor<IC_Key> janKey(icKey);

    fIdentityConstraintNames->put((void*) name, fTargetNSURI, icKey);

    // -----------------------------------------------------------------------
    // Get selector and fields
    // -----------------------------------------------------------------------
    if (!traverseIdentityConstraint(icKey, icElem)) {

        fIdentityConstraintNames->put((void*) name, fTargetNSURI, 0);
        return;
    }

    // -----------------------------------------------------------------------
    // Add key to element declaration
    // -----------------------------------------------------------------------
    elemDecl->addIdentityConstraint(icKey);
    janKey.orphan();
}

/**
  * <unique
  *   id = ID
  *   name = NCName
  *   Content: (annotation?, (selector, field+))
  * </unique>
  */
void TraverseSchema::traverseUnique(const DOMElement* const icElem,
                                    SchemaElementDecl* const elemDecl) {

    // -----------------------------------------------------------------------
    // Check Attributes
    // -----------------------------------------------------------------------
    fAttributeCheck.checkAttributes(icElem, GeneralAttributeCheck::E_Unique, this);

    // -----------------------------------------------------------------------
    // Create identity constraint
    // -----------------------------------------------------------------------
    const XMLCh* name = getElementAttValue(icElem, SchemaSymbols::fgATT_NAME);

    if (!name || !*name) {
        return;
    }

    if (!XMLString::isValidNCName(name)) {
        reportSchemaError(icElem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidDeclarationName,
                          SchemaSymbols::fgELT_UNIQUE, name);
        return;
    }

    if (!fIdentityConstraintNames) {
        fIdentityConstraintNames = new (fMemoryManager) RefHash2KeysTableOf<IdentityConstraint>(29, false);
    }

    if (fIdentityConstraintNames->containsKey(name, fTargetNSURI)) {

        reportSchemaError(icElem, XMLUni::fgXMLErrDomain, XMLErrs::IC_DuplicateDecl, name);
        return;
    }

    IC_Unique* icUnique = new (fMemoryManager) IC_Unique(name, elemDecl->getBaseName(), fMemoryManager);
    Janitor<IC_Unique> janUnique(icUnique);

    fIdentityConstraintNames->put((void*) name, fTargetNSURI, icUnique);

    // -----------------------------------------------------------------------
    // Get selector and fields
    // -----------------------------------------------------------------------
    if (!traverseIdentityConstraint(icUnique, icElem)) {

        fIdentityConstraintNames->put((void*) name, fTargetNSURI, 0);
        return;
    }

    // -----------------------------------------------------------------------
    // Add identity cosntraints to element declaration
    // -----------------------------------------------------------------------
    elemDecl->addIdentityConstraint(icUnique);
    janUnique.orphan();
}

/**
  * <keyref
  *   id = ID
  *   name = NCName
  *   refer = QName
  *   Content: (annotation?, (selector, field+))
  * </keyref>
  */
void TraverseSchema::traverseKeyRef(const DOMElement* const icElem,
                                    SchemaElementDecl* const elemDecl,
                                    const unsigned int namespaceDepth) {

    // -----------------------------------------------------------------------
    // Check Attributes
    // -----------------------------------------------------------------------
    fAttributeCheck.checkAttributes(icElem, GeneralAttributeCheck::E_KeyRef, this);

    // -----------------------------------------------------------------------
    // Verify that key reference "refer" attribute is valid
    // -----------------------------------------------------------------------
    const XMLCh* name = getElementAttValue(icElem, SchemaSymbols::fgATT_NAME);
    const XMLCh* refer = getElementAttValue(icElem, SchemaSymbols::fgATT_REFER);

    if ((!name || !*name) || (!refer || !*refer)) {
        return;
    }

    if (!XMLString::isValidNCName(name)) {
        reportSchemaError(icElem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidDeclarationName,
                          SchemaSymbols::fgELT_KEYREF, name);
        return;
    }

    const XMLCh* prefix = getPrefix(refer);
    const XMLCh* localPart = getLocalPart(refer);
    const XMLCh* uriStr = resolvePrefixToURI(icElem, prefix, namespaceDepth);
    IdentityConstraint* icKey = (fIdentityConstraintNames)
        ? fIdentityConstraintNames->get(localPart, fURIStringPool->addOrFind(uriStr)) : 0;

    if (!icKey) {
        reportSchemaError(icElem, XMLUni::fgXMLErrDomain, XMLErrs::IC_KeyRefReferNotFound, name, localPart);
        return;
    }

    // -----------------------------------------------------------------------
    // Create identity constraint
    // -----------------------------------------------------------------------
    if(fIdentityConstraintNames->containsKey(name, fTargetNSURI)) {

        reportSchemaError(icElem, XMLUni::fgXMLErrDomain, XMLErrs::IC_DuplicateDecl, name);
        return;
    }

    IC_KeyRef* icKeyRef = new (fMemoryManager) IC_KeyRef(name, elemDecl->getBaseName(), icKey, fMemoryManager);
    Janitor<IC_KeyRef> janKeyRef(icKeyRef);

    fIdentityConstraintNames->put((void*) name, fTargetNSURI, icKeyRef);

    // -----------------------------------------------------------------------
    // Get selector and fields
    // -----------------------------------------------------------------------
    if (!traverseIdentityConstraint(icKeyRef, icElem)) {

        fIdentityConstraintNames->put((void*) name, fTargetNSURI, 0);
        return;
    }

    // -----------------------------------------------------------------------
    // Add key reference to element decl
    // -----------------------------------------------------------------------
    if (icKeyRef->getFieldCount() != icKey->getFieldCount()) {

        fIdentityConstraintNames->put((void*) name, fTargetNSURI, 0);
        reportSchemaError(icElem, XMLUni::fgXMLErrDomain, XMLErrs::IC_KeyRefCardinality,
                          name, icKey->getIdentityConstraintName());
    }
    else {

        elemDecl->addIdentityConstraint(icKeyRef);
        janKeyRef.orphan();
    }
}


bool TraverseSchema::traverseIdentityConstraint(IdentityConstraint* const ic,
                                                const DOMElement* const icElem) {

    // ------------------------------------------------------------------
    // First, handle any ANNOTATION declaration
    // ------------------------------------------------------------------
    DOMElement* elem = checkContent(icElem, XUtil::getFirstChildElement(icElem), false);

    // ------------------------------------------------------------------
    // Get selector
    // ------------------------------------------------------------------
    if (elem == 0) {

//        reportSchemaError(icElem, XMLUni::fgXMLErrDomain, XMLErrs::IC_BadContent);
        return false;
    }

    if (!XMLString::equals(elem->getLocalName(), SchemaSymbols::fgELT_SELECTOR)) {

        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::IC_BadContent);
        return false;
    }

    fAttributeCheck.checkAttributes(elem, GeneralAttributeCheck::E_Selector, this);
    checkContent(icElem, XUtil::getFirstChildElement(elem), true);

    // ------------------------------------------------------------------
    // Get xpath attribute
    // ------------------------------------------------------------------
    const XMLCh* xpathExpr = getElementAttValue(elem, SchemaSymbols::fgATT_XPATH, true);
    unsigned int xpathLen = XMLString::stringLen(xpathExpr);

    if (!xpathExpr || !xpathLen) {

        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::IC_XPathExprMissing);
        return false;
    }

    fBuffer.reset();

    unsigned int startIndex = 0;
    	
    while (startIndex < xpathLen) {

        if (!XMLString::startsWith(xpathExpr + startIndex, fgForwardSlash)
            && !XMLString::startsWith(xpathExpr + startIndex, fgDot)) {
            fBuffer.append(fgDotForwardSlash);
        }

        int chOffset = XMLString::indexOf(xpathExpr, chPipe, startIndex);

        if (chOffset == -1)
            break;

        fBuffer.append(xpathExpr + startIndex, chOffset + 1 - startIndex);
        startIndex = chOffset + 1;
    }

    if (startIndex < xpathLen)
        fBuffer.append(xpathExpr + startIndex);

    // ------------------------------------------------------------------
    // Parse xpath expression
    // ------------------------------------------------------------------
    try {

        XercesXPath* sXPath = new (fMemoryManager) XercesXPath(fBuffer.getRawBuffer(), fStringPool, fNamespaceScope, fEmptyNamespaceURI, true);
        IC_Selector* icSelector = new (fMemoryManager) IC_Selector(sXPath, ic);
        ic->setSelector(icSelector);
    }
    catch (const XPathException& e) {

        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::DisplayErrorMessage, e.getMessage());
        return false;
    }

    // ------------------------------------------------------------------
    // Get fields
    // ------------------------------------------------------------------
    elem = XUtil::getNextSiblingElement(elem);

    if (elem == 0) {

        reportSchemaError(icElem, XMLUni::fgXMLErrDomain, XMLErrs::IC_BadContent);
        return false;
    }

    while (elem != 0) {

        if (!XMLString::equals(elem->getLocalName(), SchemaSymbols::fgELT_FIELD)) {
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::IC_BadContent);
        }
        else {
            // General Attribute Checking
            fAttributeCheck.checkAttributes(elem, GeneralAttributeCheck::E_Field, this);
            checkContent(icElem, XUtil::getFirstChildElement(elem), true);

            // xpath expression parsing
            xpathExpr = getElementAttValue(elem, SchemaSymbols::fgATT_XPATH, true);

            if (!xpathExpr || !*xpathExpr) {

                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::IC_XPathExprMissing);
                return false;
            }

		    if (XMLString::startsWith(xpathExpr, fgForwardSlash)
			    || XMLString::startsWith(xpathExpr, fgDot)) {
                fBuffer.set(xpathExpr);
            }
            else {

                fBuffer.set(fgDotForwardSlash);
                fBuffer.append(xpathExpr);
            }

            try {

                XercesXPath* fieldXPath = new (fMemoryManager) XercesXPath(fBuffer.getRawBuffer(), fStringPool, fNamespaceScope, fEmptyNamespaceURI);
                IC_Field* icField = new (fMemoryManager) IC_Field(fieldXPath, ic);
                ic->addField(icField);
            }
            catch (const XPathException& e) {

                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::DisplayErrorMessage, e.getMessage());
                return false;
            }
		}

        elem = XUtil::getNextSiblingElement(elem);
    }

    if (ic->getFieldCount() == 0) {
        return false;
    }

    return true;
}

// ---------------------------------------------------------------------------
//  TraverseSchema: Helper methods
// ---------------------------------------------------------------------------
void TraverseSchema::retrieveNamespaceMapping(const DOMElement* const schemaRoot) {

    DOMNamedNodeMap* schemaEltAttrs = schemaRoot->getAttributes();
    bool seenXMLNS = false;
    int attrCount = schemaEltAttrs->getLength();

    for (int i = 0; i < attrCount; i++) {

        DOMNode* attribute = schemaEltAttrs->item(i);

        if (!attribute) {
            break;
        }

        const XMLCh* attName = attribute->getNodeName();

        // starts with 'xmlns:'
        if (XMLString::startsWith(attName, fgXMLNS_Str)) {

            int offsetIndex = XMLString::indexOf(attName, chColon);
            const XMLCh* attValue = attribute->getNodeValue();

            fNamespaceScope->addPrefix(attName + offsetIndex + 1, fURIStringPool->addOrFind(attValue));
        }
        else if (XMLString::equals(attName, XMLUni::fgXMLNSString)) { // == 'xmlns'

            const XMLCh* attValue = attribute->getNodeValue();
            fNamespaceScope->addPrefix(XMLUni::fgZeroLenString, fURIStringPool->addOrFind(attValue));
            seenXMLNS = true;
        }
    } // end for

    if (!seenXMLNS && (!fTargetNSURIString || !*fTargetNSURIString)) {
        fNamespaceScope->addPrefix(XMLUni::fgZeroLenString, fEmptyNamespaceURI);
    }

    // Add mapping for xml prefix
    fNamespaceScope->addPrefix(XMLUni::fgXMLString, fURIStringPool->addOrFind(XMLUni::fgXMLURIName));
}

void TraverseSchema::processChildren(const DOMElement* const root) {

    // process <redefine>, <include> and <import> info items.
    DOMElement* child = XUtil::getFirstChildElement(root);

    for (; child != 0; child = XUtil::getNextSiblingElement(child)) {

        const XMLCh* name = child->getLocalName();

        if (XMLString::equals(name, SchemaSymbols::fgELT_ANNOTATION)) {
            traverseAnnotationDecl(child, true);
        }
        else if (XMLString::equals(name, SchemaSymbols::fgELT_INCLUDE)) {
            traverseInclude(child);
        }
        else if (XMLString::equals(name, SchemaSymbols::fgELT_IMPORT)) {
            traverseImport(child);
        }
        else if (XMLString::equals(name, SchemaSymbols::fgELT_REDEFINE)) {
            traverseRedefine(child);
        }
        else
            break;
    }

    // child refers to the first info item which is not <annotation> or
    // one of the schema inclusion/importation declarations.
    for (; child != 0; child = XUtil::getNextSiblingElement(child)) {

        const XMLCh* name = child->getLocalName();
        const XMLCh* typeName = getElementAttValue(child, SchemaSymbols::fgATT_NAME);
        int fullNameId = 0;

        if (typeName) {

            fBuffer.set(fTargetNSURIString);
            fBuffer.append(chComma);
            fBuffer.append(typeName);
            fullNameId = fStringPool->addOrFind(fBuffer.getRawBuffer());
        }

        if (XMLString::equals(name, SchemaSymbols::fgELT_ANNOTATION)) {
            traverseAnnotationDecl(child, true);
        }
        else if (XMLString::equals(name, SchemaSymbols::fgELT_SIMPLETYPE)) {

            if (typeName && *typeName) {
                if (fGlobalDeclarations[ENUM_ELT_SIMPLETYPE]->containsElement(fullNameId)
                    || fGlobalDeclarations[ENUM_ELT_COMPLEXTYPE]->containsElement(fullNameId)) {

                    reportSchemaError(child, XMLUni::fgXMLErrDomain, XMLErrs::DuplicateGlobalType,
                                      SchemaSymbols::fgELT_SIMPLETYPE, typeName, SchemaSymbols::fgELT_COMPLEXTYPE);
                    continue;
                }
                else {
                    fGlobalDeclarations[ENUM_ELT_SIMPLETYPE]->addElement(fullNameId);
                }
            }

            traverseSimpleTypeDecl(child);
        }
        else if (XMLString::equals(name, SchemaSymbols::fgELT_COMPLEXTYPE)) {

            if (typeName && *typeName) {
                if (fGlobalDeclarations[ENUM_ELT_SIMPLETYPE]->containsElement(fullNameId)
                    || fGlobalDeclarations[ENUM_ELT_COMPLEXTYPE]->containsElement(fullNameId)) {

                    reportSchemaError(child, XMLUni::fgXMLErrDomain, XMLErrs::DuplicateGlobalType,
                                      SchemaSymbols::fgELT_COMPLEXTYPE, typeName, SchemaSymbols::fgELT_SIMPLETYPE);
                    continue;
                }
                else {
                    fGlobalDeclarations[ENUM_ELT_COMPLEXTYPE]->addElement(fullNameId);
                }
            }

            traverseComplexTypeDecl(child);
        }
        else if (XMLString::equals(name, SchemaSymbols::fgELT_ELEMENT)) {

            if (typeName && *typeName) {
                if (fGlobalDeclarations[ENUM_ELT_ELEMENT]->containsElement(fullNameId)) {

                    reportSchemaError(child, XMLUni::fgXMLErrDomain, XMLErrs::DuplicateGlobalDeclaration,
                                      SchemaSymbols::fgELT_ELEMENT, typeName);
                    continue;
                }
                else {
                    fGlobalDeclarations[ENUM_ELT_ELEMENT]->addElement(fullNameId);
                }
            }

            QName* elmQName = traverseElementDecl(child, true);
            delete elmQName;
        }
        else if (XMLString::equals(name, SchemaSymbols::fgELT_ATTRIBUTEGROUP)) {

            if (typeName && *typeName) {
                if (fGlobalDeclarations[ENUM_ELT_ATTRIBUTEGROUP]->containsElement(fullNameId)) {

                    reportSchemaError(child, XMLUni::fgXMLErrDomain, XMLErrs::DuplicateGlobalDeclaration,
                                      SchemaSymbols::fgELT_ATTRIBUTEGROUP, typeName);
                    continue;
                }
                else {
                    fGlobalDeclarations[ENUM_ELT_ATTRIBUTEGROUP]->addElement(fullNameId);
                }
            }

            if (!typeName || !fAttGroupRegistry->containsKey(typeName)) {
                traverseAttributeGroupDecl(child, 0, true);
            }
        }
        else if (XMLString::equals(name, SchemaSymbols::fgELT_ATTRIBUTE)) {

            if (typeName && *typeName) {
                if (fGlobalDeclarations[ENUM_ELT_ATTRIBUTE]->containsElement(fullNameId)) {

                    reportSchemaError(child, XMLUni::fgXMLErrDomain, XMLErrs::DuplicateAttribute, typeName);
                    continue;
                }
                else {
                    fGlobalDeclarations[ENUM_ELT_ATTRIBUTE]->addElement(fullNameId);
                }
            }

            if (!typeName || !fAttributeDeclRegistry->containsKey(typeName)) {
                traverseAttributeDecl( child, 0, true);
            }
        }
        else if (XMLString::equals(name, SchemaSymbols::fgELT_GROUP)) {

            if (typeName && *typeName) {
                if (fGlobalDeclarations[ENUM_ELT_GROUP]->containsElement(fullNameId)) {

                    reportSchemaError(child, XMLUni::fgXMLErrDomain, XMLErrs::DuplicateGlobalDeclaration,
                                      SchemaSymbols::fgELT_GROUP, typeName);
                    continue;
                }
                else {
                    fGlobalDeclarations[ENUM_ELT_GROUP]->addElement(fullNameId);
                }
            }

            if (!typeName || !fGroupRegistry->containsKey(fBuffer.getRawBuffer())) {
                traverseGroupDecl(child);
            }
        }
        else if (XMLString::equals(name, SchemaSymbols::fgELT_NOTATION)) {
            traverseNotationDecl(child);
        }
        else {
            reportSchemaError(child, XMLUni::fgXMLErrDomain, XMLErrs::SchemaElementContentError);
        }
    } // for each child node

    // Handle recursing elements - if any
    ValueVectorOf<const DOMElement*>* recursingAnonTypes = fSchemaInfo->getRecursingAnonTypes();

    if (recursingAnonTypes) {

        ValueVectorOf<const XMLCh*>* recursingTypeNames = fSchemaInfo->getRecursingTypeNames();
        unsigned int recurseSize = recursingAnonTypes->size();

        for (unsigned int i=0; i < recurseSize; i++) {
            traverseComplexTypeDecl(recursingAnonTypes->elementAt(i), false,
                                    recursingTypeNames->elementAt(i));
        }

        recursingAnonTypes->removeAllElements();
        recursingTypeNames->removeAllElements();
    }
}

void TraverseSchema::preprocessChildren(const DOMElement* const root) {

    // process <redefine>, <include> and <import> info items.
    DOMElement* child = XUtil::getFirstChildElement(root);

    for (; child != 0; child = XUtil::getNextSiblingElement(child)) {

        const XMLCh* name = child->getLocalName();

        if (XMLString::equals(name, SchemaSymbols::fgELT_ANNOTATION)) {
            continue;
        }
        else if (XMLString::equals(name, SchemaSymbols::fgELT_INCLUDE)) {
            preprocessInclude(child);
        }
        else if (XMLString::equals(name, SchemaSymbols::fgELT_IMPORT)) {
            preprocessImport(child);
        }
        else if (XMLString::equals(name, SchemaSymbols::fgELT_REDEFINE)) {
            preprocessRedefine(child);
        }
        else
            break;
    }
}


DOMElement* TraverseSchema::checkContent(const DOMElement* const rootElem,
                                           DOMElement* const contentElem,
                                           const bool isEmpty) {

    DOMElement* content = contentElem;
    const XMLCh* name = getElementAttValue(rootElem,SchemaSymbols::fgATT_NAME);

    if (!content) {

       if (!isEmpty) {
           reportSchemaError(rootElem, XMLUni::fgXMLErrDomain, XMLErrs::ContentError, name);
       }

       return 0;
    }

    if (XMLString::equals(content->getLocalName(), SchemaSymbols::fgELT_ANNOTATION)) {

        traverseAnnotationDecl(content);
        content = XUtil::getNextSiblingElement(content);

        if (!content) { // must be followed by content

            if (!isEmpty) {
                reportSchemaError(contentElem, XMLUni::fgXMLErrDomain, XMLErrs::ContentError, name);
            }

            return 0;
        }

        if (XMLString::equals(content->getLocalName(), SchemaSymbols::fgELT_ANNOTATION)) {

            reportSchemaError(content, XMLUni::fgXMLErrDomain, XMLErrs::AnnotationError, name);
            return 0;
        }
    }

    return content;
}


DatatypeValidator*
TraverseSchema::getDatatypeValidator(const XMLCh* const uriStr,
                                     const XMLCh* const localPartStr) {

    DatatypeValidator* dv = 0;

    if (XMLString::equals(uriStr, SchemaSymbols::fgURI_SCHEMAFORSCHEMA)) {
        dv = fDatatypeRegistry->getDatatypeValidator(localPartStr);
    }
    else {

        fBuffer.set(uriStr);
        fBuffer.append(chComma);
        fBuffer.append(localPartStr);

        if ((uriStr && *uriStr) && !XMLString::equals(uriStr, fTargetNSURIString)) {

            Grammar* grammar = fGrammarResolver->getGrammar(uriStr);

            if (grammar && grammar->getGrammarType() == Grammar::SchemaGrammarType) {
                dv = ((SchemaGrammar*) grammar)->getDatatypeRegistry()->getDatatypeValidator(fBuffer.getRawBuffer());
            }
        }
        else {
            dv = fDatatypeRegistry->getDatatypeValidator(fBuffer.getRawBuffer());
        }
    }

    return dv;
}


DatatypeValidator*
TraverseSchema::checkForSimpleTypeValidator(const DOMElement* const content,
                                            int baseRefContext) {

    DatatypeValidator* baseValidator = traverseSimpleTypeDecl(content, false, baseRefContext);

    if (!baseValidator) {

        const XMLCh* name = getElementAttValue(content,SchemaSymbols::fgATT_NAME);
        reportSchemaError(content, XMLUni::fgXMLErrDomain, XMLErrs::UnknownSimpleType, name);
    }

    return baseValidator;
}

ComplexTypeInfo*
TraverseSchema::checkForComplexTypeInfo(const DOMElement* const content) {

    int typeNameIndex = traverseComplexTypeDecl(content, false);
    ComplexTypeInfo* baseTypeInfo = 0;

    if (typeNameIndex != -1) {
        baseTypeInfo = fComplexTypeRegistry->get(fStringPool->getValueForId(typeNameIndex));
    }

    if (typeNameIndex == -1 || baseTypeInfo == 0) {

        const XMLCh* name = getElementAttValue(content,SchemaSymbols::fgATT_NAME);
        reportSchemaError(content, XMLUni::fgXMLErrDomain, XMLErrs::UnknownComplexType, name);
    }

    return baseTypeInfo;
}

DatatypeValidator*
TraverseSchema::findDTValidator(const DOMElement* const elem,
                                const XMLCh* const derivedTypeName,
                                const XMLCh* const baseTypeName,
                                const int baseRefContext) {

    const XMLCh*       prefix = getPrefix(baseTypeName);
    const XMLCh*       localPart = getLocalPart(baseTypeName);
    const XMLCh*       uri = resolvePrefixToURI(elem, prefix);
    DatatypeValidator* baseValidator = getDatatypeValidator(uri, localPart);

    if (baseValidator == 0) {

        SchemaInfo* saveInfo = fSchemaInfo;
        DOMElement* baseTypeNode = fSchemaInfo->getTopLevelComponent(SchemaInfo::C_SimpleType,
            SchemaSymbols::fgELT_SIMPLETYPE, localPart, &fSchemaInfo);

        if (baseTypeNode != 0) {

            baseValidator = traverseSimpleTypeDecl(baseTypeNode);

            // restore schema information, if necessary
            fSchemaInfo = saveInfo;
        }
    }

    if (baseValidator == 0) {
        reportSchemaError(elem, XMLUni::fgValidityDomain, XMLValid::UnknownBaseDatatype, baseTypeName, derivedTypeName);
    }
    else if ((baseValidator->getFinalSet() & baseRefContext) != 0) {

        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::DisallowedBaseDerivation, baseTypeName);
        return 0;
    }

    return baseValidator;
}


const XMLCh* TraverseSchema::resolvePrefixToURI(const DOMElement* const elem,
                                                const XMLCh* const prefix) {

    int nameSpaceIndex = fNamespaceScope->getNamespaceForPrefix(prefix, fSchemaInfo->getNamespaceScopeLevel());
    const XMLCh* uriStr = fURIStringPool->getValueForId(nameSpaceIndex);

    if ((!uriStr || !*uriStr) && (prefix && *prefix)) {
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::UnresolvedPrefix, prefix);
        return XMLUni::fgZeroLenString;
    }

    return uriStr;
}

const XMLCh* TraverseSchema::resolvePrefixToURI(const DOMElement* const elem,
                                                const XMLCh* const prefix,
                                                const unsigned int namespaceDepth) {

    int nameSpaceIndex = fNamespaceScope->getNamespaceForPrefix(prefix, namespaceDepth);
    const XMLCh* uriStr = fURIStringPool->getValueForId(nameSpaceIndex);

    if ((!uriStr || !*uriStr) && (prefix && *prefix)) {
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::UnresolvedPrefix, prefix);
        return XMLUni::fgZeroLenString;
    }

    return uriStr;
}


QName* TraverseSchema::processElementDeclRef(const DOMElement* const elem,
                                             const XMLCh* const refName) {

    DOMElement* content = checkContent(elem, XUtil::getFirstChildElement(elem), true);

    if (content != 0) {
        reportSchemaError(elem, XMLUni::fgValidityDomain, XMLValid::NoContentForRef, SchemaSymbols::fgELT_ELEMENT);
    }

    const XMLCh* prefix = getPrefix(refName);
    const XMLCh* localPart = getLocalPart(refName);
    const XMLCh* uriStr = resolvePrefixToURI(elem, prefix);
    QName*       eltName = new (fMemoryManager) QName(prefix , localPart, uriStr != 0
                                       ? fURIStringPool->addOrFind(uriStr)
                                       : fEmptyNamespaceURI);

    //if from another schema, just return the element QName
    if (!XMLString::equals(uriStr, fTargetNSURIString)) {
        return eltName;
    }

    unsigned int uriID = eltName->getURI();
    SchemaElementDecl* refElemDecl = (SchemaElementDecl*)
        fSchemaGrammar->getElemDecl(uriID, localPart, 0, Grammar::TOP_LEVEL_SCOPE);

    //if not found, traverse the top level element that is referenced
    if (!refElemDecl) {

        SchemaInfo* saveInfo = fSchemaInfo;
        DOMElement* targetElem = fSchemaInfo->getTopLevelComponent(SchemaInfo::C_Element,
            SchemaSymbols::fgELT_ELEMENT, localPart, &fSchemaInfo);

        if (targetElem == 0)  {

            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::RefElementNotFound, localPart);
            // REVISIT do we return 0 or what? for now we will return QName created
            return eltName;
        }
        else {

            delete eltName;
            eltName = traverseElementDecl(targetElem, true);
            refElemDecl = (SchemaElementDecl*)
                fSchemaGrammar->getElemDecl(uriID, localPart, 0, Grammar::TOP_LEVEL_SCOPE);

            // restore schema information
            fSchemaInfo = saveInfo;
        }
    }

    if (refElemDecl) {

        if (fCurrentComplexType)
            fCurrentComplexType->addElement(refElemDecl);

        if (fCurrentGroupInfo)
            fCurrentGroupInfo->addElement(refElemDecl);
    }

    return eltName;
}

int TraverseSchema::parseBlockSet(const DOMElement* const elem,
                                  const int blockType, const bool isRoot) {

    const XMLCh* blockVal = (isRoot) ? getElementAttValue(elem, SchemaSymbols::fgATT_BLOCKDEFAULT)
                                     : getElementAttValue(elem, SchemaSymbols::fgATT_BLOCK);

    if (!blockVal || !*blockVal) {
        return fSchemaInfo->getBlockDefault();
    }

    int blockSet = 0;

    if (XMLString::equals(blockVal, SchemaSymbols::fgATTVAL_POUNDALL)) {

        blockSet = SchemaSymbols::XSD_EXTENSION + SchemaSymbols::XSD_RESTRICTION + SchemaSymbols::XSD_SUBSTITUTION;
        return blockSet;
    }

    XMLStringTokenizer tokenizer(blockVal);

    while (tokenizer.hasMoreTokens()) {

        XMLCh* token = tokenizer.nextToken();

        if (XMLString::equals(token, SchemaSymbols::fgATTVAL_SUBSTITUTION)
			&& blockType == ES_Block) {

            if ((blockSet & SchemaSymbols::XSD_SUBSTITUTION) == 0 ) {
                blockSet += SchemaSymbols::XSD_SUBSTITUTION;
            }
            else {
                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::SubstitutionRepeated);
            }
        }
        else if (XMLString::equals(token, SchemaSymbols::fgATTVAL_EXTENSION)) {

            if ((blockSet & SchemaSymbols::XSD_EXTENSION) == 0) {
                blockSet += SchemaSymbols::XSD_EXTENSION;
            }
            else {
                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::ExtensionRepeated);
            }
        }
        else if (XMLString::equals(token, SchemaSymbols::fgATTVAL_RESTRICTION)) {

            if ((blockSet & SchemaSymbols::XSD_RESTRICTION) == 0 ) {
                blockSet += SchemaSymbols::XSD_RESTRICTION;
            }
            else {
                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::RestrictionRepeated);
            }
        }
        else {
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidBlockValue, blockVal);
        }
    } //end while

    return (blockSet == 0 ? fSchemaInfo->getBlockDefault() : blockSet);
}

int TraverseSchema::parseFinalSet(const DOMElement* const elem,
                                  const int finalType, const bool isRoot) {

    const XMLCh* finalVal = (isRoot) ? getElementAttValue(elem, SchemaSymbols::fgATT_FINALDEFAULT)
                                     : getElementAttValue(elem, SchemaSymbols::fgATT_FINAL);

    if (!finalVal || !*finalVal) {
        return fSchemaInfo->getFinalDefault();
    }

    int finalSet = 0;

    if (XMLString::equals(finalVal, SchemaSymbols::fgATTVAL_POUNDALL)) {

        finalSet = SchemaSymbols::XSD_RESTRICTION + SchemaSymbols::XSD_LIST +
                   SchemaSymbols::XSD_UNION + SchemaSymbols::XSD_EXTENSION;
        return finalSet;
    }

    XMLStringTokenizer tokenizer(finalVal);

    while (tokenizer.hasMoreTokens()) {

        XMLCh* token = tokenizer.nextToken();

        if (XMLString::equals(token, SchemaSymbols::fgELT_UNION)
            && finalType == S_Final) {

            if ((finalSet & SchemaSymbols::XSD_UNION) == 0) {
                finalSet += SchemaSymbols::XSD_UNION;
            }
            else {
                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::UnionRepeated);
            }
        }
        else if (XMLString::equals(token, SchemaSymbols::fgATTVAL_EXTENSION)
                 && finalType != S_Final) {

            if ((finalSet & SchemaSymbols::XSD_EXTENSION) == 0) {
                finalSet += SchemaSymbols::XSD_EXTENSION;
            }
            else {
                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::ExtensionRepeated);
            }
        }
        else if (XMLString::equals(token, SchemaSymbols::fgELT_LIST)
                 && finalType == S_Final) {

            if ((finalSet & SchemaSymbols::XSD_LIST) == 0 ) {
                finalSet += SchemaSymbols::XSD_LIST;
            }
            else {
                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::ListRepeated);
            }
        }
        else if (XMLString::equals(token, SchemaSymbols::fgATTVAL_RESTRICTION)) {

            if ((finalSet & SchemaSymbols::XSD_RESTRICTION) == 0 ) {
                finalSet += SchemaSymbols::XSD_RESTRICTION;
            }
            else {
                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::RestrictionRepeated);
            }
        }
        else {
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidFinalValue, finalVal);
        }
    } //end while

    return (finalSet == 0 ? fSchemaInfo->getFinalDefault() : finalSet);
}


const DOMElement*
TraverseSchema::checkIdentityConstraintContent(const DOMElement* const content) {

    const DOMElement* child = content;

    if (child != 0) {

        do {

            if (!isIdentityConstraintName(child->getLocalName())) {
                break;
            }

            child = XUtil::getNextSiblingElement(child);

        } while (child != 0);
    }

    return child;
}

bool TraverseSchema::isIdentityConstraintName(const XMLCh* const name) {

    return (XMLString::equals(name, SchemaSymbols::fgELT_KEY)
            || XMLString::equals(name, SchemaSymbols::fgELT_KEYREF)
            || XMLString::equals(name, SchemaSymbols::fgELT_UNIQUE));
}

const XMLCh*
TraverseSchema::checkTypeFromAnotherSchema(const DOMElement* const elem,
										   const XMLCh* const typeStr) {

    const XMLCh* prefix = getPrefix(typeStr);
    const XMLCh* typeURI = resolvePrefixToURI(elem, prefix);

    if (!XMLString::equals(typeURI, fTargetNSURIString)
        && !XMLString::equals(typeURI, SchemaSymbols::fgURI_SCHEMAFORSCHEMA)
        && (typeURI && *typeURI)) {
        return typeURI;
    }

    return 0;
}

DatatypeValidator*
TraverseSchema::getElementTypeValidator(const DOMElement* const elem,
                                        const XMLCh* const typeStr,
                                        bool& noErrorDetected,
                                        const XMLCh* const otherSchemaURI)
{
    const XMLCh*       localPart = getLocalPart(typeStr);
    const XMLCh*       typeURI = otherSchemaURI;
    DatatypeValidator* dv = 0;
    SchemaInfo::ListType infoType = SchemaInfo::INCLUDE;
    SchemaInfo*          saveInfo = fSchemaInfo;
    int                  saveScope = fCurrentScope;

    if (otherSchemaURI != 0) {

        // Make sure that we have an explicit import statement.
        // Clause 4 of Schema Representation Constraint:
        // http://www.w3.org/TR/xmlschema-1/#src-resolve
        unsigned int uriId = fURIStringPool->addOrFind(otherSchemaURI);

        if (!fSchemaInfo->isImportingNS(uriId)) {

            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidNSReference, otherSchemaURI);
            return 0;
        }

        dv = getDatatypeValidator(typeURI, localPart);

        if (dv) {
            return dv;
        }

        SchemaInfo* impInfo = fSchemaInfo->getImportInfo(uriId);

        if (!impInfo || impInfo->getProcessed()) {

            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::TypeNotFound, typeURI, localPart);
            return 0;
        }

        infoType = SchemaInfo::IMPORT;
        restoreSchemaInfo(impInfo, infoType);
    }
    else {
        const XMLCh* prefix = getPrefix(typeStr);

        typeURI = resolvePrefixToURI(elem, prefix);
        dv = getDatatypeValidator(typeURI, localPart);
    }

    if (!dv) {

        if (!XMLString::equals(typeURI, SchemaSymbols::fgURI_SCHEMAFORSCHEMA)
            || XMLString::equals(fTargetNSURIString, SchemaSymbols::fgURI_SCHEMAFORSCHEMA)) {

            DOMElement* typeElem = fSchemaInfo->getTopLevelComponent(SchemaInfo::C_SimpleType,
                SchemaSymbols::fgELT_SIMPLETYPE, localPart, &fSchemaInfo);

            if (typeElem) {
                dv = traverseSimpleTypeDecl(typeElem);
            }
        }

        // restore schema information, if necessary
        if (saveInfo != fSchemaInfo) {
            restoreSchemaInfo(saveInfo, infoType, saveScope);
        }

        if (!dv) {

            noErrorDetected = false;
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::TypeNotFound, typeURI, localPart);
        }
    }

    return dv;
}


DatatypeValidator*
TraverseSchema::getAttrDatatypeValidatorNS(const DOMElement* const elem,
                                           const XMLCh* localPart,
                                           const XMLCh* typeURI)
{
    DatatypeValidator*   dv = getDatatypeValidator(typeURI, localPart);
    SchemaInfo::ListType infoType = SchemaInfo::INCLUDE;
    SchemaInfo*          saveInfo = fSchemaInfo;
    int                  saveScope = fCurrentScope;

    if (!XMLString::equals(typeURI, fTargetNSURIString)
        && (typeURI && *typeURI)) {

        // Make sure that we have an explicit import statement.
        // Clause 4 of Schema Representation Constraint:
        // http://www.w3.org/TR/xmlschema-1/#src-resolve
        unsigned int uriId = fURIStringPool->addOrFind(typeURI);

        if (!fSchemaInfo->isImportingNS(uriId)) {

            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidNSReference, typeURI);
            return 0;
        }

        if (!dv) {
            SchemaInfo* impInfo = fSchemaInfo->getImportInfo(uriId);

            if (!impInfo || impInfo->getProcessed())
                return 0;

            infoType = SchemaInfo::IMPORT;
            restoreSchemaInfo(impInfo, infoType);
        }
    }

    if (!dv) {

        DOMElement* typeElem = fSchemaInfo->getTopLevelComponent
        (
            SchemaInfo::C_SimpleType
            , SchemaSymbols::fgELT_SIMPLETYPE
            , localPart
            , &fSchemaInfo
        );

        if (typeElem)
            dv = traverseSimpleTypeDecl(typeElem);

        // restore schema information, if necessary
        if (saveInfo != fSchemaInfo) {
            restoreSchemaInfo(saveInfo, infoType, saveScope);
        }
    }

    return dv;
}

ComplexTypeInfo*
TraverseSchema::getElementComplexTypeInfo(const DOMElement* const elem,
                                          const XMLCh* const typeStr,
                                          bool& noErrorDetected,
                                          const XMLCh* const otherSchemaURI)
{
    const XMLCh*         localPart = getLocalPart(typeStr);
    const XMLCh*         prefix = getPrefix(typeStr);
    const XMLCh*         typeURI = (otherSchemaURI) ? otherSchemaURI : resolvePrefixToURI(elem, prefix);
    ComplexTypeInfo*     typeInfo = 0;
    SchemaInfo*          saveInfo = fSchemaInfo;
    SchemaInfo::ListType infoType = SchemaInfo::INCLUDE;
    int                  saveScope = fCurrentScope;

    fBuffer.set(typeURI);
    fBuffer.append(chComma);
    fBuffer.append(localPart);

    if (otherSchemaURI != 0) {

        // Make sure that we have an explicit import statement.
        // Clause 4 of Schema Representation Constraint:
        // http://www.w3.org/TR/xmlschema-1/#src-resolve
        unsigned int uriId = fURIStringPool->addOrFind(typeURI);

        if (!fSchemaInfo->isImportingNS(uriId))
            return 0;

        Grammar* aGrammar = fGrammarResolver->getGrammar(typeURI);

        if (!aGrammar || aGrammar->getGrammarType() != Grammar::SchemaGrammarType) {
            return 0;
        }

        typeInfo = ((SchemaGrammar*)aGrammar)->getComplexTypeRegistry()->get(fBuffer.getRawBuffer());

        if (typeInfo) {
            return typeInfo;
        }

        SchemaInfo* impInfo = fSchemaInfo->getImportInfo(uriId);

        if (!impInfo || impInfo->getProcessed()) {
            return 0;
        }

        infoType = SchemaInfo::IMPORT;
        restoreSchemaInfo(impInfo, infoType);
    }
    else {
        typeInfo = fComplexTypeRegistry->get(fBuffer.getRawBuffer());
    }

    if (!typeInfo) {

        if (!XMLString::equals(typeURI, SchemaSymbols::fgURI_SCHEMAFORSCHEMA) ||
            XMLString::equals(fTargetNSURIString, SchemaSymbols::fgURI_SCHEMAFORSCHEMA)) {

            DOMElement* typeNode = fSchemaInfo->getTopLevelComponent(SchemaInfo::C_ComplexType,
                SchemaSymbols::fgELT_COMPLEXTYPE, localPart, &fSchemaInfo);

            if (typeNode) {

                int typeIndex = traverseComplexTypeDecl(typeNode);
                typeInfo =  fComplexTypeRegistry->get(fStringPool->getValueForId(typeIndex));
            }
        }
    }

    // restore schema information
    restoreSchemaInfo(saveInfo, infoType, saveScope);

    return typeInfo;
}


SchemaElementDecl*
TraverseSchema::getSubstituteGroupElemDecl(const DOMElement* const elem,
                                           const XMLCh* const name,
                                           bool& noErrorDetected) {

    const XMLCh*         nameURI =  resolvePrefixToURI(elem, getPrefix(name));
    const XMLCh*         localPart = getLocalPart(name);
    SchemaElementDecl*   elemDecl = 0;
    SchemaInfo*          saveInfo = fSchemaInfo;
    SchemaInfo::ListType infoType = SchemaInfo::INCLUDE;
    int                  saveScope = fCurrentScope;
    unsigned int         uriId = fURIStringPool->addOrFind(nameURI);
    bool                 wasAdded = false;

    if (!XMLString::equals(nameURI, fTargetNSURIString)) {

        // Make sure that we have an explicit import statement.
        // Clause 4 of Schema Representation Constraint:
        // http://www.w3.org/TR/xmlschema-1/#src-resolve
        if (!fSchemaInfo->isImportingNS(uriId)) {

            noErrorDetected = false;
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidNSReference, nameURI);
            return (SchemaElementDecl*) fSchemaGrammar->findOrAddElemDecl(uriId, localPart, XMLUni::fgZeroLenString,
                                                     0, Grammar::TOP_LEVEL_SCOPE, wasAdded);
        }

        Grammar* grammar = fGrammarResolver->getGrammar(nameURI);

        if (grammar && grammar->getGrammarType() == Grammar::SchemaGrammarType) {
            elemDecl = (SchemaElementDecl*)
                grammar->getElemDecl(uriId, localPart, 0, Grammar::TOP_LEVEL_SCOPE);
        }
        else {

            noErrorDetected = false;
            reportSchemaError(elem, XMLUni::fgValidityDomain, XMLValid::GrammarNotFound, nameURI);
            return (SchemaElementDecl*) fSchemaGrammar->findOrAddElemDecl(uriId, localPart, XMLUni::fgZeroLenString,
                                                     0, Grammar::TOP_LEVEL_SCOPE, wasAdded);
        }

        if (!elemDecl) {

            SchemaInfo* impInfo = fSchemaInfo->getImportInfo(uriId);

            if (!impInfo || impInfo->getProcessed()) {

                noErrorDetected = false;
                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::TypeNotFound, nameURI, localPart);
                return (SchemaElementDecl*) grammar->findOrAddElemDecl(uriId, localPart, XMLUni::fgZeroLenString,
                                                  0, Grammar::TOP_LEVEL_SCOPE, wasAdded);
            }

            infoType = SchemaInfo::IMPORT;
            restoreSchemaInfo(impInfo, infoType);		
        }
    }
    else {
        elemDecl = (SchemaElementDecl*)
            fSchemaGrammar->getElemDecl(fTargetNSURI, localPart, 0, Grammar::TOP_LEVEL_SCOPE);
    }

    if (!elemDecl) {

        DOMElement* subsGroupElem = fSchemaInfo->getTopLevelComponent(SchemaInfo::C_Element,
            SchemaSymbols::fgELT_ELEMENT,localPart, &fSchemaInfo);

        if (subsGroupElem != 0) {

            QName* subsGroupQName = traverseElementDecl(subsGroupElem, true);
            Janitor<QName> janQName(subsGroupQName);

            if (subsGroupQName) {
                elemDecl = (SchemaElementDecl*) fSchemaGrammar->getElemDecl(fTargetNSURI, localPart,0, Grammar::TOP_LEVEL_SCOPE);
            }
        }

        if (!elemDecl) {

            noErrorDetected = false;
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::TypeNotFound, nameURI, localPart);
            elemDecl = (SchemaElementDecl*) fSchemaGrammar->findOrAddElemDecl(uriId, localPart, XMLUni::fgZeroLenString,
                                                         0, Grammar::TOP_LEVEL_SCOPE, wasAdded);
        }
    }

    // restore schema information, if necessary
    if (saveInfo != fSchemaInfo) {
        restoreSchemaInfo(saveInfo, infoType, saveScope);
    }

    return elemDecl;
}

bool
TraverseSchema::isSubstitutionGroupValid(const DOMElement* const elem,
                                         const SchemaElementDecl* const subsElemDecl,
                                         const ComplexTypeInfo* const typeInfo,
                                         const DatatypeValidator* const validator,
                                         const XMLCh* const elemName,
                                         const bool toEmit) {

    // here we must do two things:
    // 1.  Make sure there actually *is* a relation between the types of
    // the element being nominated and the element doing the nominating;
    // (see PR 3.3.6 point #3 in the first tableau, for instance; this
    // and the corresponding tableaux from 3.4.6 and 3.14.6 rule out the nominated
    // element having an anonymous type declaration.
    // 2.  Make sure the nominated element allows itself to be nominated by
    // an element with the given type-relation.
    // Note:  we assume that (complex|simple)Type processing checks
    // whether the type in question allows itself to
    // be modified as this element desires.

    // if substitution element has any as content model type, return true
    bool subsRestricted = false;
    if (subsElemDecl->getModelType() == SchemaElementDecl::Any) {

        if ((subsElemDecl->getFinalSet() & SchemaSymbols::XSD_RESTRICTION) == 0
            || (typeInfo == 0 && validator == 0))
            return true;
        else
            subsRestricted = true;
    }
    // Check for type relationship;
    // that is, make sure that the type we're deriving has some relatoinship
    // to substitutionGroupElt's type.
    else if (typeInfo) { // do complexType case ...need testing

        int derivationMethod = typeInfo->getDerivedBy();

        if (typeInfo->getContentType() == SchemaElementDecl::Simple) {  // take care of complexType based on simpleType case...

            DatatypeValidator* elemDV = typeInfo->getDatatypeValidator();
            DatatypeValidator* subsValidator = subsElemDecl->getDatatypeValidator();

            if (elemDV == subsValidator) {
                return true;
            }
            else if (subsValidator && subsValidator->isSubstitutableBy(elemDV)) {
                if ((subsElemDecl->getFinalSet() & derivationMethod) == 0) {
                    return true;
                }
                else {
                    subsRestricted = true;
                }
            }
        }
        else { // complex content

            ComplexTypeInfo* subsTypeInfo = subsElemDecl->getComplexTypeInfo();

            if (subsTypeInfo == typeInfo)
                return true;

            const ComplexTypeInfo* elemTypeInfo = typeInfo;

            for (; elemTypeInfo && elemTypeInfo != subsTypeInfo;
                elemTypeInfo = elemTypeInfo->getBaseComplexTypeInfo()) {
            }

            if (elemTypeInfo) {
                if ((subsElemDecl->getFinalSet() & derivationMethod) == 0) {
                    return true;
                }
                else {
                    subsRestricted = true;
                }
            }
        }
    }
    else if (validator) { // do simpleType case...

        // first, check for type relation.
        DatatypeValidator* subsValidator = subsElemDecl->getDatatypeValidator();

        if (subsValidator == validator) {
            return true;
        }
        else if (subsValidator && subsValidator->isSubstitutableBy(validator)
            && ((subsElemDecl->getFinalSet() & SchemaSymbols::XSD_RESTRICTION) == 0)) {
                return true;
        }
    }

    if (toEmit) {
        if (subsRestricted) {
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidSubstitutionGroupElement,
                              elemName, subsElemDecl->getBaseName());
        }
        else {
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::SubstitutionGroupTypeMismatch, elemName);
        }
    }

    return false;
}


SchemaElementDecl*
TraverseSchema::createSchemaElementDecl(const DOMElement* const elem,
                                        const bool topLevel,
                                        const unsigned short elemType,
                                        bool& isDuplicate,
                                        const bool isFixedVal)
{
    const XMLCh* name = getElementAttValue(elem, SchemaSymbols::fgATT_NAME);
    const XMLCh* elemForm = getElementAttValue(elem, SchemaSymbols::fgATT_FORM);
    int enclosingScope = fCurrentScope;
    int uriIndex = fEmptyNamespaceURI;

    //refer to 4.3.2 in "XML Schema Part 1: Structures"
    if (topLevel) {

        uriIndex = fTargetNSURI;
        enclosingScope = Grammar::TOP_LEVEL_SCOPE;
    }
    else if (((!elemForm || !*elemForm) &&
             (fSchemaInfo->getElemAttrDefaultQualified() & Elem_Def_Qualified))
             || XMLString::equals(elemForm,SchemaSymbols::fgATTVAL_QUALIFIED)) {

        uriIndex = fTargetNSURI;
    }

    // Check for duplicate elements
    SchemaElementDecl* other = (SchemaElementDecl*)
        fSchemaGrammar->getElemDecl(uriIndex, name, 0, enclosingScope);

    if (other != 0) {

        isDuplicate = true;
        return other;
    }

    int blockSet = parseBlockSet(elem, ES_Block);
    int finalSet = parseFinalSet(elem, EC_Final);
    int elementMiscFlags = 0;
    const XMLCh* nillable = getElementAttValue(elem, SchemaSymbols::fgATT_NILLABLE);
    const XMLCh* abstract = getElementAttValue(elem, SchemaSymbols::fgATT_ABSTRACT);

    if (nillable && *nillable) {

        if (XMLString::equals(nillable, SchemaSymbols::fgATTVAL_TRUE)
            || XMLString::equals(nillable, fgValueOne)) {
            elementMiscFlags += SchemaSymbols::XSD_NILLABLE;
        }
    }

    if (abstract && *abstract) {

        if (XMLString::equals(abstract, SchemaSymbols::fgATTVAL_TRUE)
            || XMLString::equals(abstract, fgValueOne)) {
            elementMiscFlags += SchemaSymbols::XSD_ABSTRACT;
        }
    }

    if (isFixedVal) {
        elementMiscFlags += SchemaSymbols::XSD_FIXED;
    }

    const XMLCh* prefix = getPrefix(name);
    SchemaElementDecl* elemDecl = new (fMemoryManager) SchemaElementDecl
    (
        prefix
        , name
        , uriIndex
        , (SchemaElementDecl::ModelTypes) elemType
        , enclosingScope
        , fMemoryManager
    );

    elemDecl->setFinalSet(finalSet);
    elemDecl->setBlockSet(blockSet);
    elemDecl->setMiscFlags(elementMiscFlags);
    elemDecl->setCreateReason(XMLElementDecl::Declared);

    return elemDecl;
}


void TraverseSchema::processAttributeDeclRef(const DOMElement* const elem,
                                             ComplexTypeInfo* const typeInfo,
                                             const XMLCh* const refName,
                                             const XMLCh* const useAttr,
                                             const XMLCh* const defaultVal,
                                             const XMLCh* const fixedVal) {

    if (!typeInfo && !fCurrentAttGroupInfo) {
        return;
    }

    const XMLCh* prefix = getPrefix(refName);
    const XMLCh* localPart = getLocalPart(refName);
    const XMLCh* uriStr = resolvePrefixToURI(elem, prefix);
    unsigned int attURI = fURIStringPool->addOrFind(uriStr);

    // Check for duplicate references
    if (typeInfo && typeInfo->getAttDef(localPart, attURI)) {

        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::DuplicateRefAttribute, uriStr, localPart);
        return;
    }
    else if (fCurrentAttGroupInfo && fCurrentAttGroupInfo->containsAttribute(localPart, attURI)) {

        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::DuplicateRefAttribute, uriStr, localPart);
        return;
    }

    // check for different namespace
    SchemaInfo* saveInfo = fSchemaInfo;
    SchemaInfo::ListType infoType = SchemaInfo::INCLUDE;
    SchemaAttDef* refAttDef = 0;
    int saveScope = fCurrentScope;

    if (!XMLString::equals(uriStr, fTargetNSURIString)) {

        // Make sure that we have an explicit import statement.
        // Clause 4 of Schema Representation Constraint:
        // http://www.w3.org/TR/xmlschema-1/#src-resolve
        unsigned int uriId = fURIStringPool->addOrFind(uriStr);

        if (!fSchemaInfo->isImportingNS(uriId)) {

            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidNSReference, uriStr);
            return;
        }

        Grammar* grammar = fGrammarResolver->getGrammar(uriStr);

        if (grammar == 0 || grammar->getGrammarType() != Grammar::SchemaGrammarType) {

            reportSchemaError(elem, XMLUni::fgValidityDomain, XMLValid::GrammarNotFound, uriStr);
            return;
        }

        refAttDef = (SchemaAttDef*) ((SchemaGrammar*) grammar)->getAttributeDeclRegistry()->get(localPart);

        if (!refAttDef) {

            SchemaInfo* impInfo = fSchemaInfo->getImportInfo(attURI);

            if (!impInfo || impInfo->getProcessed()) {

                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::TopLevelAttributeNotFound, refName);
                return;
            }

            infoType = SchemaInfo::IMPORT;
            restoreSchemaInfo(impInfo, infoType);
        }
    }

    // if Global attribute registry does not contain the ref attribute, get
    // the referred attribute declaration and traverse it.
    if (!refAttDef) {
		
        if (fAttributeDeclRegistry->containsKey(localPart) == false) {

            DOMElement* referredAttribute = fSchemaInfo->getTopLevelComponent(SchemaInfo::C_Attribute,
                SchemaSymbols::fgELT_ATTRIBUTE, localPart, &fSchemaInfo);

            if (referredAttribute != 0) {
                traverseAttributeDecl(referredAttribute, 0, true);
            }
        }

        refAttDef = (SchemaAttDef*) fAttributeDeclRegistry->get(localPart);
    }

    // restore schema information, if necessary
    if (fSchemaInfo != saveInfo) {
        restoreSchemaInfo(saveInfo, infoType, saveScope);
    }

    if (!refAttDef) {

        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::TopLevelAttributeNotFound, refName);
        return;
    }

    XMLAttDef::DefAttTypes refAttDefType = refAttDef->getDefaultType();
    const XMLCh* refAttValue = refAttDef->getValue();
    bool invalidAttUse = false;

    if (refAttDefType == XMLAttDef::Fixed &&
        (defaultVal || (fixedVal && !XMLString::equals(fixedVal, refAttValue)))) {

        invalidAttUse = true;
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::AttUseCorrect, refName);
    }

    DatatypeValidator* attDV = refAttDef->getDatatypeValidator();

    //check for multiple attributes with type derived from ID
    if (attDV && attDV->getType() == DatatypeValidator::ID) {

        if (fCurrentAttGroupInfo) {

            if (fCurrentAttGroupInfo->containsTypeWithId()) {

                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::AttGrpPropCorrect3, refName);
                return;
            }

            fCurrentAttGroupInfo->setTypeWithId(true);
        }
        else {

            if (typeInfo->containsAttWithTypeId()) {

                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::AttDeclPropCorrect5, refName);
                return;
            }

            typeInfo->setAttWithTypeId(true);
        }
    }

    bool required = XMLString::equals(useAttr,SchemaSymbols::fgATTVAL_REQUIRED);
    bool prohibited = XMLString::equals(useAttr,SchemaSymbols::fgATTVAL_PROHIBITED);
    QName* attQName = refAttDef->getAttName();
    SchemaAttDef* attDef = new (fMemoryManager) SchemaAttDef(attQName->getPrefix(),
                                            attQName->getLocalPart(),
                                            attQName->getURI(),
                                            refAttValue,
                                            refAttDef->getType(),
                                            refAttDefType);

    if (refAttDefType == XMLAttDef::Fixed) {
        if (required && !invalidAttUse) {
            attDef->setDefaultType(XMLAttDef::Required_And_Fixed);
        }
    }
    else {

        if (prohibited) {
            attDef->setDefaultType(XMLAttDef::Prohibited);
        }
        else {

            const XMLCh* valueConstraint = defaultVal;

            if (required){

                if (fixedVal) {

                    attDef->setDefaultType(XMLAttDef::Required_And_Fixed);
                    valueConstraint = fixedVal;
                }
                else {
                    attDef->setDefaultType(XMLAttDef::Required);
                }
            }

            if (valueConstraint) {

                // validate content of value constraint
                if (attDV) {

                    if (attDV->getType() == DatatypeValidator::ID) {
                        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::AttDeclPropCorrect3,
                                          SchemaSymbols::fgATT_REF, refName);
                    }
                    else {
                        try {
                            attDV->validate(valueConstraint);
                        }
                        catch(const XMLException& excep) {
                            reportSchemaError(elem, XMLUni::fgValidityDomain, XMLValid::DisplayErrorMessage, excep.getMessage());
                        }
                        catch (...) {
                            reportSchemaError(elem, XMLUni::fgValidityDomain, XMLValid::DatatypeValidationFailure, valueConstraint);
                        }
                    }
                }

                attDef->setValue(valueConstraint);
            }
        }
    }

    attDef->setDatatypeValidator(attDV);

    bool toClone = false;

    if (typeInfo) {

        toClone = true;
        typeInfo->addAttDef(attDef);
    }

    if (fCurrentAttGroupInfo) {
        fCurrentAttGroupInfo->addAttDef(attDef, toClone);
    }
}


void TraverseSchema::checkMinMax(ContentSpecNode* const specNode,
                                 const DOMElement* const elem,
                                 const int allContextFlag) {

    int minOccurs = 1;
    int maxOccurs = 1;
    const XMLCh* minOccursStr = getElementAttValue(elem, SchemaSymbols::fgATT_MINOCCURS, true);
    const XMLCh* maxOccursStr = getElementAttValue(elem, SchemaSymbols::fgATT_MAXOCCURS, true);

    if (!minOccursStr || !*minOccursStr) {
        if (specNode)
            minOccurs = specNode->getMinOccurs();
    }
    else {
        try {
            minOccurs = XMLString::parseInt(minOccursStr);
        }
        catch (...) {
            minOccurs = 1;
        }

        if (specNode)
            specNode->setMinOccurs(minOccurs);
    }

    bool isMaxUnbounded = XMLString::equals(maxOccursStr, fgUnbounded);

    if (isMaxUnbounded) {
        maxOccurs = SchemaSymbols::XSD_UNBOUNDED;
        if (specNode)
            specNode->setMaxOccurs(maxOccurs);
    }
    else {
        if (!maxOccursStr || !*maxOccursStr) {
            if (specNode)
                maxOccurs = specNode->getMaxOccurs();
        }
        else {
            try {
                maxOccurs = XMLString::parseInt(maxOccursStr);
            }
            catch(...) {
                maxOccurs = minOccurs;
            }

            if (specNode)
                specNode->setMaxOccurs(maxOccurs);
        }
    }

    // Constraint checking for min/max value
    if (!isMaxUnbounded) {

        XMLCh tmpMinStr[128];
        XMLCh tmpMaxStr[128];

        XMLString::binToText(minOccurs, tmpMinStr, 127, 10);
        XMLString::binToText(maxOccurs, tmpMaxStr, 127, 10);

        if (maxOccurs < 1) {
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidAttValue,
                              tmpMaxStr, SchemaSymbols::fgATT_MAXOCCURS);
            if (specNode)
                specNode->setMaxOccurs(minOccurs);
        }
        else if (maxOccurs < minOccurs) {

            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidMin2MaxOccurs,
                              tmpMinStr, tmpMaxStr);
            if (specNode)
                specNode->setMaxOccurs(minOccurs);
        }
    }

    if (minOccurs == 0 && maxOccurs == 0){
        return;
    }

    // Constraint checking for 'all' content
    bool isAllElement = (allContextFlag == All_Element);
    bool isAllGroup = (allContextFlag == All_Group);
    bool isGroupRefAll = (allContextFlag == Group_Ref_With_All);

    if (isAllElement || isAllGroup || isGroupRefAll) {

        if (maxOccurs != 1) {

            // set back correct value in order to carry on
            if (specNode) {

                specNode->setMaxOccurs(1);

                if (minOccurs > 1)
                    specNode->setMinOccurs(1);
            }

            if (isAllElement) {
                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::BadMinMaxAllElem);
            }
            else {
                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::BadMinMaxAllCT);
            }
        }
    }
}


void TraverseSchema::processComplexContent(const DOMElement* const ctElem,
                                           const XMLCh* const typeName,
                                           const DOMElement* const childElem,
                                           ComplexTypeInfo* const typeInfo,
                                           const XMLCh* const baseRawName,
                                           const XMLCh* const baseLocalPart,
                                           const XMLCh* const baseURI,
                                           const bool isMixed,
                                           const bool isBaseAnyType) {

    ContentSpecNode*    specNode = 0;
    const DOMElement* attrNode = 0;
    int                 typeDerivedBy = typeInfo->getDerivedBy();
    ComplexTypeInfo*    baseTypeInfo = typeInfo->getBaseComplexTypeInfo();
    int baseContentType = (baseTypeInfo) ? baseTypeInfo->getContentType() : SchemaElementDecl::Empty;

    if (baseTypeInfo) {

        if (typeDerivedBy == SchemaSymbols::XSD_RESTRICTION) {

            // check to see if the baseType permits derivation by restriction
            if((baseTypeInfo->getFinalSet() & typeDerivedBy) != 0) {

                reportSchemaError(ctElem, XMLUni::fgXMLErrDomain, XMLErrs::ForbiddenDerivationByRestriction,
                                  baseLocalPart);
                throw TraverseSchema::InvalidComplexTypeInfo;
            }
        }
        else {

            // check to see if the baseType permits derivation by extension
            if((baseTypeInfo->getFinalSet() & typeDerivedBy) != 0) {

                reportSchemaError(ctElem, XMLUni::fgXMLErrDomain, XMLErrs::ForbiddenDerivationByExtension, baseLocalPart);
                throw TraverseSchema::InvalidComplexTypeInfo; // REVISIT - should we continue
            }

            processElements(ctElem, baseTypeInfo, typeInfo);
        }
    }

    if (childElem != 0) {

        fCircularCheckIndex = fCurrentTypeNameStack->size();

        // --------------------------------------------------------------------
        // GROUP, ALL, SEQUENCE or CHOICE, followed by attributes, if specified.
        // Note that it's possible that only attributes are specified.
        // --------------------------------------------------------------------
        const XMLCh* childName = childElem->getLocalName();

        if (XMLString::equals(childName, SchemaSymbols::fgELT_GROUP)) {

            XercesGroupInfo* grpInfo = traverseGroupDecl(childElem, false);

            if (grpInfo) {

                specNode = grpInfo->getContentSpec();

                if (specNode) {

                    int contentContext = specNode->hasAllContent() ? Group_Ref_With_All : Not_All_Context;

                    specNode = new (fMemoryManager) ContentSpecNode(*specNode);
                    checkMinMax(specNode, childElem, contentContext);
                }
            }

            attrNode = XUtil::getNextSiblingElement(childElem);

        }
        else if (XMLString::equals(childName, SchemaSymbols::fgELT_SEQUENCE)) {

            specNode = traverseChoiceSequence(childElem, ContentSpecNode::Sequence);
            checkMinMax(specNode, childElem);
            attrNode = XUtil::getNextSiblingElement(childElem);
        }
        else if (XMLString::equals(childName, SchemaSymbols::fgELT_CHOICE)) {

            specNode = traverseChoiceSequence(childElem, ContentSpecNode::Choice);
            checkMinMax(specNode, childElem);
            attrNode = XUtil::getNextSiblingElement(childElem);
        }
        else if (XMLString::equals(childName, SchemaSymbols::fgELT_ALL)) {

            specNode = traverseAll(childElem);
            checkMinMax(specNode, childElem, All_Group);
            attrNode = XUtil::getNextSiblingElement(childElem);
        }
        else if (isAttrOrAttrGroup(childElem)) {
            // reset the contentType
            typeInfo->setContentType(SchemaElementDecl::Any);
            attrNode = childElem;
        }
        else {
            reportSchemaError(childElem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidChildInComplexType, childName);
        }
    }

    typeInfo->setContentSpec(specNode);
    typeInfo->setAdoptContentSpec(true);

    // -----------------------------------------------------------------------
    // Merge in information from base, if it exists
    // -----------------------------------------------------------------------
    if (baseTypeInfo) {

        ContentSpecNode* baseSpecNode = baseTypeInfo->getContentSpec();

        if (typeDerivedBy == SchemaSymbols::XSD_RESTRICTION) {

            //check derivation valid - content type is empty (5.2)
            if (!typeInfo->getContentSpec()) {

                if (baseContentType != SchemaElementDecl::Empty
                    && !emptiableParticle(baseSpecNode)) {
                    reportSchemaError(ctElem, XMLUni::fgXMLErrDomain, XMLErrs::EmptyComplexRestrictionDerivation);
                }
            }

            // Delay particle constraint checking (5.3) until we have processed
            // the whole schema.
        }
        else {

            // Compose the final content model by concatenating the base and
            // the current in sequence
            if (!specNode) {

                if (baseSpecNode) {
                    specNode = new (fMemoryManager) ContentSpecNode(*baseSpecNode);
                    typeInfo->setContentSpec(specNode);
                    typeInfo->setAdoptContentSpec(true);
                }
            }
            else if (baseSpecNode) {

                if (specNode->hasAllContent() || baseSpecNode->hasAllContent()) {

                    reportSchemaError(ctElem, XMLUni::fgXMLErrDomain, XMLErrs::NotAllContent);
                    throw TraverseSchema::InvalidComplexTypeInfo; // REVISIT - should we continue
                }

                // Check for derivation valid (extension) - 1.4.3.2.2.1
                if ((isMixed && baseContentType == SchemaElementDecl::Children)
                    || (!isMixed && baseContentType != SchemaElementDecl::Children)) {

                    reportSchemaError(ctElem, XMLUni::fgXMLErrDomain, XMLErrs::MixedOrElementOnly, baseLocalPart, typeName);
                    throw TraverseSchema::InvalidComplexTypeInfo; //REVISIT - should we continue
                }

                typeInfo->setAdoptContentSpec(false);
                typeInfo->setContentSpec(
                    new (fMemoryManager) ContentSpecNode(ContentSpecNode::Sequence,
                                        new (fMemoryManager) ContentSpecNode(*baseSpecNode),
                                        specNode));
                typeInfo->setAdoptContentSpec(true);
            }
        }
    }
    else {
        typeInfo->setDerivedBy(0);
    }

    // -------------------------------------------------------------
    // Set the content type
    // -------------------------------------------------------------
    if (isBaseAnyType && typeDerivedBy == SchemaSymbols::XSD_EXTENSION) {

        ContentSpecNode* anySpecNode = new (fMemoryManager) ContentSpecNode(new (fMemoryManager) QName(XMLUni::fgZeroLenString,
                                                                     XMLUni::fgZeroLenString,
                                                                     fEmptyNamespaceURI),
                                                           false);

        anySpecNode->setType(ContentSpecNode::Any_Lax);
        anySpecNode->setMinOccurs(0);
        anySpecNode->setMaxOccurs(SchemaSymbols::XSD_UNBOUNDED);

        if (!specNode) {
            typeInfo->setContentSpec(anySpecNode);
            typeInfo->setDerivedBy(typeDerivedBy);
        }
        else {

            typeInfo->setAdoptContentSpec(false);
            typeInfo->setContentSpec(new (fMemoryManager) ContentSpecNode(ContentSpecNode::Sequence, anySpecNode, specNode));
            typeInfo->setAdoptContentSpec(true);

            if (!isMixed) {

                reportSchemaError(ctElem, XMLUni::fgXMLErrDomain, XMLErrs::MixedOrElementOnly, baseLocalPart, typeName);
                throw TraverseSchema::InvalidComplexTypeInfo; //REVISIT - should we continue
            }
        }

        typeInfo->setContentType(SchemaElementDecl::Mixed_Complex);
    }
    else if (isMixed) {

        if (specNode != 0) {
            typeInfo->setContentType(SchemaElementDecl::Mixed_Complex);
        }
        else {
            // add #PCDATA leaf and set its minOccurs to 0
            ContentSpecNode* pcdataNode =
                  new (fMemoryManager) ContentSpecNode(new (fMemoryManager) QName(XMLUni::fgZeroLenString,
                                                XMLUni::fgZeroLenString,
                                                XMLElementDecl::fgPCDataElemId),
                                      false);

            pcdataNode->setMinOccurs(0);
            typeInfo->setContentSpec(pcdataNode);
            typeInfo->setAdoptContentSpec(true);
            typeInfo->setContentType(SchemaElementDecl::Mixed_Simple);
        }
    }
    else if (typeInfo->getContentSpec() == 0) {
        typeInfo->setContentType(SchemaElementDecl::Empty);
    }
    else {
        typeInfo->setContentType(SchemaElementDecl::Children);
    }

    // -------------------------------------------------------------
    // Now, check attributes and handle
    // -------------------------------------------------------------
    if (attrNode != 0) {

        if (!isAttrOrAttrGroup(attrNode)) {
            reportSchemaError(attrNode, XMLUni::fgXMLErrDomain, XMLErrs::InvalidChildInComplexType,
                              attrNode->getLocalName());
        }
        else {
            processAttributes(ctElem, attrNode, baseRawName, baseLocalPart, baseURI, typeInfo, isBaseAnyType);
        }
    }
    else if (baseTypeInfo != 0 || isBaseAnyType) {
        processAttributes(ctElem, 0, baseRawName, baseLocalPart, baseURI, typeInfo, isBaseAnyType);
    }
}


void TraverseSchema::processBaseTypeInfo(const DOMElement* const elem,
                                         const XMLCh* const baseName,
                                         const XMLCh* const localPart,
                                         const XMLCh* const uriStr,
                                         ComplexTypeInfo* const typeInfo) {

    SchemaInfo*          saveInfo = fSchemaInfo;
    ComplexTypeInfo*     baseComplexTypeInfo = 0;
    DatatypeValidator*   baseDTValidator = 0;
    SchemaInfo::ListType infoType = SchemaInfo::INCLUDE;
    int                  saveScope = fCurrentScope;

    // check if the base type is from another schema
    if (!XMLString::equals(uriStr, fTargetNSURIString)) {

        // check for datatype validator if it's schema for schema URI
        if (XMLString::equals(uriStr, SchemaSymbols::fgURI_SCHEMAFORSCHEMA)) {

            baseDTValidator = getDatatypeValidator(uriStr, localPart);

            if (!baseDTValidator) {
                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidNSReference, uriStr);
                throw TraverseSchema::InvalidComplexTypeInfo;
            }
        }
        else {
			
            // Make sure that we have an explicit import statement.
            // Clause 4 of Schema Representation Constraint:
            // http://www.w3.org/TR/xmlschema-1/#src-resolve
            unsigned int uriId = fURIStringPool->addOrFind(uriStr);

            if (!fSchemaInfo->isImportingNS(uriId)) {

                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidNSReference, uriStr);
                throw TraverseSchema::InvalidComplexTypeInfo;
            }

            baseComplexTypeInfo = getTypeInfoFromNS(elem, uriStr, localPart);

            if (!baseComplexTypeInfo) {

                SchemaInfo* impInfo = fSchemaInfo->getImportInfo(fURIStringPool->addOrFind(uriStr));

                if (!impInfo || impInfo->getProcessed()) {
                    reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::BaseTypeNotFound, baseName);
                    throw TraverseSchema::InvalidComplexTypeInfo;
                }

                infoType = SchemaInfo::IMPORT;
                restoreSchemaInfo(impInfo, infoType);
            }
        }
    }
    else {

        fBuffer.set(uriStr);
        fBuffer.append(chComma);
        fBuffer.append(localPart);

        // assume the base is a complexType and try to locate the base type first
        const XMLCh* fullBaseName = fBuffer.getRawBuffer();
        baseComplexTypeInfo = fComplexTypeRegistry->get(fullBaseName);

        // Circular check
        if (baseComplexTypeInfo) {

            if (fCurrentTypeNameStack->containsElement(fStringPool->addOrFind(fullBaseName), fCircularCheckIndex)) {

                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::NoCircularDefinition, fullBaseName);
                throw TraverseSchema::InvalidComplexTypeInfo;
            }
            else if (fCurrentTypeNameStack->containsElement(fStringPool->addOrFind(fullBaseName))) {

                typeInfo->setBaseComplexTypeInfo(baseComplexTypeInfo);
                throw TraverseSchema::RecursingElement;
            }
            else if (baseComplexTypeInfo->getPreprocessed()) {
                baseComplexTypeInfo = 0;
            }
        }
    }

    // if not found, 2 possibilities:
    //           1: ComplexType in question has not been compiled yet;
    //           2: base is SimpleType;
    if (!baseComplexTypeInfo && !baseDTValidator) {

        baseDTValidator = getDatatypeValidator(uriStr, localPart);

        if (baseDTValidator == 0) {

            DOMElement* baseTypeNode = fSchemaInfo->getTopLevelComponent(SchemaInfo::C_ComplexType,
                SchemaSymbols::fgELT_COMPLEXTYPE, localPart, &fSchemaInfo);

            if (baseTypeNode != 0) {

                int baseTypeSymbol = traverseComplexTypeDecl(baseTypeNode);
                baseComplexTypeInfo = fComplexTypeRegistry->get(fStringPool->getValueForId(baseTypeSymbol));
            }
            else {

                baseTypeNode = fSchemaInfo->getTopLevelComponent(SchemaInfo::C_SimpleType,
                    SchemaSymbols::fgELT_SIMPLETYPE, localPart, &fSchemaInfo);

                if (baseTypeNode != 0) {

                    baseDTValidator = traverseSimpleTypeDecl(baseTypeNode);

                    if (baseDTValidator == 0)  {

                        // restore schema information, if necessary
                        if (saveInfo != fSchemaInfo) {
                            restoreSchemaInfo(saveInfo, infoType, saveScope);
                        }

                        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::TypeNotFound, uriStr, localPart, uriStr);
                        throw TraverseSchema::InvalidComplexTypeInfo;
                    }
                }
                else {

                    // restore schema information, if necessary
                    if (saveInfo != fSchemaInfo) {
                        restoreSchemaInfo(saveInfo, infoType, saveScope);
                    }
                    reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::BaseTypeNotFound, baseName);
                    throw TraverseSchema::InvalidComplexTypeInfo;
                }
            }
        }
    } // end if

    // restore schema information, if necessary
    if (saveInfo != fSchemaInfo) {
        restoreSchemaInfo(saveInfo, infoType, saveScope);
    }

    typeInfo->setBaseComplexTypeInfo(baseComplexTypeInfo);
    typeInfo->setBaseDatatypeValidator(baseDTValidator);
}


ComplexTypeInfo* TraverseSchema::getTypeInfoFromNS(const DOMElement* const elem,
                                                   const XMLCh* const uriStr,
                                                   const XMLCh* const localPart)
{
    Grammar* grammar = fGrammarResolver->getGrammar(uriStr);

    if (grammar != 0 && grammar->getGrammarType() == Grammar::SchemaGrammarType) {

        fBuffer.set(uriStr);
        fBuffer.append(chComma);
        fBuffer.append(localPart);

        ComplexTypeInfo* typeInfo =
            ((SchemaGrammar*)grammar)->getComplexTypeRegistry()->get(fBuffer.getRawBuffer());

        return typeInfo;
    }
    else {
        reportSchemaError(elem, XMLUni::fgValidityDomain, XMLValid::GrammarNotFound, uriStr);
    }

    return 0;
}


void TraverseSchema::processAttributes(const DOMElement* const elem,
                                       const DOMElement* const attElem,
                                       const XMLCh* const baseRawName,
                                       const XMLCh* const baseLocalPart,
                                       const XMLCh* const baseURI,
                                       ComplexTypeInfo* const typeInfo,
                                       const bool isBaseAnyType) {

    // If we do not have a complexTypeInfo, then what is the point of
    // processing.
    if (typeInfo == 0) {
        return;
    }

    const DOMElement* child = attElem;
    SchemaAttDef* attWildCard = 0;
    Janitor<SchemaAttDef> janAttWildCard(0);
    XercesAttGroupInfo* attGroupInfo = 0;
    ValueVectorOf<XercesAttGroupInfo*> attGroupList(4);

    for (; child != 0; child = XUtil::getNextSiblingElement(child)) {

        const XMLCh* childName = child->getLocalName();

        if (XMLString::equals(childName, SchemaSymbols::fgELT_ATTRIBUTE)) {
            traverseAttributeDecl(child, typeInfo);
        }
        else if (XMLString::equals(childName, SchemaSymbols::fgELT_ATTRIBUTEGROUP)) {
            attGroupInfo = traverseAttributeGroupDecl(child, typeInfo);
            if (attGroupInfo && !attGroupList.containsElement(attGroupInfo)) {
                attGroupList.addElement(attGroupInfo);
            }
        }
        else if (XMLString::equals(childName, SchemaSymbols::fgELT_ANYATTRIBUTE) ) {
            attWildCard = traverseAnyAttribute(child);
            janAttWildCard.reset(attWildCard);
        }
        else {
            reportSchemaError(child, XMLUni::fgXMLErrDomain, XMLErrs::InvalidChildInComplexType, childName);
        }
    }

    // -------------------------------------------------------------
    // Handle wild card/any attribute
    // -------------------------------------------------------------
    ComplexTypeInfo* baseTypeInfo = typeInfo->getBaseComplexTypeInfo();
    int derivedBy = typeInfo->getDerivedBy();
    unsigned int attGroupListSize = attGroupList.size();

    if (attGroupListSize) {

        SchemaAttDef* completeWildCard = 0;
        Janitor<SchemaAttDef> janCompleteWildCard(0);
        XMLAttDef::DefAttTypes defAttType;
        bool defAttTypeSet = false;

        for (unsigned int i=0; i < attGroupListSize; i++) {

            attGroupInfo = attGroupList.elementAt(i);
            unsigned int anyAttCount = attGroupInfo->anyAttributeCount();

            if (anyAttCount) {

                if (!defAttTypeSet) {

                    defAttType = (attWildCard) ? attWildCard->getDefaultType()
                                               : attGroupInfo->anyAttributeAt(0)->getDefaultType();
                    defAttTypeSet = true;
                }

                SchemaAttDef* attGroupWildCard = attGroupInfo->getCompleteWildCard();

                if (!attGroupWildCard) {

                    attGroupWildCard = new (fMemoryManager) SchemaAttDef(attGroupInfo->anyAttributeAt(0));

                    for (unsigned int i= 1; i < anyAttCount; i++) {
                        attWildCardIntersection(attGroupWildCard, attGroupInfo->anyAttributeAt(i));
                    }

                    attGroupInfo->setCompleteWildCard(attGroupWildCard);
                }

                if (completeWildCard) {
                    attWildCardIntersection(completeWildCard, attGroupWildCard);
                }
                else {
                    completeWildCard = new (fMemoryManager) SchemaAttDef(attGroupWildCard);
                    janCompleteWildCard.reset(completeWildCard);
                }
            }

        }

        if (completeWildCard) {

            if (attWildCard) {
                attWildCardIntersection(attWildCard, completeWildCard);
            }
            else {

                attWildCard = completeWildCard;
                janCompleteWildCard.orphan();
                janAttWildCard.reset(attWildCard);
            }

            attWildCard->setDefaultType(defAttType);
		}
    }

    SchemaAttDef* baseAttWildCard = (baseTypeInfo) ? baseTypeInfo->getAttWildCard() : 0;
    Janitor<SchemaAttDef> janBaseAttWildCard(0);

    if (derivedBy == SchemaSymbols::XSD_EXTENSION) {

        if (isBaseAnyType) {

            baseAttWildCard = new (fMemoryManager) SchemaAttDef(XMLUni::fgZeroLenString,
                                               XMLUni::fgZeroLenString,
                                               fEmptyNamespaceURI, XMLAttDef::Any_Any,
                                               XMLAttDef::ProcessContents_Lax);
            janBaseAttWildCard.reset(baseAttWildCard);
        }
		
        if (baseAttWildCard && attWildCard) {

            XMLAttDef::DefAttTypes saveDefType = attWildCard->getDefaultType();
            attWildCardUnion(attWildCard, baseAttWildCard);
            attWildCard->setDefaultType(saveDefType);
        }
    }

    // -------------------------------------------------------------
    // insert wildcard attribute
    // -------------------------------------------------------------
    if (attWildCard) {

        typeInfo->setAttWildCard(attWildCard);
        janAttWildCard.orphan();

        if (attWildCard->getType() == XMLAttDef::AttTypes_Unknown) {
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::NotExpressibleWildCardIntersection);
        }
    }
    else if (baseAttWildCard && derivedBy == SchemaSymbols::XSD_EXTENSION) {

        if (isBaseAnyType) {

            typeInfo->setAttWildCard(baseAttWildCard);
            janBaseAttWildCard.orphan();
        }
        else {

            SchemaAttDef* newWildCard = new (fMemoryManager) SchemaAttDef(baseAttWildCard);
            typeInfo->setAttWildCard(newWildCard);
        }
    }

    // -------------------------------------------------------------
    // Check attributes derivation OK
    // -------------------------------------------------------------
    bool baseWithAttributes = (baseTypeInfo && baseTypeInfo->hasAttDefs());
    bool childWithAttributes = (typeInfo->hasAttDefs() || typeInfo->getAttWildCard());

    if (derivedBy == SchemaSymbols::XSD_RESTRICTION && childWithAttributes) {

        if (!baseWithAttributes && !baseAttWildCard) {
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::BadAttDerivation_1);
        }
        else {
            checkAttDerivationOK(elem, baseTypeInfo, typeInfo);
        }
    }

    // -------------------------------------------------------------
    // merge in base type's attribute decls
    // -------------------------------------------------------------
    if (baseTypeInfo && baseTypeInfo->hasAttDefs()) {

        SchemaAttDefList& baseAttList = (SchemaAttDefList&)
                                        baseTypeInfo->getAttDefList();

        while (baseAttList.hasMoreElements()) {

            SchemaAttDef& attDef = (SchemaAttDef&) baseAttList.nextElement();
            QName* attName = attDef.getAttName();
            const XMLCh* localPart = attName->getLocalPart();

            // if found a duplicate, then skip the one from the base type
            if (typeInfo->getAttDef(localPart, attName->getURI()) != 0) {

                if (derivedBy == SchemaSymbols::XSD_EXTENSION) {
                    reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::DuplicateAttInDerivation, localPart);
                }

                continue;
            }

            if (attDef.getDefaultType() != XMLAttDef::Prohibited) {

                SchemaAttDef* newAttDef = new (fMemoryManager) SchemaAttDef(attName->getPrefix(),
                                                           attName->getLocalPart(),
                                                           attName->getURI(),
                                                           attDef.getValue(),
                                                           attDef.getType(),
                                                           attDef.getDefaultType(),
                                                           attDef.getEnumeration());

                newAttDef->setDatatypeValidator(attDef.getDatatypeValidator());
                typeInfo->addAttDef(newAttDef);
            }
        }
    }
}


void TraverseSchema::defaultComplexTypeInfo(ComplexTypeInfo* const typeInfo) {

    if (typeInfo) {

        typeInfo->setDerivedBy(0);
        typeInfo->setContentType(SchemaElementDecl::Any);
        typeInfo->setDatatypeValidator(0);
        typeInfo->setContentSpec(0);
        typeInfo->setBaseComplexTypeInfo(0);
        typeInfo->setBaseDatatypeValidator(0);
    }
}


InputSource* TraverseSchema::resolveSchemaLocation(const XMLCh* const loc) {

    // ------------------------------------------------------------------
    // Create an input source
    // ------------------------------------------------------------------
    InputSource* srcToFill = 0;
    normalizeURI(loc, fBuffer);

    const XMLCh* normalizedURI = fBuffer.getRawBuffer();
    if (fEntityHandler){
        srcToFill = fEntityHandler->resolveEntity(XMLUni::fgZeroLenString, normalizedURI);
    }

    //  If they didn't create a source via the entity resolver, then we
    //  have to create one on our own.
    if (!srcToFill) {

        try {

            XMLURL urlTmp(fSchemaInfo->getCurrentSchemaURL(), normalizedURI);

            if (urlTmp.isRelative()) {
                ThrowXML(MalformedURLException,
                         XMLExcepts::URL_NoProtocolPresent);
            }
            else {
                if (fScanner->getStandardUriConformant() && urlTmp.hasInvalidChar())
                    ThrowXML(MalformedURLException, XMLExcepts::URL_MalformedURL);
                srcToFill = new (fMemoryManager) URLInputSource(urlTmp);
            }
        }
        catch(const MalformedURLException& e) {
            // Its not a URL, so lets assume its a local file name if non-standard URI is allowed
            if (!fScanner->getStandardUriConformant())
                srcToFill = new (fMemoryManager) LocalFileInputSource(fSchemaInfo->getCurrentSchemaURL(),normalizedURI);
            else
                throw e;
        }
    }

    return srcToFill;
}


void TraverseSchema::restoreSchemaInfo(SchemaInfo* const toRestore,
                                       SchemaInfo::ListType const aListType,
                                       const int saveScope) {


    if (aListType == SchemaInfo::IMPORT) { // restore grammar info

        fSchemaInfo->setScopeCount(fScopeCount);

        int targetNSURI = toRestore->getTargetNSURI();
        fSchemaGrammar = (SchemaGrammar*) fGrammarResolver->getGrammar(toRestore->getTargetNSURIString());

        if (!fSchemaGrammar) {
            return;
        }

        fTargetNSURI = targetNSURI;
        fCurrentScope = saveScope;
        fScopeCount = toRestore->getScopeCount();
        fDatatypeRegistry = fSchemaGrammar->getDatatypeRegistry();
        fTargetNSURIString = fSchemaGrammar->getTargetNamespace();
        fGroupRegistry = fSchemaGrammar->getGroupInfoRegistry();
        fAttGroupRegistry = fSchemaGrammar->getAttGroupInfoRegistry();
        fAttributeDeclRegistry = fSchemaGrammar->getAttributeDeclRegistry();
        fComplexTypeRegistry = fSchemaGrammar->getComplexTypeRegistry();
        fValidSubstitutionGroups = fSchemaGrammar->getValidSubstitutionGroups();
        fNamespaceScope = fSchemaGrammar->getNamespaceScope();
        fAttributeCheck.setIDRefList(fSchemaGrammar->getIDRefList());
    }

    fSchemaInfo = toRestore;
}


bool
TraverseSchema::emptiableParticle(const ContentSpecNode* const specNode) {

    if (!fFullConstraintChecking || (specNode->getMinTotalRange() == 0)) {
        return true;
    }

    return false;
}

void TraverseSchema::checkFixedFacet(const DOMElement* const elem,
                                     const XMLCh* const facetName,
                                     const DatatypeValidator* const baseDV,
                                     unsigned int& flags)
{
    const XMLCh* fixedFacet = getElementAttValue(elem, SchemaSymbols::fgATT_FIXED);

    if ((fixedFacet && *fixedFacet) &&
        (XMLString::equals(fixedFacet, SchemaSymbols::fgATTVAL_TRUE)
         || XMLString::equals(fixedFacet, fgValueOne))) {

        if (XMLString::equals(SchemaSymbols::fgELT_MINLENGTH, facetName)) {
            flags |= DatatypeValidator::FACET_MINLENGTH;
        }
        else if (XMLString::equals(SchemaSymbols::fgELT_MAXLENGTH, facetName)) {
            flags |= DatatypeValidator::FACET_MAXLENGTH;
        }
        else if (XMLString::equals(SchemaSymbols::fgELT_MAXEXCLUSIVE, facetName)) {
            flags |= DatatypeValidator::FACET_MAXEXCLUSIVE;
        }
        else if (XMLString::equals(SchemaSymbols::fgELT_MAXINCLUSIVE, facetName)) {
            flags |= DatatypeValidator::FACET_MAXINCLUSIVE;
        }
        else if (XMLString::equals(SchemaSymbols::fgELT_MINEXCLUSIVE, facetName)) {
            flags |= DatatypeValidator::FACET_MINEXCLUSIVE;
        }
        else if (XMLString::equals(SchemaSymbols::fgELT_MININCLUSIVE, facetName)) {
            flags |= DatatypeValidator::FACET_MININCLUSIVE;
        }
        else if (XMLString::equals(SchemaSymbols::fgELT_TOTALDIGITS, facetName)) {
            flags |= DatatypeValidator::FACET_TOTALDIGITS;
        }
        else if (XMLString::equals(SchemaSymbols::fgELT_FRACTIONDIGITS, facetName)) {
            flags |= DatatypeValidator::FACET_FRACTIONDIGITS;
        }
        else if ((XMLString::equals(SchemaSymbols::fgELT_WHITESPACE, facetName)) &&
                 baseDV->getType() == DatatypeValidator::String) {
            flags |= DatatypeValidator::FACET_WHITESPACE;
        }
    }
}

void
TraverseSchema::buildValidSubstitutionListB(const DOMElement* const elem,
                                            SchemaElementDecl* const elemDecl,
                                            SchemaElementDecl* const subsElemDecl) {

    SchemaElementDecl* chainElemDecl = subsElemDecl->getSubstitutionGroupElem();

    while (chainElemDecl) {

        int chainElemURI = chainElemDecl->getURI();
        XMLCh* chainElemName = chainElemDecl->getBaseName();
        ValueVectorOf<SchemaElementDecl*>* validSubsElements =
            fValidSubstitutionGroups->get(chainElemName, chainElemURI);

        if (!validSubsElements) {

			if (fTargetNSURI == chainElemURI) {
                break; // an error must have occured
            }

            SchemaGrammar* aGrammar = (SchemaGrammar*)
                fGrammarResolver->getGrammar(fURIStringPool->getValueForId(chainElemURI));

            if (!aGrammar)
                break;

            validSubsElements = aGrammar->getValidSubstitutionGroups()->get(chainElemName, chainElemURI);

            if (!validSubsElements) {
                break;
            }
			
            validSubsElements = new (fMemoryManager) ValueVectorOf<SchemaElementDecl*>(*validSubsElements);
            fValidSubstitutionGroups->put((void*) chainElemName, chainElemURI, validSubsElements);
        }

        if (validSubsElements->containsElement(elemDecl) ||
            !isSubstitutionGroupValid(elem, chainElemDecl, elemDecl->getComplexTypeInfo(),
                                      elemDecl->getDatatypeValidator(), 0, false)) {
            break;
        }

        validSubsElements->addElement(elemDecl);

        // update related subs. info in case of circular import
        BaseRefVectorEnumerator<SchemaInfo> importingEnum = fSchemaInfo->getImportingListEnumerator();

        while (importingEnum.hasMoreElements()) {

            const SchemaInfo& curRef = importingEnum.nextElement();
            SchemaGrammar* aGrammar = (SchemaGrammar*) fGrammarResolver->getGrammar(curRef.getTargetNSURIString());
            ValueVectorOf<SchemaElementDecl*>* subsElemList =
                aGrammar->getValidSubstitutionGroups()->get(chainElemName, chainElemURI);

            if (subsElemList && !subsElemList->containsElement(elemDecl)) {
                subsElemList->addElement(elemDecl);
            }
        }

        chainElemDecl = chainElemDecl->getSubstitutionGroupElem();
    }
}

void
TraverseSchema::buildValidSubstitutionListF(const DOMElement* const elem,
                                            SchemaElementDecl* const elemDecl,
                                            SchemaElementDecl* const subsElemDecl) {

    int elemURI = elemDecl->getURI();
    XMLCh* elemName = elemDecl->getBaseName();
    ValueVectorOf<SchemaElementDecl*>* validSubsElements =fValidSubstitutionGroups->get(elemName, elemURI);

    if (validSubsElements) {

        int subsElemURI = subsElemDecl->getURI();
        XMLCh* subsElemName = subsElemDecl->getBaseName();
        ValueVectorOf<SchemaElementDecl*>* validSubs = fValidSubstitutionGroups->get(subsElemName, subsElemURI);

        if (!validSubs) {

			if (fTargetNSURI == subsElemURI) {
                return; // an error must have occured
            }

            SchemaGrammar* aGrammar = (SchemaGrammar*)
                fGrammarResolver->getGrammar(fURIStringPool->getValueForId(subsElemURI));

            if (!aGrammar)
                return;

            validSubs = aGrammar->getValidSubstitutionGroups()->get(subsElemName, subsElemURI);

            if (!validSubs) {
                return;
            }
			
            validSubs = new (fMemoryManager) ValueVectorOf<SchemaElementDecl*>(*validSubs);
            fValidSubstitutionGroups->put((void*) subsElemName, subsElemURI, validSubs);
        }

        unsigned int elemSize = validSubsElements->size();
        for (unsigned int i=0; i<elemSize; i++) {

            SchemaElementDecl* chainElem = validSubsElements->elementAt(i);

            if (validSubs->containsElement(chainElem)) {
                continue;
            }

            if (isSubstitutionGroupValid(elem, subsElemDecl, chainElem->getComplexTypeInfo(),
                                         chainElem->getDatatypeValidator(), 0, false)) {
                validSubs->addElement(chainElem);
                buildValidSubstitutionListB(elem, chainElem, subsElemDecl);
            }
        }
    }
}

void TraverseSchema::checkEnumerationRequiredNotation(const DOMElement* const elem,
                                                      const XMLCh* const name,
                                                      const XMLCh* const type) {

    const XMLCh* localPart = getLocalPart(type);

    if (XMLString::equals(localPart, SchemaSymbols::fgELT_NOTATION)) {
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::NoNotationType, name);
    }
}

XercesGroupInfo* TraverseSchema::processGroupRef(const DOMElement* const elem,
                                                 const XMLCh* const refName) {

    if (checkContent(elem, XUtil::getFirstChildElement(elem), true) != 0) {
        reportSchemaError(elem, XMLUni::fgValidityDomain, XMLValid::NoContentForRef, SchemaSymbols::fgELT_GROUP);
    }

    const XMLCh* prefix = getPrefix(refName);
    const XMLCh* localPart = getLocalPart(refName);
    const XMLCh* uriStr = resolvePrefixToURI(elem, prefix);

    fBuffer.set(uriStr);
    fBuffer.append(chComma);
    fBuffer.append(localPart);

    unsigned int nameIndex = fStringPool->addOrFind(fBuffer.getRawBuffer());
	
    if (fCurrentGroupStack->containsElement(nameIndex)) {

        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::NoCircularDefinition, localPart);
        return 0;
    }

    XercesGroupInfo*     groupInfo = 0;
    SchemaInfo*          saveInfo = fSchemaInfo;
    SchemaInfo::ListType infoType = SchemaInfo::INCLUDE;
    int                  saveScope = fCurrentScope;

    //if from another target namespace
    if (!XMLString::equals(uriStr, fTargetNSURIString)) {

        // Make sure that we have an explicit import statement.
        // Clause 4 of Schema Representation Constraint:
        // http://www.w3.org/TR/xmlschema-1/#src-resolve
        unsigned int uriId = fURIStringPool->addOrFind(uriStr);

        if (!fSchemaInfo->isImportingNS(uriId)) {

            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidNSReference, uriStr);
            return 0;
        }

        Grammar* aGrammar = fGrammarResolver->getGrammar(uriStr);

        if (!aGrammar || aGrammar->getGrammarType() != Grammar::SchemaGrammarType) {

            reportSchemaError(elem, XMLUni::fgValidityDomain, XMLValid::GrammarNotFound, uriStr);
            return 0;
        }

        groupInfo = ((SchemaGrammar*)aGrammar)->getGroupInfoRegistry()->get(fStringPool->getValueForId(nameIndex));

        if (!groupInfo) {

            SchemaInfo* impInfo = fSchemaInfo->getImportInfo(fURIStringPool->addOrFind(uriStr));

            if (!impInfo || impInfo->getProcessed()) {

                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::DeclarationNotFound,
                                  SchemaSymbols::fgELT_GROUP, uriStr, localPart);
                return 0;
            }

            infoType = SchemaInfo::IMPORT;
            restoreSchemaInfo(impInfo, infoType);
        }
    }
    else {
        groupInfo = fGroupRegistry->get(fStringPool->getValueForId(nameIndex));
    }

    if (!groupInfo) {

        DOMElement* groupElem = fSchemaInfo->getTopLevelComponent(SchemaInfo::C_Group,
            SchemaSymbols::fgELT_GROUP, localPart, &fSchemaInfo);

        if (groupElem != 0) {

            groupInfo = traverseGroupDecl(groupElem);

            // restore schema information
            restoreSchemaInfo(saveInfo, infoType, saveScope);

            if (groupInfo && (fCurrentGroupInfo || infoType == SchemaInfo::IMPORT)) {
                copyGroupElements(elem, groupInfo, fCurrentGroupInfo,
                                  (infoType == SchemaInfo::IMPORT) ? fCurrentComplexType : 0);
            }

            return groupInfo;
        }
        else {
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::DeclarationNotFound,
                              SchemaSymbols::fgELT_GROUP, uriStr, localPart);
        }

        // restore schema information, if necessary
        if (saveInfo != fSchemaInfo) {
            restoreSchemaInfo(saveInfo, infoType, saveScope);
        }
    }
    else {
        copyGroupElements(elem, groupInfo, fCurrentGroupInfo, fCurrentComplexType);
    }

    return groupInfo;
}


XercesAttGroupInfo*
TraverseSchema::processAttributeGroupRef(const DOMElement* const elem,
                                         const XMLCh* const refName,
                                         ComplexTypeInfo* const typeInfo) {

if (checkContent(elem, XUtil::getFirstChildElement(elem), true) != 0) {
        reportSchemaError(elem ,XMLUni::fgValidityDomain, XMLValid::NoContentForRef, SchemaSymbols::fgELT_ATTRIBUTEGROUP);
    }

    const                XMLCh* prefix = getPrefix(refName);
    const                XMLCh* localPart = getLocalPart(refName);
    const                XMLCh* uriStr = resolvePrefixToURI(elem, prefix);
    XercesAttGroupInfo*  attGroupInfo = 0;
    SchemaInfo*          saveInfo = fSchemaInfo;
    SchemaInfo::ListType infoType = SchemaInfo::INCLUDE;
    int                  saveScope = fCurrentScope;

    if (!XMLString::equals(uriStr, fTargetNSURIString)) {

        // Make sure that we have an explicit import statement.
        // Clause 4 of Schema Representation Constraint:
        // http://www.w3.org/TR/xmlschema-1/#src-resolve
        unsigned int uriId = fURIStringPool->addOrFind(uriStr);

        if (!fSchemaInfo->isImportingNS(uriId)) {

            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidNSReference, uriStr);
            return 0;
        }

        attGroupInfo = traverseAttributeGroupDeclNS(elem, uriStr, localPart);

        if (!attGroupInfo) {
            SchemaInfo* impInfo = fSchemaInfo->getImportInfo(fURIStringPool->addOrFind(uriStr));

            if (!impInfo || impInfo->getProcessed()) {

                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::DeclarationNotFound,
                                  SchemaSymbols::fgELT_ATTRIBUTEGROUP, uriStr, localPart);
                return 0;
            }

            infoType = SchemaInfo::IMPORT;
            restoreSchemaInfo(impInfo, infoType);
        }
    }
    else {

        attGroupInfo = fAttGroupRegistry->get(localPart);
    }

    if (!attGroupInfo) {

        // traverse top level attributeGroup - if found
        DOMElement* attGroupElem = fSchemaInfo->getTopLevelComponent(SchemaInfo::C_AttributeGroup,
            SchemaSymbols::fgELT_ATTRIBUTEGROUP, localPart, &fSchemaInfo);

        if (attGroupElem != 0) {

            // circular attribute check
            if (fDeclStack->containsElement(attGroupElem)) {

                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::NoCircularDefinition, refName);
                return 0;
            }

            attGroupInfo = traverseAttributeGroupDecl(attGroupElem, typeInfo, true);

            if (attGroupInfo && fCurrentAttGroupInfo) {
                copyAttGroupAttributes(elem, attGroupInfo, fCurrentAttGroupInfo, 0);
            }

            // restore schema information, if necessary
            if (saveInfo != fSchemaInfo) {
                restoreSchemaInfo(saveInfo, infoType, saveScope);
            }

            return attGroupInfo;
        }
        else {
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::DeclarationNotFound,
                              SchemaSymbols::fgELT_ATTRIBUTEGROUP, uriStr, localPart);
        }
    }

    if (attGroupInfo) {
        copyAttGroupAttributes(elem, attGroupInfo, fCurrentAttGroupInfo, typeInfo);
    }

    // restore schema information, if necessary
    if (saveInfo != fSchemaInfo) {
        restoreSchemaInfo(saveInfo, infoType);
    }

    return attGroupInfo;
}

void TraverseSchema::processElements(const DOMElement* const elem,
                                     ComplexTypeInfo* const baseTypeInfo,
                                     ComplexTypeInfo* const newTypeInfo) {

    unsigned int elemCount = baseTypeInfo->elementCount();

    if (elemCount) {

        int newTypeScope = newTypeInfo->getScopeDefined();
        int schemaURI = fURIStringPool->addOrFind(SchemaSymbols::fgURI_SCHEMAFORSCHEMA);

        for (unsigned int i=0; i < elemCount; i++) {

            SchemaGrammar*     aGrammar = fSchemaGrammar;
            SchemaElementDecl* elemDecl = baseTypeInfo->elementAt(i);
            int elemURI = elemDecl->getURI();
            int elemScope = elemDecl->getEnclosingScope();

            if (elemScope != Grammar::TOP_LEVEL_SCOPE) {

                if (elemURI != fTargetNSURI && elemURI != schemaURI && elemURI != fEmptyNamespaceURI) {

                    Grammar* aGrammar =
                        fGrammarResolver->getGrammar(fURIStringPool->getValueForId(elemURI));

                    if (!aGrammar || aGrammar->getGrammarType() != Grammar::SchemaGrammarType) {
                        continue; // REVISIT - error message
                    }
                }

                const XMLCh*             localPart = elemDecl->getBaseName();
                const SchemaElementDecl* other = (SchemaElementDecl*)
                    aGrammar->getElemDecl(elemURI, localPart, 0, newTypeScope);

                if (other) {

                    if (elemDecl->getComplexTypeInfo() != other->getComplexTypeInfo()
                        || elemDecl->getDatatypeValidator() != other->getDatatypeValidator()) {
                        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::DuplicateElementDeclaration, localPart);
                    }

                    continue;
                }
                elemDecl->setEnclosingScope(newTypeScope);
                ((SchemaGrammar*) aGrammar)->putGroupElemDecl(elemDecl);
                elemDecl->setEnclosingScope(elemScope);
            }

            newTypeInfo->addElement(elemDecl);
        }
    }
}


void TraverseSchema::copyGroupElements(const DOMElement* const elem,
                                       XercesGroupInfo* const fromGroup,
                                       XercesGroupInfo* const toGroup,
                                       ComplexTypeInfo* const typeInfo) {

    unsigned int elemCount = fromGroup->elementCount();
    int newScope = (typeInfo) ? typeInfo->getScopeDefined() : 0;

    if (typeInfo)
        fromGroup->setCheckElementConsistency(false);

    for (unsigned int i = 0; i < elemCount; i++) {

        SchemaElementDecl*       elemDecl = fromGroup->elementAt(i);

        if (typeInfo) {

            int elemScope = elemDecl->getEnclosingScope();

            if (elemScope != Grammar::TOP_LEVEL_SCOPE) {

                int                      elemURI = elemDecl->getURI();
                const XMLCh*             localPart = elemDecl->getBaseName();
                const SchemaElementDecl* other = (SchemaElementDecl*)
                        fSchemaGrammar->getElemDecl(elemURI, localPart, 0, fCurrentScope);

                if (other) {

                    if (elemDecl->getComplexTypeInfo() != other->getComplexTypeInfo()
                        || elemDecl->getDatatypeValidator() != other->getDatatypeValidator()) {
                       reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::DuplicateElementDeclaration, localPart);
                    }

                    continue;
                }

                elemDecl->setEnclosingScope(newScope);
                fSchemaGrammar->putGroupElemDecl(elemDecl);
                elemDecl->setEnclosingScope(elemScope);
            }

            typeInfo->addElement(elemDecl);
        }

        if (toGroup) {
            toGroup->addElement(elemDecl);
        }
    }
}

void TraverseSchema::copyAttGroupAttributes(const DOMElement* const elem,
                                            XercesAttGroupInfo* const fromAttGroup,
                                            XercesAttGroupInfo* const toAttGroup,
                                            ComplexTypeInfo* const typeInfo) {

    unsigned int attCount = fromAttGroup->attributeCount();

    for (unsigned int i=0; i < attCount; i++) {

        SchemaAttDef* attDef = fromAttGroup->attributeAt(i);
        QName* attName = attDef->getAttName();
        const XMLCh* localPart = attName->getLocalPart();
        DatatypeValidator* attDV = attDef->getDatatypeValidator();

        if (typeInfo) {

            if (typeInfo->getAttDef(localPart, attName->getURI())) {

                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::DuplicateAttribute, localPart);
                continue;
            }

            if (attDV && attDV->getType() == DatatypeValidator::ID) {

                if (typeInfo->containsAttWithTypeId()) {

                    reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::AttDeclPropCorrect5, localPart);
                    continue;
                }

                typeInfo->setAttWithTypeId(true);
            }

            typeInfo->addAttDef(new (fMemoryManager) SchemaAttDef(attDef));

            if (toAttGroup) {
                toAttGroup->addAttDef(attDef, true);
            }
        }
        else {

            if (toAttGroup->containsAttribute(localPart, attName->getURI())) {

                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::DuplicateAttribute, localPart);
                continue;
            }

            if (attDV && attDV->getType() == DatatypeValidator::ID) {

                if (toAttGroup->containsTypeWithId()) {

                    reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::AttGrpPropCorrect3, localPart);
                    continue;
                }

                toAttGroup->setTypeWithId(true);
            }

            toAttGroup->addAttDef(attDef, true);
        }
    }

    if (toAttGroup) {
        unsigned int anyAttCount = fromAttGroup->anyAttributeCount();

        for (unsigned int j=0; j < anyAttCount; j++) {
            toAttGroup->addAnyAttDef(fromAttGroup->anyAttributeAt(j), true);
        }
    }
}

void
TraverseSchema::attWildCardIntersection(SchemaAttDef* const resultWildCard,
                                        const SchemaAttDef* const compareWildCard) {

    XMLAttDef::AttTypes typeR = resultWildCard->getType();
    XMLAttDef::AttTypes typeC = compareWildCard->getType();

    //If either O1 or O2 is any, then the other must be the value.
    if (typeC == XMLAttDef::Any_Any ||
        typeR == XMLAttDef::AttTypes_Unknown) {
        return;
    }

    if (typeR == XMLAttDef::Any_Any ||
        typeC == XMLAttDef::AttTypes_Unknown) {

        resultWildCard->resetNamespaceList();
        copyWildCardData(compareWildCard, resultWildCard);
        return;
    }

    // If either O1 or O2 is a pair of not and a namespace name and the other
    // is a set, then that set, minus the negated namespace name if it was in
    // is the value
    if ((typeC == XMLAttDef::Any_Other && typeR == XMLAttDef::Any_List) ||
        (typeR == XMLAttDef::Any_Other && typeC == XMLAttDef::Any_List)) {

		unsigned int compareURI = 0;
        ValueVectorOf<unsigned int>* nameURIList = 0;

        if (typeC == XMLAttDef::Any_List) {
            nameURIList = compareWildCard->getNamespaceList();
            compareURI = resultWildCard->getAttName()->getURI();
        }
        else {
            nameURIList = resultWildCard->getNamespaceList();
            compareURI = compareWildCard->getAttName()->getURI();
        }

        unsigned int listSize = (nameURIList) ? nameURIList->size() : 0;

        if (listSize) {

            bool                        found = false;
            ValueVectorOf<unsigned int> tmpURIList(listSize);

            for (unsigned int i=0; i < listSize; i++) {

                unsigned int nameURI = nameURIList->elementAt(i);

                if (nameURI != compareURI &&
                    nameURI != (unsigned int) fEmptyNamespaceURI) {
                    tmpURIList.addElement(nameURI);
                }
                else {
                    found = true;
                }
            }

            if (found || typeC == XMLAttDef::Any_List) {
                resultWildCard->setNamespaceList(&tmpURIList);
            }
        }

        if (typeC == XMLAttDef::Any_List) {
            copyWildCardData(compareWildCard, resultWildCard);
        }

        return;
    }

    // If both O1 and O2 are sets, then the intersection of those sets must be
    // the value.
    if (typeR == XMLAttDef::Any_List && typeC == XMLAttDef::Any_List) {
		
        ValueVectorOf<unsigned int>* uriListR = resultWildCard->getNamespaceList();
        ValueVectorOf<unsigned int>* uriListC = compareWildCard->getNamespaceList();
        unsigned int listSize = (uriListC) ? uriListC->size() : 0;

        if (listSize) {

            ValueVectorOf<unsigned int> tmpURIList(listSize);

            for (unsigned int i=0; i < listSize; i++) {

                unsigned int uriName = uriListC->elementAt(i);

                if (uriListR && uriListR->containsElement(uriName)) {
                    tmpURIList.addElement(uriName);
                }
            }

            resultWildCard->setNamespaceList(&tmpURIList);
        }
        else {
            resultWildCard->resetNamespaceList();
        }

        return;
    }

    // If the two are negations of different namespace names, then:
    //  if one is a negation of absent, then result is negation of namespace
    //  else intersection is not expressible.
    if (typeR == XMLAttDef::Any_Other && typeC == XMLAttDef::Any_Other) {

        QName* qnameR = resultWildCard->getAttName();

        if (qnameR->getURI() != compareWildCard->getAttName()->getURI()) {

            if (qnameR->getURI() == (unsigned int)fEmptyNamespaceURI) {
			    qnameR->setURI(compareWildCard->getAttName()->getURI());
            }
			else if (compareWildCard->getAttName()->getURI() != (unsigned int)fEmptyNamespaceURI) {

                qnameR->setURI(fEmptyNamespaceURI);
                resultWildCard->setType(XMLAttDef::AttTypes_Unknown);
            }
        }
    }
}


void
TraverseSchema::attWildCardUnion(SchemaAttDef* const resultWildCard,
                                 const SchemaAttDef* const compareWildCard) {

    XMLAttDef::AttTypes typeR = resultWildCard->getType();
    XMLAttDef::AttTypes typeC = compareWildCard->getType();

    //If either O1 or O2 is any, then the other must be the value.
    if (typeR == XMLAttDef::Any_Any ||
        typeR == XMLAttDef::AttTypes_Unknown) {
        return;
    }

    if (typeC == XMLAttDef::Any_Any ||
        typeC == XMLAttDef::AttTypes_Unknown) {

        resultWildCard->resetNamespaceList();
        copyWildCardData(compareWildCard, resultWildCard);
        return;
    }

    // If both O1 and O2 are sets, then the union of those sets must be
    // the value.
    if (typeR == XMLAttDef::Any_List && typeC == XMLAttDef::Any_List) {

        ValueVectorOf<unsigned int>* uriListR = resultWildCard->getNamespaceList();
        ValueVectorOf<unsigned int>* uriListC = compareWildCard->getNamespaceList();
        unsigned int listSizeC = (uriListC) ? uriListC->size() : 0;

        if (listSizeC) {

            if (!uriListR || !uriListR->size()) {

                resultWildCard->setNamespaceList(uriListC);
                return;
            }

            ValueVectorOf<unsigned int> tmpURIList(*uriListR);

            for (unsigned int i = 0; i < listSizeC; i++) {

                unsigned int uriName = uriListC->elementAt(i);

                if (!uriListR->containsElement(uriName)) {
                    tmpURIList.addElement(uriName);
                }
            }

            resultWildCard->setNamespaceList(&tmpURIList);
        }

        return;
    }

    // If the two are negations of different namespace names, then not and
    // absent must be the value
    if (typeR == XMLAttDef::Any_Other && typeC == XMLAttDef::Any_Other) {

        QName* qnameR = resultWildCard->getAttName();

        if (qnameR->getURI() != compareWildCard->getAttName()->getURI()) {

            qnameR->setURI(fEmptyNamespaceURI);
            resultWildCard->setType(XMLAttDef::Any_Other);
        }
    }

    // 5. If either O1 or O2 is a pair of not and a namespace name and the
    //    other is a set, then:
    //    1. If the set includes both the negated namespace name and absent
    //       then any must be the value.
    //    2. If the set includes the negated namespace name but not absent,
    //       then a pair of not and absent must be the value.
    //    3. If the set includes absent but not the negated namespace name,
    //       then the union is not expressible.
    //    4. If the set does not include either the negated namespace or
    //       absent, then whichever of O1 or O2 is a pair of not and a
    //       namespace name.
    //
    // 6. If either O1 or O2 is a pair of not and absent and the other is a
    //    set, then:
    //    1. If the set includes absent then any must be the value.
    //    2. If the set does not include absent, then a pair of not and
    //       absent.
    if ((typeC == XMLAttDef::Any_Other && typeR == XMLAttDef::Any_List) ||
		(typeR == XMLAttDef::Any_Other && typeC == XMLAttDef::Any_List)) {

        ValueVectorOf<unsigned int>* nameURIList = 0;
        QName* attNameR = resultWildCard->getAttName();
        unsigned int compareURI = 0;

        if (typeC == XMLAttDef::Any_List) {
            nameURIList = compareWildCard->getNamespaceList();
            compareURI = attNameR->getURI();
        }
        else {
            nameURIList = resultWildCard->getNamespaceList();
            compareURI = compareWildCard->getAttName()->getURI();
        }

        // 6. not and absent
        if (compareURI == (unsigned int) fEmptyNamespaceURI) {

            if (nameURIList) {

                // 6.1 result is any
                if (nameURIList->containsElement(compareURI)) {

                    resultWildCard->setType(XMLAttDef::Any_Any);
                    attNameR->setURI(fEmptyNamespaceURI);
                }
                // 6.2 result is not and absent
                else if (typeR == XMLAttDef::Any_List){

                    resultWildCard->setType(XMLAttDef::Any_Other);
                    attNameR->setURI(fEmptyNamespaceURI);
                }
            }
            // 6.2 result is not and absent
            else if (typeR == XMLAttDef::Any_List) {

                resultWildCard->setType(XMLAttDef::Any_Other);
                attNameR->setURI(fEmptyNamespaceURI);
            }
        }
        // 5. not and namespace
        else {

            // 5.3 result is not expressible
            if (!nameURIList) {
                resultWildCard->setType(XMLAttDef::AttTypes_Unknown);
                attNameR->setURI(fEmptyNamespaceURI);
            }
            else {
                bool containsAbsent =
                    nameURIList->containsElement(fEmptyNamespaceURI);
                bool containsNamespace =
                    nameURIList->containsElement(compareURI);

                // 5.1 result is any
                if (containsAbsent && containsNamespace) {

                    resultWildCard->setType(XMLAttDef::Any_Any);
                    attNameR->setURI(fEmptyNamespaceURI);
                }
                // 5.2 result is not and absent
                else if (containsNamespace) {

                    resultWildCard->setType(XMLAttDef::Any_Other);
                    attNameR->setURI(fEmptyNamespaceURI);
                }
                // 5.3 result is not expressible
                else if (containsAbsent) {

                    resultWildCard->setType(XMLAttDef::AttTypes_Unknown);
                    attNameR->setURI(fEmptyNamespaceURI);
                }
                // 5.4. whichever is not and namespace
                else if (typeR == XMLAttDef::Any_List) {

                    resultWildCard->setType(XMLAttDef::Any_Other);
                    attNameR->setURI(compareURI);
                }
            }
        }

        resultWildCard->resetNamespaceList();
    }
}


void TraverseSchema::checkAttDerivationOK(const DOMElement* const elem,
                                          const ComplexTypeInfo* const baseTypeInfo,
                                          const ComplexTypeInfo* const childTypeInfo) {

    SchemaAttDefList& childAttList = (SchemaAttDefList&) childTypeInfo->getAttDefList();
    const SchemaAttDef* baseAttWildCard = baseTypeInfo->getAttWildCard();

    while (childAttList.hasMoreElements()) {

        SchemaAttDef& childAttDef = (SchemaAttDef&) childAttList.nextElement();
        QName* childAttName = childAttDef.getAttName();
        const XMLCh* childLocalPart = childAttName->getLocalPart();
        const SchemaAttDef* baseAttDef = baseTypeInfo->getAttDef(childLocalPart, childAttName->getURI());

        if (baseAttDef) {

            XMLAttDef::DefAttTypes baseAttDefType = baseAttDef->getDefaultType();
            XMLAttDef::DefAttTypes childAttDefType = childAttDef.getDefaultType();

            // Constraint 2.1.1 & 3 + check for prohibited base attribute
            if (baseAttDefType == XMLAttDef::Prohibited
                && childAttDefType != XMLAttDef::Prohibited) {
                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::BadAttDerivation_8, childLocalPart);
            }

            if ((baseAttDefType & XMLAttDef::Required)
                && !(childAttDefType & XMLAttDef::Required)) {
                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::BadAttDerivation_2, childLocalPart);
            }

            // Constraint 2.1.2
            DatatypeValidator* baseDV = baseAttDef->getDatatypeValidator();
            DatatypeValidator* childDV = childAttDef.getDatatypeValidator();
            if (!baseDV || !baseDV->isSubstitutableBy(childDV)) {
                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::BadAttDerivation_3, childLocalPart);
            }

            // Constraint 2.1.3
            if ((baseAttDefType & XMLAttDef::Fixed) &&
                (!(childAttDefType & XMLAttDef::Fixed) ||
                 !XMLString::equals(baseAttDef->getValue(), childAttDef.getValue()))) {
                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::BadAttDerivation_4, childLocalPart);
            }
        }
        // Constraint 2.2
        else if (!baseAttWildCard ||
                 !wildcardAllowsNamespace(baseAttWildCard, childAttName->getURI())) {
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::BadAttDerivation_5, childLocalPart);
        }
    }

    // Constraint 4
    const SchemaAttDef* childAttWildCard = childTypeInfo->getAttWildCard();

    if (childAttWildCard) {

        if (!baseAttWildCard) {
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::BadAttDerivation_6);
        }
        else if (!isWildCardSubset(baseAttWildCard, childAttWildCard)) {
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::BadAttDerivation_7);
        }
        else if (childAttWildCard->getDefaultType() < baseAttWildCard->getDefaultType()) {
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::BadAttDerivation_9);
        }
    }
}

void TraverseSchema::checkAttDerivationOK(const DOMElement* const elem,
                                          const XercesAttGroupInfo* const baseAttGrpInfo,
                                          const XercesAttGroupInfo* const childAttGrpInfo) {

    unsigned int baseAttCount = baseAttGrpInfo->attributeCount();
    unsigned int baseAnyAttCount = baseAttGrpInfo->anyAttributeCount();
    unsigned int childAttCount = childAttGrpInfo->attributeCount();
    unsigned int childAnyAttCount = childAttGrpInfo->anyAttributeCount();

    if ((childAttCount || childAnyAttCount) && (!baseAttCount && !baseAnyAttCount)) {
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::BadAttDerivation_1);
    }

    const SchemaAttDef* baseAttWildCard = (baseAnyAttCount) ? baseAttGrpInfo->anyAttributeAt(0) : 0;

    for (unsigned int i=0; i<childAttCount; i++) {

        const SchemaAttDef* childAttDef = childAttGrpInfo->attributeAt(i);
        QName* childAttName = childAttDef->getAttName();
        const XMLCh* childLocalPart = childAttName->getLocalPart();
        const SchemaAttDef* baseAttDef = baseAttGrpInfo->getAttDef(childLocalPart, childAttName->getURI());

        if (baseAttDef) {

            XMLAttDef::DefAttTypes baseAttDefType = baseAttDef->getDefaultType();
            XMLAttDef::DefAttTypes childAttDefType = childAttDef->getDefaultType();

            // Constraint 2.1.1 & 3 + check for prohibited base attribute
            if (baseAttDefType == XMLAttDef::Prohibited
                && childAttDefType != XMLAttDef::Prohibited) {
                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::BadAttDerivation_8, childLocalPart);
            }

            if ((baseAttDefType & XMLAttDef::Required)
                && !(childAttDefType & XMLAttDef::Required)) {
                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::BadAttDerivation_2, childLocalPart);
            }

            // Constraint 2.1.2
            DatatypeValidator* baseDV = baseAttDef->getDatatypeValidator();
            DatatypeValidator* childDV = childAttDef->getDatatypeValidator();
            if (!baseDV || !baseDV->isSubstitutableBy(childDV)) {
                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::BadAttDerivation_3, childLocalPart);
            }

            // Constraint 2.1.3
            if ((baseAttDefType & XMLAttDef::Fixed) &&
                (!(childAttDefType & XMLAttDef::Fixed) ||
                 !XMLString::equals(baseAttDef->getValue(), childAttDef->getValue()))) {
                reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::BadAttDerivation_4, childLocalPart);
            }
        }
        // Constraint 2.2
        else if (!baseAttWildCard ||
                 !wildcardAllowsNamespace(baseAttWildCard, childAttName->getURI())) {
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::BadAttDerivation_5, childLocalPart);
        }
    }

    // Constraint 4
    const SchemaAttDef* childAttWildCard = (childAnyAttCount) ? childAttGrpInfo->anyAttributeAt(0) : 0;

    if (childAttWildCard) {

        if (!baseAttWildCard) {
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::BadAttDerivation_6);
        }
        else if (!isWildCardSubset(baseAttWildCard, childAttWildCard)) {
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::BadAttDerivation_7);
        }
        else if (childAttWildCard->getDefaultType() < baseAttWildCard->getDefaultType()) {
            reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::BadAttDerivation_9);
        }
    }
}

bool TraverseSchema::wildcardAllowsNamespace(const SchemaAttDef* const wildCard,
                                             const unsigned int nameURI) {

    XMLAttDef::AttTypes wildCardType = wildCard->getType();

    // The constraint must be any
    if (wildCardType == XMLAttDef::Any_Any) {
        return true;
    }

    // All of the following must be true:
    //    2.1 The constraint is a pair of not and a namespace name or �absent
    //    2.2 The value must not be identical to the �namespace test�.
    //    2.3 The value must not be �absent�.
    if (wildCardType == XMLAttDef::Any_Other &&
		((int) nameURI) != fEmptyNamespaceURI &&
        wildCard->getAttName()->getURI() != nameURI) {
        return true;
    }

    // The constraint is a set, and the value is identical to one of the
    // members of the set
    if (wildCardType == XMLAttDef::Any_List) {

        ValueVectorOf<unsigned int>* nameURIList = wildCard->getNamespaceList();		

        if (nameURIList->containsElement(nameURI)) {
            return true;
        }
    }

    return false;
}

bool TraverseSchema::isWildCardSubset(const SchemaAttDef* const baseAttWildCard,
                                      const SchemaAttDef* const childAttWildCard) {

    XMLAttDef::AttTypes baseWildCardType = baseAttWildCard->getType();
    XMLAttDef::AttTypes childWildCardType = childAttWildCard->getType();

    if (baseWildCardType == XMLAttDef::AttTypes_Unknown ||
        childWildCardType == XMLAttDef::AttTypes_Unknown) {
        return false;
    }

    // 1 super must be any.
    if (baseWildCardType == XMLAttDef::Any_Any) {
        return true;
    }

    // 2 All of the following must be true:
    //     2.1 sub must be a pair of not and a namespace name or �absent�.
    //     2.2 super must be a pair of not and the same value.
    if (childWildCardType == XMLAttDef::Any_Other && baseWildCardType == XMLAttDef::Any_Other &&
        childAttWildCard->getAttName()->getURI() == baseAttWildCard->getAttName()->getURI()) {
        return true;
    }

    // 3 All of the following must be true:
    //     3.1 sub must be a set whose members are either namespace names or �absent�.
    //     3.2 One of the following must be true:
    //          3.2.1 super must be the same set or a superset thereof.
    //          3.2.2 super must be a pair of not and a namespace name or �absent� and
    //                 that value must not be in sub's set.
    if (childWildCardType == XMLAttDef::Any_List) {

        ValueVectorOf<unsigned int>* childURIList = childAttWildCard->getNamespaceList();

		if (baseWildCardType == XMLAttDef::Any_List) {

            ValueVectorOf<unsigned int>* baseURIList = baseAttWildCard->getNamespaceList();
            unsigned int childListSize = (childURIList) ? childURIList->size() : 0;

            for (unsigned int i=0; i<childListSize; i++) {
                if (!baseURIList->containsElement(childURIList->elementAt(i))) {
                    return false;
                }
            }

            return true;
        }
        else if (baseWildCardType == XMLAttDef::Any_Other) {
            if (!childURIList->containsElement(baseAttWildCard->getAttName()->getURI())) {
                return true;
            }
        }
    }

    return false;
}

bool TraverseSchema::openRedefinedSchema(const DOMElement* const redefineElem) {

    if (fPreprocessedNodes->containsKey(redefineElem)) {

        restoreSchemaInfo(fPreprocessedNodes->get(redefineElem));
        return true;
    }

    // ------------------------------------------------------------------
    // Get 'schemaLocation' attribute
    // ------------------------------------------------------------------
    const XMLCh* schemaLocation = getElementAttValue(redefineElem, SchemaSymbols::fgATT_SCHEMALOCATION);

    if (!schemaLocation || !*schemaLocation) {
        reportSchemaError(redefineElem, XMLUni::fgXMLErrDomain, XMLErrs::DeclarationNoSchemaLocation, SchemaSymbols::fgELT_REDEFINE);
        return false;
    }

    // ------------------------------------------------------------------
    // Resolve schema location
    // ------------------------------------------------------------------
    InputSource*         srcToFill = resolveSchemaLocation(schemaLocation);
    Janitor<InputSource> janSrc(srcToFill);

    // Nothing to do
    if (!srcToFill) {
        return false;
    }

    const XMLCh* includeURL = srcToFill->getSystemId();

    if (XMLString::equals(includeURL, fSchemaInfo->getCurrentSchemaURL())) {
        return false;
    }

    SchemaInfo* redefSchemaInfo = fSchemaInfoList->get(includeURL, fTargetNSURI);

    if (redefSchemaInfo) {

        reportSchemaError(redefineElem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidRedefine, includeURL);
        return false;
    }

    // ------------------------------------------------------------------
    // Parse input source
    // ------------------------------------------------------------------
    if (!fParser)
        fParser = new (fMemoryManager) XSDDOMParser;

    fParser->setValidationScheme(XercesDOMParser::Val_Never);
    fParser->setDoNamespaces(true);
    fParser->setUserEntityHandler(fEntityHandler);
    fParser->setUserErrorReporter(fErrorReporter);

    // Should just issue warning if the schema is not found
    const bool flag = srcToFill->getIssueFatalErrorIfNotFound();
    srcToFill->setIssueFatalErrorIfNotFound(false);

    fParser->parse(*srcToFill) ;

    // Reset the InputSource
    srcToFill->setIssueFatalErrorIfNotFound(flag);

    if (fParser->getSawFatal() && fScanner->getExitOnFirstFatal())
        reportSchemaError(redefineElem, XMLUni::fgXMLErrDomain, XMLErrs::SchemaScanFatalError);

    // ------------------------------------------------------------------
    // Get root element
    // ------------------------------------------------------------------
    DOMDocument* document = fParser->getDocument();

    if (!document) {
        return false;
    }
    else {

        DOMElement* root = document->getDocumentElement();

        if (root == 0) {
            return false;
        }

        const XMLCh* targetNSURIString = root->getAttribute(SchemaSymbols::fgATT_TARGETNAMESPACE);

        // check to see if targetNameSpace is right
        if (*targetNSURIString
            && !XMLString::equals(targetNSURIString,fTargetNSURIString)){
            reportSchemaError(root, XMLUni::fgXMLErrDomain, XMLErrs::RedefineNamespaceDifference,
                              schemaLocation, targetNSURIString);
            return false;
        }

        // if targetNamespace is empty, change it to redefin'g schema
        // targetNamespace
        if (!*targetNSURIString && root->getAttributeNode(XMLUni::fgXMLNSString) == 0
            && fTargetNSURI != fEmptyNamespaceURI) {
            root->setAttribute(XMLUni::fgXMLNSString, fTargetNSURIString);
        }

        // --------------------------------------------------------
        // Update schema information with redefined schema
        // --------------------------------------------------------
        redefSchemaInfo = fSchemaInfo;
        fSchemaInfo = new (fMemoryManager) SchemaInfo(0, 0, 0, fTargetNSURI, fScopeCount,
                                     fNamespaceScope->increaseDepth(),
                                     XMLString::replicate(includeURL),
                                     fTargetNSURIString, root);

        traverseSchemaHeader(root);
        fSchemaInfoList->put((void*) fSchemaInfo->getCurrentSchemaURL(), fSchemaInfo->getTargetNSURI(), fSchemaInfo);
        redefSchemaInfo->addSchemaInfo(fSchemaInfo, SchemaInfo::INCLUDE);
        fPreprocessedNodes->put((void*) redefineElem, fSchemaInfo);
    }

    return true;
}

void TraverseSchema::renameRedefinedComponents(const DOMElement* const redefineElem,
                                               SchemaInfo* const redefiningSchemaInfo,
                                               SchemaInfo* const redefinedSchemaInfo) {

    DOMElement* child = XUtil::getFirstChildElement(redefineElem);

    for (; child != 0; child = XUtil::getNextSiblingElement(child)) {

        const XMLCh* childName = child->getLocalName();

        if (XMLString::equals(childName, SchemaSymbols::fgELT_ANNOTATION)) {
            continue;
        }

        // if component already redefined skip
        const XMLCh* typeName = getElementAttValue(child, SchemaSymbols::fgATT_NAME);

        fBuffer.set(fTargetNSURIString);
        fBuffer.append(chComma);
        fBuffer.append(typeName);

        if (fRedefineComponents->containsKey(childName, fStringPool->addOrFind(fBuffer.getRawBuffer()))) {
            continue;
        }

        // Rename
        const XMLCh* tmpChildName = fStringPool->getValueForId(fStringPool->addOrFind(childName));

        if (validateRedefineNameChange(child, tmpChildName, typeName, 1, redefiningSchemaInfo)) {
            fixRedefinedSchema(child, redefinedSchemaInfo, tmpChildName, typeName, 1);
        }
        else {
            redefiningSchemaInfo->addFailedRedefine(child);
        }
    }
}

bool TraverseSchema::validateRedefineNameChange(const DOMElement* const redefineChildElem,
                                                const XMLCh* const redefineChildComponentName,
                                                const XMLCh* const redefineChildTypeName,
                                                const int redefineNameCounter,
                                                SchemaInfo* const redefiningSchemaInfo) {

    const XMLCh* baseTypeName = 0;
    unsigned int typeNameId = fStringPool->addOrFind(redefineChildTypeName);

    fBuffer.set(fTargetNSURIString);
    fBuffer.append(chComma);
    fBuffer.append(redefineChildTypeName);

    int   fullTypeNameId = fStringPool->addOrFind(fBuffer.getRawBuffer());
    const XMLCh* typeNameStr = fStringPool->getValueForId(fullTypeNameId);

    restoreSchemaInfo(redefiningSchemaInfo);

    if (XMLString::equals(redefineChildComponentName,SchemaSymbols::fgELT_SIMPLETYPE)) {

        if (fDatatypeRegistry->getDatatypeValidator(typeNameStr)) {
            return false;
        }

        DOMElement* grandKid = XUtil::getFirstChildElement(redefineChildElem);

        if (grandKid && XMLString::equals(grandKid->getLocalName(), SchemaSymbols::fgELT_ANNOTATION)) {
            grandKid = XUtil::getNextSiblingElement(grandKid);
        }

        if (grandKid == 0) {

            reportSchemaError(redefineChildElem, XMLUni::fgXMLErrDomain, XMLErrs::Redefine_InvalidSimpleType);
            return false;
        }
        else if(!XMLString::equals(grandKid->getLocalName(), SchemaSymbols::fgELT_RESTRICTION)) {

            reportSchemaError(grandKid, XMLUni::fgXMLErrDomain, XMLErrs::Redefine_InvalidSimpleType);
            return false;
        }

        baseTypeName = getElementAttValue(grandKid, SchemaSymbols::fgATT_BASE);
        const XMLCh* prefix = getPrefix(baseTypeName);
        const XMLCh* localPart = getLocalPart(baseTypeName);
        const XMLCh* uriStr = resolvePrefixToURI(grandKid, prefix);

        if (fTargetNSURI != (int) fURIStringPool->addOrFind(uriStr)
            || fStringPool->addOrFind(localPart) != typeNameId) {
            reportSchemaError(grandKid, XMLUni::fgXMLErrDomain, XMLErrs::Redefine_InvalidSimpleTypeBase);
            return false;
        }

        // now we have to do the renaming...
        getRedefineNewTypeName(baseTypeName, redefineNameCounter, fBuffer);
        grandKid->setAttribute(SchemaSymbols::fgATT_BASE, fBuffer.getRawBuffer());
        fRedefineComponents->put((void*) SchemaSymbols::fgELT_SIMPLETYPE,
                                 fullTypeNameId, 0);
    }
    else if (XMLString::equals(redefineChildComponentName,SchemaSymbols::fgELT_COMPLEXTYPE)) {

        if (fComplexTypeRegistry->containsKey(typeNameStr)) {
            return false;
        }

        DOMElement* grandKid = XUtil::getFirstChildElement(redefineChildElem);

        if (grandKid && XMLString::equals(grandKid->getLocalName(), SchemaSymbols::fgELT_ANNOTATION)) {
            grandKid = XUtil::getNextSiblingElement(grandKid);
        }

        if (grandKid == 0) {
            reportSchemaError(redefineChildElem, XMLUni::fgXMLErrDomain, XMLErrs::Redefine_InvalidComplexType);
            return false;
        } else {

            // have to go one more level down; let another pass worry whether complexType is valid.
            DOMElement* greatGrandKid = XUtil::getFirstChildElement(grandKid);

            if (greatGrandKid != 0 &&
                XMLString::equals(greatGrandKid->getLocalName(), SchemaSymbols::fgELT_ANNOTATION)) {
                greatGrandKid = XUtil::getNextSiblingElement(greatGrandKid);
            }

            if (greatGrandKid == 0) {

                reportSchemaError(grandKid, XMLUni::fgXMLErrDomain, XMLErrs::Redefine_InvalidComplexType);
                return false;
            } else {

                const XMLCh* greatGrandKidName = greatGrandKid->getLocalName();

                if (!XMLString::equals(greatGrandKidName, SchemaSymbols::fgELT_RESTRICTION)
                    && !XMLString::equals(greatGrandKidName, SchemaSymbols::fgELT_EXTENSION)) {

                    reportSchemaError(greatGrandKid, XMLUni::fgXMLErrDomain, XMLErrs::Redefine_InvalidComplexType);
                    return false;
                }

                baseTypeName = getElementAttValue(greatGrandKid, SchemaSymbols::fgATT_BASE);
                const XMLCh* prefix = getPrefix(baseTypeName);
                const XMLCh* localPart = getLocalPart(baseTypeName);
                const XMLCh* uriStr = resolvePrefixToURI(greatGrandKid, prefix);

                if (fTargetNSURI != (int) fURIStringPool->addOrFind(uriStr)
                    || fStringPool->addOrFind(localPart) != typeNameId) {
                    reportSchemaError(greatGrandKid, XMLUni::fgXMLErrDomain, XMLErrs::Redefine_InvalidComplexTypeBase);
                    return false;
                }

                // now we have to do the renaming...
                getRedefineNewTypeName(baseTypeName, redefineNameCounter, fBuffer);
                greatGrandKid->setAttribute(SchemaSymbols::fgATT_BASE, fBuffer.getRawBuffer());
                fRedefineComponents->put((void*) SchemaSymbols::fgELT_COMPLEXTYPE,
                                         fullTypeNameId, 0);
            }
        }
    }
    else if (XMLString::equals(redefineChildComponentName, SchemaSymbols::fgELT_GROUP)) {

        if (fGroupRegistry->containsKey(typeNameStr)) {
            return false;
        }

        int groupRefCount = changeRedefineGroup(redefineChildElem, redefineChildComponentName,
                                                redefineChildTypeName, redefineNameCounter);

        if (groupRefCount > 1) {

            reportSchemaError(redefineChildElem, XMLUni::fgXMLErrDomain, XMLErrs::Redefine_GroupRefCount);
            return false;
        }
        else if (groupRefCount == 0) {
            // put a dummy value, default is null.
            // when processing groups, we will check that table, if a value
            // is found, we need to do a particle derivation check.
            fRedefineComponents->put((void*) SchemaSymbols::fgELT_GROUP,
                                     fullTypeNameId, fSchemaInfo->getCurrentSchemaURL());
        }
        else {
            fRedefineComponents->put((void*) SchemaSymbols::fgELT_GROUP, fullTypeNameId, 0);
        }
    }
    else if (XMLString::equals(redefineChildComponentName, SchemaSymbols::fgELT_ATTRIBUTEGROUP)) {

        if (fAttGroupRegistry->containsKey(redefineChildTypeName)) {
            return false;
        }

        int attGroupRefCount = changeRedefineGroup(redefineChildElem, redefineChildComponentName,
                                                   redefineChildTypeName, redefineNameCounter);

        if (attGroupRefCount > 1) {

            reportSchemaError(redefineChildElem, XMLUni::fgXMLErrDomain, XMLErrs::Redefine_AttGroupRefCount);
            return false;
        }
        else if (attGroupRefCount == 0) {
            // put a dummy value, default is null.
            // when processing attributeGroups, we will check that table, if
            // a value is found, we need to check for attribute derivation ok
            // (by restriction)
            fRedefineComponents->put((void*) SchemaSymbols::fgELT_ATTRIBUTEGROUP,
                                     fullTypeNameId, fSchemaInfo->getCurrentSchemaURL());
        }
        else {
            fRedefineComponents->put((void*) SchemaSymbols::fgELT_ATTRIBUTEGROUP, fullTypeNameId, 0);
        }
    }
    else {
        reportSchemaError
        (
            redefineChildElem
            , XMLUni::fgXMLErrDomain
            , XMLErrs::Redefine_InvalidChild
            , redefineChildComponentName
        );
        return false;
    }

    return true;
}

int TraverseSchema::changeRedefineGroup(const DOMElement* const redefineChildElem,
                                        const XMLCh* const redefineChildComponentName,
                                        const XMLCh* const redefineChildTypeName,
                                        const int redefineNameCounter) {
    int result = 0;
    DOMElement* child = XUtil::getFirstChildElement(redefineChildElem);

    for (; child != 0; child = XUtil::getNextSiblingElement(child)) {

        const XMLCh* name = child->getLocalName();

        if (XMLString::equals(name, SchemaSymbols::fgELT_ANNOTATION)) {
            continue;
        }

        if (!XMLString::equals(name, redefineChildComponentName)) {
            result += changeRedefineGroup(child, redefineChildComponentName, redefineChildTypeName, redefineNameCounter);
        } else {
            const XMLCh* refName = getElementAttValue(child, SchemaSymbols::fgATT_REF);

            if (refName && *refName) {

                const XMLCh* prefix = getPrefix(refName);
                const XMLCh* localPart = getLocalPart(refName);
                const XMLCh* uriStr = resolvePrefixToURI(child, prefix);

                if (fTargetNSURI == (int) fURIStringPool->addOrFind(uriStr)
                    && fStringPool->addOrFind(localPart) == fStringPool->addOrFind(redefineChildTypeName)) {

                    // now we have to do the renaming...
                    getRedefineNewTypeName(refName, redefineNameCounter, fBuffer);
                    child->setAttribute(SchemaSymbols::fgATT_REF, fBuffer.getRawBuffer());
                    result++;

                    if(XMLString::equals(redefineChildComponentName, SchemaSymbols::fgELT_GROUP)) {

                        const XMLCh* minOccurs = getElementAttValue(child, SchemaSymbols::fgATT_MINOCCURS);
                        const XMLCh* maxOccurs = getElementAttValue(child, SchemaSymbols::fgATT_MAXOCCURS);

                        if (((maxOccurs && *maxOccurs) && !XMLString::equals(maxOccurs, fgValueOne))
                            || ((minOccurs && *minOccurs) && !XMLString::equals(minOccurs, fgValueOne))) {
                            reportSchemaError(child, XMLUni::fgXMLErrDomain, XMLErrs::Redefine_InvalidGroupMinMax, redefineChildTypeName);
                        }
                    }
                }
            } // if ref was null some other stage of processing will flag the error
        }
    }

    return result;
}


void TraverseSchema::fixRedefinedSchema(const DOMElement* const elem,
                                        SchemaInfo* const redefinedSchemaInfo,
                                        const XMLCh* const redefineChildComponentName,
                                        const XMLCh* const redefineChildTypeName,
                                        const int redefineNameCounter) {

    bool foundIt = false;
    DOMElement* child = XUtil::getFirstChildElement(redefinedSchemaInfo->getRoot());

    restoreSchemaInfo(redefinedSchemaInfo);

    for (; child != 0; child = XUtil::getNextSiblingElement(child)) {

        const XMLCh* name = child->getLocalName();

        if (XMLString::equals(name, redefineChildComponentName)) {

            const XMLCh* infoItemName = getElementAttValue(child, SchemaSymbols::fgATT_NAME);

            if(!XMLString::equals(infoItemName, redefineChildTypeName)) {
                continue;
            }
            else { // found it!

                // now we have to do the renaming...
                foundIt = true;
                getRedefineNewTypeName(infoItemName, redefineNameCounter, fBuffer);
                child->setAttribute(SchemaSymbols::fgATT_NAME, fBuffer.getRawBuffer());
                break;
            }
        }
        else if (XMLString::equals(name, SchemaSymbols::fgELT_REDEFINE)) { // need to search the redefine decl...

            for (DOMElement* redefChild = XUtil::getFirstChildElement(child);
				 redefChild != 0;
				 redefChild = XUtil::getNextSiblingElement(redefChild)) {

                const XMLCh* redefName = redefChild->getLocalName();

                if (XMLString::equals(redefName, redefineChildComponentName)) {

                    const XMLCh* infoItemName = getElementAttValue(redefChild, SchemaSymbols::fgATT_NAME);

                    if(!XMLString::equals(infoItemName, redefineChildTypeName)) {
                        continue;
                    }
                    else { // found it!

                        if (!openRedefinedSchema(child)) {

                            redefinedSchemaInfo->addFailedRedefine(child);
                            return;
                        }

                        foundIt = true;

                        SchemaInfo* reRedefinedSchemaInfo = fSchemaInfo;

                        if (validateRedefineNameChange(redefChild, redefineChildComponentName, redefineChildTypeName, redefineNameCounter + 1, redefinedSchemaInfo)) {

                            fixRedefinedSchema(redefChild, reRedefinedSchemaInfo, redefineChildComponentName, redefineChildTypeName, redefineNameCounter + 1);

                            // now we have to do the renaming...
                            getRedefineNewTypeName(infoItemName, redefineNameCounter, fBuffer);
                            const XMLCh* newInfoItemName = fStringPool->getValueForId(fStringPool->addOrFind(fBuffer.getRawBuffer()));
                            redefChild->setAttribute(SchemaSymbols::fgATT_NAME, newInfoItemName);

                            // and we now know we will traverse this, so set fRedefineComponents appropriately...
                            fBuffer.set(fTargetNSURIString);
                            fBuffer.append(chComma);
                            fBuffer.append(newInfoItemName);
                        }
                        else {

                            fixRedefinedSchema(redefChild, reRedefinedSchemaInfo, redefineChildComponentName, redefineChildTypeName, redefineNameCounter);
                            redefinedSchemaInfo->addFailedRedefine(redefChild);

                            // and we now know we will traverse this, so set fRedefineComponents appropriately...
                            fBuffer.set(fTargetNSURIString);
                            fBuffer.append(chComma);
                            fBuffer.append(infoItemName);
                        }

                        unsigned int infoItemNameId = fStringPool->addOrFind(fBuffer.getRawBuffer());

                        if (!fRedefineComponents->containsKey(redefineChildComponentName, infoItemNameId)) {
                            fRedefineComponents->put((void*) redefineChildComponentName, infoItemNameId, 0);
                        }

                        break;
                    }
                }
            } //for

            if (foundIt) {
                break;
            }
        }
    } //for

    if(!foundIt) {
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::Redefine_DeclarationNotFound, redefineChildTypeName);
    }
}

bool TraverseSchema::isSubstitutionGroupCircular(SchemaElementDecl* const elemDecl,
                                                 SchemaElementDecl* const subsElemDecl)
{

    if (elemDecl == subsElemDecl)
        return true;

    SchemaElementDecl* tmpElemDecl = subsElemDecl->getSubstitutionGroupElem();

    while (tmpElemDecl)
    {
        if (tmpElemDecl == elemDecl)
            return true;

        tmpElemDecl = tmpElemDecl->getSubstitutionGroupElem();
    }

    return false;
}

// ---------------------------------------------------------------------------
//  TraverseSchema: Error reporting methods
// ---------------------------------------------------------------------------
void TraverseSchema::reportSchemaError(const XSDLocator* const aLocator,
                                       const XMLCh* const msgDomain,
                                       const int errorCode) {

    fXSDErrorReporter.emitError(errorCode, msgDomain, aLocator);
}

void TraverseSchema::reportSchemaError(const XSDLocator* const aLocator,
                                       const XMLCh* const msgDomain,
                                       const int errorCode,
                                       const XMLCh* const text1,
                                       const XMLCh* const text2,
                                       const XMLCh* const text3,
                                       const XMLCh* const text4) {

    fXSDErrorReporter.emitError(errorCode, msgDomain, aLocator, text1, text2, text3, text4);
}

void TraverseSchema::reportSchemaError(const DOMElement* const elem,
                                       const XMLCh* const msgDomain,
                                       const int errorCode) {

    fLocator->setValues(fSchemaInfo->getCurrentSchemaURL(), 0,
                        ((XSDElementNSImpl*) elem)->getLineNo(),
                        ((XSDElementNSImpl*) elem)->getColumnNo());

    fXSDErrorReporter.emitError(errorCode, msgDomain, fLocator);
}

void TraverseSchema::reportSchemaError(const DOMElement* const elem,
                                       const XMLCh* const msgDomain,
                                       const int errorCode,
                                       const XMLCh* const text1,
                                       const XMLCh* const text2,
                                       const XMLCh* const text3,
                                       const XMLCh* const text4) {

    fLocator->setValues(fSchemaInfo->getCurrentSchemaURL(), 0,
                        ((XSDElementNSImpl*) elem)->getLineNo(),
                        ((XSDElementNSImpl*) elem)->getColumnNo());

    fXSDErrorReporter.emitError(errorCode, msgDomain, fLocator, text1, text2, text3, text4);
}

// ---------------------------------------------------------------------------
//  TraverseSchema: Init/CleanUp methods
// ---------------------------------------------------------------------------
void TraverseSchema::init() {

    fXSDErrorReporter.setErrorReporter(fErrorReporter);
    fXSDErrorReporter.setExitOnFirstFatal(fScanner->getExitOnFirstFatal());

    fFullConstraintChecking = fScanner->getValidationSchemaFullChecking();

    fDatatypeRegistry = fSchemaGrammar->getDatatypeRegistry();
    fStringPool = fGrammarResolver->getStringPool();
    fEmptyNamespaceURI = fScanner->getEmptyNamespaceId();
    fCurrentTypeNameStack = new (fMemoryManager) ValueVectorOf<unsigned int>(8);
    fCurrentGroupStack = new (fMemoryManager) ValueVectorOf<unsigned int>(8);

    fGlobalDeclarations = (ValueVectorOf<unsigned int>**) fMemoryManager->allocate
    (
        ENUM_ELT_SIZE * sizeof(ValueVectorOf<unsigned int>*)
    );//new ValueVectorOf<unsigned int>*[ENUM_ELT_SIZE];
    for(unsigned int i=0; i < ENUM_ELT_SIZE; i++)
        fGlobalDeclarations[i] = new (fMemoryManager) ValueVectorOf<unsigned int>(8);

    fNotationRegistry = new (fMemoryManager) RefHash2KeysTableOf<XMLCh>(13, false);
    fSchemaInfoList = new (fMemoryManager) RefHash2KeysTableOf<SchemaInfo>(29);
    fPreprocessedNodes = new (fMemoryManager) RefHashTableOf<SchemaInfo>(29, false, new (fMemoryManager) HashPtr());
    fLocator = new XSDLocator();
    fDeclStack = new (fMemoryManager) ValueVectorOf<const DOMElement*>(16);
}

void TraverseSchema::cleanUp() {

    delete fSchemaInfoList;
    delete fCurrentTypeNameStack;
    delete fCurrentGroupStack;

    for(unsigned int i=0; i < ENUM_ELT_SIZE; i++)
        delete fGlobalDeclarations[i];

    fMemoryManager->deallocate(fGlobalDeclarations);//delete [] fGlobalDeclarations;

    delete fNotationRegistry;
    delete fRedefineComponents;
    delete fIdentityConstraintNames;
    delete fDeclStack;
    delete fIC_ElementsNS;
    delete fIC_NamespaceDepthNS;
    delete fIC_NodeListNS;
    delete fPreprocessedNodes;
    delete fLocator;
    delete fParser;
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file TraverseSchema.cpp
  */

