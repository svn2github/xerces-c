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
 * Revision 1.9  2001/03/21 21:56:08  tng
 * Schema: Add Schema Grammar, Schema Validator, and split the DTDValidator into DTDValidator, DTDScanner, and DTDGrammar.
 *
 * Revision 1.8  2001/02/15 15:56:29  tng
 * Schema: Add setSchemaValidation and getSchemaValidation for DOMParser and SAXParser.
 * Add feature "http://apache.org/xml/features/validation/schema" for SAX2XMLReader.
 * New data field  fSchemaValidation in XMLScanner as the flag.
 *
 * Revision 1.7  2001/01/15 21:26:33  tng
 * Performance Patches by David Bertoni.
 *
 * Details: (see xerces-c-dev mailing Jan 14)
 * XMLRecognizer.cpp: the internal encoding string XMLUni::fgXMLChEncodingString
 * was going through this function numerous times.  As a result, the top hot-spot
 * for the parse was _wcsicmp().  The real problem is that the Microsofts wide string
 * functions are unbelievably slow.  For things like encodings, it might be
 * better to use a special comparison function that only considers a-z and
 * A-Z as characters with case.  This works since the character set for
 * encodings is limit to printable ASCII characters.
 *
 *  XMLScanner2.cpp: This also has some case-sensitive vs. insensitive compares.
 * They are also much faster.  The other tweak is to only make a copy of an attribute
 * string if it needs to be split.  And then, the strategy is to try to use a
 * stack-based buffer, rather than a dynamically-allocated one.
 *
 * SAX2XMLReaderImpl.cpp: Again, more case-sensitive vs. insensitive comparisons.
 *
 * KVStringPair.cpp & hpp: By storing the size of the allocation, the storage can
 * likely be re-used many times, cutting down on dynamic memory allocations.
 *
 * XMLString.hpp: a more efficient implementation of stringLen().
 *
 * DTDValidator.cpp: another case of using a stack-based buffer when possible
 *
 * These patches made a big difference in parse time in some of our test
 * files, especially the ones are very attribute-heavy.
 *
 * Revision 1.6  2000/12/22 20:41:52  tng
 * XMLUni::fgEmptyString which is defined as "EMPTY" is incorrectly used as an empty string; in fact XMLUni::fgZeroLenString should be used instead
 *
 * Revision 1.5  2000/12/22 15:16:51  tng
 * SAX2-ext's LexicalHandler support added by David Bertoni.
 *
 * Revision 1.4  2000/08/09 23:39:58  jpolast
 * should be namespace-prefixes; not namespaces-prefixes
 *
 * Revision 1.3  2000/08/09 22:16:12  jpolast
 * many conformance & stability changes:
 *   - ContentHandler::resetDocument() removed
 *   - attrs param of ContentHandler::startDocument() made const
 *   - SAXExceptions thrown now have msgs
 *   - removed duplicate function signatures that had 'const'
 *       [ eg: getContentHander() ]
 *   - changed getFeature and getProperty to apply to const objs
 *   - setProperty now takes a void* instead of const void*
 *   - SAX2XMLReaderImpl does not inherit from SAXParser anymore
 *   - Reuse Validator (http://apache.org/xml/features/reuse-validator) implemented
 *   - Features & Properties now read-only during parse
 *
 * Revision 1.2  2000/08/07 22:53:44  jpolast
 * fixes for when 'namespaces' feature is turned off:
 *   * namespaces-prefixes only used when namespaces is on
 *   * URIs not looked up when namespaces is off, blank string instead
 *   * default validation scheme is validation on, auto-validation off.
 *
 * Revision 1.1  2000/08/02 18:04:41  jpolast
 * initial checkin of sax2 implemenation
 * submitted by Simon Fell (simon@fell.com)
 * and Joe Polastre (jpolast@apache.org)
 *
 *
 */

#include <util/IOException.hpp>
#include <util/XMLChTranscoder.hpp>
#include <util/RefStackOf.hpp>
#include <util/XMLUniDefs.hpp>
#include <sax2/ContentHandler.hpp>
#include <sax2/LexicalHandler.hpp>
#include <sax/DTDHandler.hpp>
#include <sax/ErrorHandler.hpp>
#include <sax/EntityResolver.hpp>
#include <sax/SAXParseException.hpp>
#include <sax/SAXException.hpp>
#include <internal/XMLScanner.hpp>
#include <parsers/SAX2XMLReaderImpl.hpp>
#include <string.h>

// SAX2 Core: http://xml.org/sax/features/validation
const XMLCh SAX2XMLReaderImpl::SAX_CORE_VALIDATION[] = {
		chLatin_h, chLatin_t, chLatin_t, chLatin_p,
		chColon, chForwardSlash, chForwardSlash,
		chLatin_x, chLatin_m, chLatin_l, chPeriod,
		chLatin_o, chLatin_r, chLatin_g, chForwardSlash,
		chLatin_s, chLatin_a, chLatin_x, chForwardSlash,
		chLatin_f, chLatin_e, chLatin_a,
		chLatin_t, chLatin_u, chLatin_r,
		chLatin_e, chLatin_s, chForwardSlash,
		chLatin_v, chLatin_a, chLatin_l,
		chLatin_i, chLatin_d, chLatin_a, chLatin_t,
		chLatin_i, chLatin_o, chLatin_n, chNull
};

// SAX2 Core: http://xml.org/sax/features/namespaces
const XMLCh SAX2XMLReaderImpl::SAX_CORE_NAMESPACES[] = {
		chLatin_h, chLatin_t, chLatin_t, chLatin_p,
		chColon, chForwardSlash, chForwardSlash,
		chLatin_x, chLatin_m, chLatin_l, chPeriod,
		chLatin_o, chLatin_r, chLatin_g, chForwardSlash,
		chLatin_s, chLatin_a, chLatin_x, chForwardSlash,
		chLatin_f, chLatin_e, chLatin_a,
		chLatin_t, chLatin_u, chLatin_r,
		chLatin_e, chLatin_s, chForwardSlash,
		chLatin_n, chLatin_a, chLatin_m,
		chLatin_e, chLatin_s, chLatin_p, chLatin_a,
		chLatin_c, chLatin_e, chLatin_s, chNull
};

//SAX2 Core: http://xml.org/sax/features/namespace-prefixes
const XMLCh SAX2XMLReaderImpl::SAX_CORE_NAMESPACES_PREFIXES[] = {
		chLatin_h, chLatin_t, chLatin_t, chLatin_p,
		chColon, chForwardSlash, chForwardSlash,
		chLatin_x, chLatin_m, chLatin_l, chPeriod,
		chLatin_o, chLatin_r, chLatin_g, chForwardSlash,
		chLatin_s, chLatin_a, chLatin_x, chForwardSlash,
		chLatin_f, chLatin_e, chLatin_a,
		chLatin_t, chLatin_u, chLatin_r,
		chLatin_e, chLatin_s, chForwardSlash,
		chLatin_n, chLatin_a, chLatin_m,
		chLatin_e, chLatin_s, chLatin_p, chLatin_a,
		chLatin_c, chLatin_e,
		chDash, chLatin_p, chLatin_r, chLatin_e,
		chLatin_f, chLatin_i, chLatin_x,
		chLatin_e, chLatin_s, chNull
};

//Xerces: http://apache.org/xml/features/validation/dynamic
const XMLCh SAX2XMLReaderImpl::SAX_XERCES_DYNAMIC[] = {
		chLatin_h, chLatin_t, chLatin_t, chLatin_p,
		chColon, chForwardSlash, chForwardSlash,
		chLatin_a, chLatin_p, chLatin_a, chLatin_c, chLatin_h, chLatin_e, chPeriod,
		chLatin_o, chLatin_r, chLatin_g, chForwardSlash,
		chLatin_x, chLatin_m, chLatin_l, chForwardSlash,
		chLatin_f, chLatin_e, chLatin_a,
		chLatin_t, chLatin_u, chLatin_r,
		chLatin_e, chLatin_s, chForwardSlash,
		chLatin_v, chLatin_a, chLatin_l,
		chLatin_i, chLatin_d, chLatin_a, chLatin_t,
		chLatin_i, chLatin_o, chLatin_n, chForwardSlash,
		chLatin_d, chLatin_y, chLatin_n, chLatin_a,
		chLatin_m, chLatin_i, chLatin_c, chNull
};

//Xerces: http://apache.org/xml/features/validation/reuse-grammar
const XMLCh SAX2XMLReaderImpl::SAX_XERCES_REUSEGRAMMAR[] = {
		chLatin_h, chLatin_t, chLatin_t, chLatin_p,
		chColon, chForwardSlash, chForwardSlash,
		chLatin_a, chLatin_p, chLatin_a, chLatin_c, chLatin_h, chLatin_e, chPeriod,
		chLatin_o, chLatin_r, chLatin_g, chForwardSlash,
		chLatin_x, chLatin_m, chLatin_l, chForwardSlash,
		chLatin_f, chLatin_e, chLatin_a,
		chLatin_t, chLatin_u, chLatin_r,
		chLatin_e, chLatin_s, chForwardSlash,
		chLatin_v, chLatin_a, chLatin_l,
		chLatin_i, chLatin_d, chLatin_a, chLatin_t,
		chLatin_i, chLatin_o, chLatin_n, chForwardSlash,
		chLatin_r, chLatin_e, chLatin_u, chLatin_s, chLatin_e, chDash,
      chLatin_g, chLatin_r, chLatin_a,	chLatin_m, chLatin_m,
      chLatin_a, chLatin_r, chNull
};

//Xerces: http://apache.org/xml/features/validation/schema
const XMLCh SAX2XMLReaderImpl::SAX_XERCES_SCHEMA[] = {
		chLatin_h, chLatin_t, chLatin_t, chLatin_p,
		chColon, chForwardSlash, chForwardSlash,
		chLatin_a, chLatin_p, chLatin_a, chLatin_c, chLatin_h, chLatin_e, chPeriod,
		chLatin_o, chLatin_r, chLatin_g, chForwardSlash,
		chLatin_x, chLatin_m, chLatin_l, chForwardSlash,
		chLatin_f, chLatin_e, chLatin_a,
		chLatin_t, chLatin_u, chLatin_r,
		chLatin_e, chLatin_s, chForwardSlash,
		chLatin_v, chLatin_a, chLatin_l,
		chLatin_i, chLatin_d, chLatin_a, chLatin_t,
		chLatin_i, chLatin_o, chLatin_n, chForwardSlash,
		chLatin_s, chLatin_c, chLatin_h, chLatin_e,
		chLatin_m, chLatin_a, chNull
};

//deprecated
//Xerces: http://apache.org/xml/features/validation/reuse-validator
const XMLCh SAX2XMLReaderImpl::SAX_XERCES_REUSEVALIDATOR[] = {
		chLatin_h, chLatin_t, chLatin_t, chLatin_p,
		chColon, chForwardSlash, chForwardSlash,
		chLatin_a, chLatin_p, chLatin_a, chLatin_c, chLatin_h, chLatin_e, chPeriod,
		chLatin_o, chLatin_r, chLatin_g, chForwardSlash,
		chLatin_x, chLatin_m, chLatin_l, chForwardSlash,
		chLatin_f, chLatin_e, chLatin_a,
		chLatin_t, chLatin_u, chLatin_r,
		chLatin_e, chLatin_s, chForwardSlash,
		chLatin_v, chLatin_a, chLatin_l,
		chLatin_i, chLatin_d, chLatin_a, chLatin_t,
		chLatin_i, chLatin_o, chLatin_n, chForwardSlash,
		chLatin_r, chLatin_e, chLatin_u, chLatin_s,
		chLatin_e, chDash, chLatin_v,
		chLatin_a, chLatin_l,
		chLatin_i, chLatin_d, chLatin_a, chLatin_t,
		chLatin_o, chLatin_r, chNull
};

SAX2XMLReaderImpl::SAX2XMLReaderImpl() :
    fDocHandler(0)
    , fDTDHandler(0)
    , fElemDepth(0)
    , fEntityResolver(0)
    , fErrorHandler(0)
    , fLexicalHandler(0)
    , fAdvDHCount(0)
    , fAdvDHList(0)
    , fAdvDHListSize(32)
    , fParseInProgress(false)
    , fScanner(0)
{
    //
    //  Create a scanner and tell it what validator to use. Then set us
    //  as the document event handler so we can fill the DOM document.
    //
    fScanner = new XMLScanner(0);

    // Create the initial advanced handler list array and zero it out
    fAdvDHList = new XMLDocumentHandler*[fAdvDHListSize];
    memset(fAdvDHList, 0, sizeof(void*) * fAdvDHListSize);
	
	// SAX2 default is for namespaces (feature http://xml.org/sax/features/namespaces) to be on
	setDoNamespaces(true) ;

	// SAX2 default is for namespaces-prefixes to be on
	fnamespacePrefix = true;

	// SAX2 default: validation on, auto-validation off
	fValidation = true;
	fautoValidation = false;

	// default: reuseValidator is off
	fReuseGrammar = false;

	// default: schema is on
	setSchemaValidation(true);
	
	fPrefixes    = new RefStackOf<XMLBuffer> (10, false) ;
	tempAttrVec  = new RefVectorOf<XMLAttr>  (10, false) ;
	prefixCounts = new ValueStackOf<unsigned int>(10) ;
}

SAX2XMLReaderImpl::~SAX2XMLReaderImpl()
{
    delete [] fAdvDHList;
    delete fScanner;
    delete fPrefixes;
    delete tempAttrVec;
    delete prefixCounts ;
}


// ---------------------------------------------------------------------------
//  SAX2XMLReaderImpl Validator functions
// ---------------------------------------------------------------------------
void SAX2XMLReaderImpl::setValidator(XMLValidator* valueToAdopt)
{
	fScanner->setValidator(valueToAdopt);
}

XMLValidator* SAX2XMLReaderImpl::getValidator() const
{
	return fScanner->getValidator();
}

// ---------------------------------------------------------------------------
//  SAX2XMLReader Interface
// ---------------------------------------------------------------------------
void SAX2XMLReaderImpl::setContentHandler(ContentHandler* const handler)
{
	fDocHandler = handler;
    if (fDocHandler)
    {
        //
        //  Make sure we are set as the document handler with the scanner.
        //  We may already be (if advanced handlers are installed), but its
        //  not worthing checking, just do it.
        //
        fScanner->setDocHandler(this);
    }
     else
    {
        //
        //  If we don't have any advanced handlers either, then deinstall us
        //  from the scanner because we don't need document events anymore.
        //
        if (!fAdvDHCount)
            fScanner->setDocHandler(0);
    }

}

void SAX2XMLReaderImpl::setDTDHandler(DTDHandler* const handler)
{
    fDTDHandler = handler;
    if (fDTDHandler)
        fScanner->setDocTypeHandler(this);
    else
        fScanner->setDocTypeHandler(0);
}


void SAX2XMLReaderImpl::setErrorHandler(ErrorHandler* const handler)
{
    //
    //  Store the handler. Then either install or deinstall us as the
    //  error reporter on the scanner.
    //
    fErrorHandler = handler;
    if (fErrorHandler)
        fScanner->setErrorReporter(this);
     else
        fScanner->setErrorReporter(0);
}


void SAX2XMLReaderImpl::setLexicalHandler(LexicalHandler* const handler)
{
    fLexicalHandler = handler;
}


void SAX2XMLReaderImpl::setEntityResolver(EntityResolver* const resolver)
{
    fEntityResolver = resolver;
    if (fEntityResolver)
        fScanner->setEntityHandler(this);
    else
        fScanner->setEntityHandler(0);
}

void SAX2XMLReaderImpl::parse (const   InputSource&    source)
{
    // Avoid multiple entrance
    if (fParseInProgress)
        ThrowXML(IOException, XMLExcepts::Gen_ParseInProgress);

    try
    {
        fParseInProgress = true;
        fScanner->scanDocument(source, fReuseGrammar);
        fParseInProgress = false;
    }

    catch (...)
    {
        fParseInProgress = false;
        throw;
    }
}

void SAX2XMLReaderImpl::parse (const   XMLCh* const    systemId)
{
    // Avoid multiple entrance
    if (fParseInProgress)
        ThrowXML(IOException, XMLExcepts::Gen_ParseInProgress);

    try
    {
        fParseInProgress = true;
        fScanner->scanDocument(systemId, fReuseGrammar);
        fParseInProgress = false;
    }

    catch (...)
    {
        fParseInProgress = false;
        throw;
    }
}

void SAX2XMLReaderImpl::parse (const   char* const     systemId)
{
    // Avoid multiple entrance
    if (fParseInProgress)
        ThrowXML(IOException, XMLExcepts::Gen_ParseInProgress);

    try
    {
        fParseInProgress = true;
        fScanner->scanDocument(systemId, fReuseGrammar);
        fParseInProgress = false;
    }

    catch (...)
    {
        fParseInProgress = false;
        throw;
    }
}

// ---------------------------------------------------------------------------
//  SAX2XMLReaderImpl: Overrides of the XMLDocumentHandler interface
// ---------------------------------------------------------------------------
void SAX2XMLReaderImpl::docCharacters(  const   XMLCh* const    chars
                                , const unsigned int    length
                                , const bool            cdataSection)
{
    // Suppress the chars before the root element.
    if (!fElemDepth)
        return;

   // Call the installed LexicalHandler.
   if (cdataSection && fLexicalHandler)
        fLexicalHandler->startCDATA();

    // Just map to the SAX document handler
    if (fDocHandler)
        fDocHandler->characters(chars, length);

   // Call the installed LexicalHandler.
   if (cdataSection && fLexicalHandler)
        fLexicalHandler->endCDATA();

    //
    //  If there are any installed advanced handlers, then lets call them
    //  with this info.
    //
    for (unsigned int index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->docCharacters(chars, length, cdataSection);
}


void SAX2XMLReaderImpl::docComment(const XMLCh* const commentText)
{
   // Call the installed LexicalHandler.
   if (fLexicalHandler)
   {
        // SAX2 reports comment text like characters -- as an
        // array with a length.
        fLexicalHandler->comment(commentText, XMLString::stringLen(commentText));
   }

    // Suppress passing through any comments before the root element.
    if (!fElemDepth)
        return;

    //
    //  OK, if there are any installed advanced handlers,
   // then let's call them with this info.
    //
    for (unsigned int index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->docComment(commentText);
}


void SAX2XMLReaderImpl::XMLDecl(const   XMLCh* const
                        , const XMLCh* const
                        , const XMLCh* const
                        , const XMLCh* const)
{
    // SAX has no way to report this event
}


void SAX2XMLReaderImpl::docPI(  const   XMLCh* const    target
                        , const XMLCh* const    data)
{
    // Just map to the SAX document handler
    if (fDocHandler)
        fDocHandler->processingInstruction(target, data);

    //
    //  If there are any installed advanced handlers, then lets call them
    //  with this info.
    //
    for (unsigned int index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->docPI(target, data);
}


void SAX2XMLReaderImpl::endDocument()
{
    if (fDocHandler)
        fDocHandler->endDocument();

    //
    //  If there are any installed advanced handlers, then lets call them
    //  with this info.
    //
    for (unsigned int index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->endDocument();
}


void SAX2XMLReaderImpl::endEntityReference(const XMLEntityDecl& entityDecl)
{
   // Call the installed LexicalHandler.
   if (fLexicalHandler)
        fLexicalHandler->endEntity(entityDecl.getName());

    //
    //  SAX has no way to report this event. But, if there are any installed
    //  advanced handlers, then lets call them with this info.
    //
    for (unsigned int index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->endEntityReference(entityDecl);
}


void SAX2XMLReaderImpl::ignorableWhitespace(const   XMLCh* const    chars
                                    , const unsigned int    length
                                    , const bool            cdataSection)
{
    // Do not report the whitespace before the root element.
    if (!fElemDepth)
        return;

    // Just map to the SAX document handler
    if (fDocHandler)
        fDocHandler->ignorableWhitespace(chars, length);

    //
    //  If there are any installed advanced handlers, then lets call them
    //  with this info.
    //
    for (unsigned int index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->ignorableWhitespace(chars, length, cdataSection);
}


void SAX2XMLReaderImpl::resetDocument()
{
    //
    //  If there are any installed advanced handlers, then lets call them
    //  with this info.
    //
    for (unsigned int index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->resetDocument();

    // Make sure our element depth flag gets set back to zero
    fElemDepth = 0;
}


void SAX2XMLReaderImpl::startDocument()
{
    // Just map to the SAX document handler
    if (fDocHandler)
    {
        fDocHandler->setDocumentLocator(fScanner->getLocator());
        fDocHandler->startDocument();
    }

    //
    //  If there are any installed advanced handlers, then lets call them
    //  with this info.
    //
    for (unsigned int index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->startDocument();
}


void SAX2XMLReaderImpl::
startElement(   const   XMLElementDecl&         elemDecl
                , const unsigned int            elemURLId
                , const XMLCh* const            elemPrefix
                , const RefVectorOf<XMLAttr>&   attrList
                , const unsigned int            attrCount
                , const bool                    isEmpty
                , const bool                    isRoot)
{

	// Bump the element depth counter if not empty
    if (!isEmpty)
        fElemDepth++;

    if (fDocHandler)
	{
	
		if (getDoNamespaces())
		{
            unsigned int numPrefix = 0;
			const XMLCh*   nsString = XMLUni::fgXMLNSString;
			const XMLCh*   nsPrefix = 0;
			const XMLCh*   nsURI    = 0;
			const XMLAttr* tempAttr = 0;

            if (!fnamespacePrefix)
			{
                tempAttrVec->removeAllElements();
			}

			for (unsigned int i = 0; i < attrCount; i++)
			{
				tempAttr = attrList.elementAt(i);
                  if (XMLString::compareString(tempAttr->getQName(), nsString) == 0)
					nsURI = tempAttr->getValue();
                  if (XMLString::compareString(tempAttr->getPrefix(), nsString) == 0)
				{
					nsPrefix = tempAttr->getName();
					nsURI = tempAttr->getValue();
				}
				if (!fnamespacePrefix)
				{
					if (nsURI == 0)
						tempAttrVec->addElement((XMLAttr* const)tempAttr);
				}
				if (nsURI != 0)
				{
					if (nsPrefix == 0)
						nsPrefix = XMLUni::fgZeroLenString;
					fDocHandler->startPrefixMapping(nsPrefix, nsURI);
					
					XMLBuffer &buf = fStringBuffers.bidOnBuffer()  ;
					buf.set ( nsPrefix ) ;
					fPrefixes->push(&buf) ;

					numPrefix++;
				}
				nsURI = 0;
				nsPrefix = 0;	
			}
			prefixCounts->push(numPrefix) ;
			if (!fnamespacePrefix)
				fAttrList.setVector(tempAttrVec, tempAttrVec->size(), fScanner);
			else
                fAttrList.setVector(&attrList, attrCount, fScanner);

			// call startElement() with namespace declarations
			XMLBufBid URIBufferBid ( &fStringBuffers ) ;
			XMLBuffer &URIBuffer = URIBufferBid.getBuffer() ;

			fScanner->getURIText(elemURLId, (XMLBuffer &)URIBuffer);
			
			fDocHandler->startElement(URIBuffer.getRawBuffer(),
										elemDecl.getBaseName(),
										elemDecl.getFullName(),
										fAttrList);
		}
		else
		{
			fAttrList.setVector(&attrList, attrCount, fScanner);
			fDocHandler->startElement(XMLUni::fgZeroLenString,
										elemDecl.getBaseName(),
										elemDecl.getFullName(),
										fAttrList);
		}


        // If its empty, send the end tag event now
        if (isEmpty)
		{

			// call endPrefixMapping appropriately.
			if (getDoNamespaces())
			{
				XMLBufBid URIBufferBid ( &fStringBuffers ) ;
				XMLBuffer &URIBuffer = URIBufferBid.getBuffer() ;

				fScanner->getURIText(elemURLId, (XMLBuffer &)URIBuffer);
			
				fDocHandler->endElement(	URIBuffer.getRawBuffer(),
											elemDecl.getBaseName(),
											elemDecl.getFullName() );
				unsigned int numPrefix = prefixCounts->pop();
				for (unsigned int i = 0; i < numPrefix; ++i)
				{
					XMLBuffer * buf = fPrefixes->pop() ;
					fDocHandler->endPrefixMapping( buf->getRawBuffer() );
					fStringBuffers.releaseBuffer(*buf) ;
				}
			}
			else
			{
				fDocHandler->endElement(XMLUni::fgZeroLenString,
							elemDecl.getBaseName(),
							elemDecl.getFullName() );

			}

		}
    }

    //
    //  If there are any installed advanced handlers, then lets call them
    //  with this info.
    //
    for (unsigned int index = 0; index < fAdvDHCount; index++)
    {
        fAdvDHList[index]->startElement
        (
            elemDecl
            , elemURLId
            , elemPrefix
            , attrList
            , attrCount
            , isEmpty
            , isRoot
        );
    }
}

void SAX2XMLReaderImpl::endElement( const   XMLElementDecl& elemDecl
                            , const unsigned int    uriId
                            , const bool            isRoot)
{
    // Just map to the SAX document handler
    if (fDocHandler)
	{

		// get the prefixes back so that we can call endPrefixMapping()
        if (getDoNamespaces())
		{
			XMLBufBid URIBufferBid ( &fStringBuffers ) ;
			XMLBuffer &URIBuffer = URIBufferBid.getBuffer() ;

			fScanner->getURIText(uriId, URIBuffer ) ;
			fDocHandler->endElement(	URIBuffer.getRawBuffer(),
										elemDecl.getBaseName(),
										elemDecl.getFullName() );

			unsigned int numPrefix = prefixCounts->pop();
			for (unsigned int i = 0; i < numPrefix; i++)
			{
				XMLBuffer * buf = fPrefixes->pop() ;
				fDocHandler->endPrefixMapping( buf->getRawBuffer() );
				fStringBuffers.releaseBuffer(*buf) ;
			}
		}
		else
		{
			fDocHandler->endElement(XMLUni::fgZeroLenString,
										elemDecl.getBaseName(),
										elemDecl.getFullName() );
		}
	
	}

    //
    //  If there are any installed advanced handlers, then lets call them
    //  with this info.
    //
    for (unsigned int index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->endElement(elemDecl, uriId, isRoot);

    //
    //  Dump the element depth down again. Don't let it underflow in case
    //  of malformed XML.
    //
    if (fElemDepth)
        fElemDepth--;
}

void SAX2XMLReaderImpl::startEntityReference(const XMLEntityDecl& entityDecl)
{
   // Call the installed LexicalHandler.
   if (fLexicalHandler)
        fLexicalHandler->startEntity(entityDecl.getName());
    //
    //  SAX has no way to report this. But, If there are any installed
    //  advanced handlers, then lets call them with this info.
    //
    for (unsigned int index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->startEntityReference(entityDecl);
}

// ---------------------------------------------------------------------------
//  SAX2XMLReaderImpl: Overrides of the DocTypeHandler interface
// ---------------------------------------------------------------------------
void SAX2XMLReaderImpl::attDef( const   DTDElementDecl& elemDecl
                        , const DTDAttDef&      attDef
                        , const bool            ignoring)
{
    // Unused by SAX DTDHandler interface at this time
}


void SAX2XMLReaderImpl::doctypeComment(const XMLCh* const)
{
    // Unused by SAX DTDHandler interface at this time
}


void SAX2XMLReaderImpl::doctypeDecl(const   DTDElementDecl& elemDecl
                            , const XMLCh* const    publicId
                            , const XMLCh* const    systemId
                            , const bool            hasIntSubset)
{
   // Call the installed LexicalHandler.
   if (fLexicalHandler)
        fLexicalHandler->startDTD(elemDecl.getFullName(), publicId, systemId);

    // Unused by SAX DTDHandler interface at this time
}


void SAX2XMLReaderImpl::doctypePI(  const   XMLCh* const
                            , const XMLCh* const)
{
    // Unused by SAX DTDHandler interface at this time
}


void SAX2XMLReaderImpl::doctypeWhitespace(  const   XMLCh* const    chars
                                    , const unsigned int    length)
{
    // Unused by SAX DTDHandler interface at this time
}


void SAX2XMLReaderImpl::elementDecl(const DTDElementDecl&, const bool)
{
    // Unused by SAX DTDHandler interface at this time
}


void SAX2XMLReaderImpl::endAttList(const DTDElementDecl&)
{
    // Unused by SAX DTDHandler interface at this time
}


void SAX2XMLReaderImpl::endIntSubset()
{
    // Unused by SAX DTDHandler interface at this time
}


void SAX2XMLReaderImpl::endExtSubset()
{
   // Call the installed LexicalHandler.
   if (fLexicalHandler)
        fLexicalHandler->endDTD();

    // Unused by SAX DTDHandler interface at this time
}


void SAX2XMLReaderImpl::entityDecl( const   DTDEntityDecl&  entityDecl
                            , const bool            isPEDecl
                            , const bool            isIgnored)
{
    //
    //  If we have a DTD handler, and this entity is not ignored, and
    //  its an unparsed entity, then send this one.
    //
    if (fDTDHandler && !isIgnored)
    {
        if (entityDecl.isUnparsed())
        {
            fDTDHandler->unparsedEntityDecl
            (
                entityDecl.getName()
                , entityDecl.getPublicId()
                , entityDecl.getSystemId()
                , entityDecl.getNotationName()
            );
        }
    }
}


void SAX2XMLReaderImpl::resetDocType()
{
    // Just map to the DTD handler
    if (fDTDHandler)
        fDTDHandler->resetDocType();
}


void SAX2XMLReaderImpl::notationDecl(   const   XMLNotationDecl&    notDecl
                                , const bool                isIgnored)
{
    if (fDTDHandler && !isIgnored)
    {
        fDTDHandler->notationDecl
        (
            notDecl.getName()
            , notDecl.getPublicId()
            , notDecl.getSystemId()
        );
    }
}


void SAX2XMLReaderImpl::startAttList(const DTDElementDecl&)
{
    // Unused by SAX DTDHandler interface at this time
}


void SAX2XMLReaderImpl::startIntSubset()
{
    // Unused by SAX DTDHandler interface at this time
}


void SAX2XMLReaderImpl::startExtSubset()
{
    // Unused by SAX DTDHandler interface at this time
}


void SAX2XMLReaderImpl::TextDecl(   const  XMLCh* const
                            , const XMLCh* const)
{
    // Unused by SAX DTDHandler interface at this time
}


// ---------------------------------------------------------------------------
//  SAX2XMLReaderImpl: Handlers for the XMLEntityHandler interface
// ---------------------------------------------------------------------------
void SAX2XMLReaderImpl::endInputSource(const InputSource&)
{
}

bool SAX2XMLReaderImpl::expandSystemId(const XMLCh* const, XMLBuffer&)
{
    return false;
}


void SAX2XMLReaderImpl::resetEntities()
{
    // Nothing to do for this one
}


InputSource* SAX2XMLReaderImpl::resolveEntity(   const   XMLCh* const    publicId
                                               , const   XMLCh* const    systemId)
{
    // Just map to the SAX entity resolver handler
    if (fEntityResolver)
        return fEntityResolver->resolveEntity(publicId, systemId);
    return 0;
}


void SAX2XMLReaderImpl::startInputSource(const InputSource&)
{
    // Nothing to do for this one
}

// ---------------------------------------------------------------------------
//  SAX2XMLReaderImpl: Overrides of the XMLErrorReporter interface
// ---------------------------------------------------------------------------
void SAX2XMLReaderImpl::resetErrors()
{
    if (fErrorHandler)
        fErrorHandler->resetErrors();
}


void SAX2XMLReaderImpl::error(  const   unsigned int                code
                        , const XMLCh* const                msgDomain
                        , const XMLErrorReporter::ErrTypes  errType
                        , const XMLCh* const                errorText
                        , const XMLCh* const                systemId
                        , const XMLCh* const                publicId
                        , const unsigned int                lineNum
                        , const unsigned int                colNum)
{
    SAXParseException toThrow = SAXParseException
    (
        errorText
        , publicId
        , systemId
        , lineNum
        , colNum
    );

    if (!fErrorHandler)
    {
        if (errType == XMLErrorReporter::ErrType_Fatal)
            throw toThrow;
        else
            return;
    }

    if (errType == XMLErrorReporter::ErrType_Warning)
        fErrorHandler->warning(toThrow);
    else if (errType == XMLErrorReporter::ErrType_Fatal)
        fErrorHandler->fatalError(toThrow);
    else
        fErrorHandler->error(toThrow);
}


// ---------------------------------------------------------------------------
//  SAX2XMLReaderImpl: Features and Properties
// ---------------------------------------------------------------------------

void SAX2XMLReaderImpl::setFeature(const XMLCh* const name, const bool value)
{

	if (fParseInProgress)
		throw SAXNotSupportedException("Feature modification is not supported during parse.");
	
	if (XMLString::compareIString(name, SAX2XMLReaderImpl::SAX_CORE_NAMESPACES) == 0)
	{
		setDoNamespaces(value);
	}

	else if (XMLString::compareIString(name, SAX2XMLReaderImpl::SAX_CORE_VALIDATION) == 0)
	{
		fValidation = value;
		if (fValidation)
			if (fautoValidation)
				setValidationScheme(Val_Auto);
			else
				setValidationScheme(Val_Always);
		else
			setValidationScheme(Val_Never);
	}


	else if (XMLString::compareIString(name, SAX2XMLReaderImpl::SAX_CORE_NAMESPACES_PREFIXES) == 0)
	{
		fnamespacePrefix = value;
	}

	else if (XMLString::compareIString(name, SAX2XMLReaderImpl::SAX_XERCES_DYNAMIC) == 0)
	{
		fautoValidation = value;
		// for auto validation, the sax2 core validation feature must also be enabled.
		if (fValidation)
			if (fautoValidation)
				setValidationScheme(Val_Auto);
			else
				setValidationScheme(Val_Always);
		else
			setValidationScheme(Val_Never);
	}

	else if (XMLString::compareIString(name, SAX2XMLReaderImpl::SAX_XERCES_REUSEVALIDATOR) == 0)
	{
		fReuseGrammar = value;
	}

	else if (XMLString::compareIString(name, SAX2XMLReaderImpl::SAX_XERCES_REUSEGRAMMAR) == 0)
	{
		fReuseGrammar = value;
	}

	else if (XMLString::compareIString(name, SAX2XMLReaderImpl::SAX_XERCES_SCHEMA) == 0)
	{
		setSchemaValidation(value);
	}

   else
       throw SAXNotRecognizedException("Unknown Feature");
}

bool SAX2XMLReaderImpl::getFeature(const XMLCh* const name) const
{
	if (XMLString::compareIString(name, SAX2XMLReaderImpl::SAX_CORE_NAMESPACES) == 0)
		return getDoNamespaces();
	else if (XMLString::compareIString(name, SAX2XMLReaderImpl::SAX_CORE_VALIDATION) == 0)
		return fValidation;
	else if (XMLString::compareIString(name, SAX2XMLReaderImpl::SAX_CORE_NAMESPACES_PREFIXES) == 0)
		return fnamespacePrefix;
	else if (XMLString::compareIString(name, SAX2XMLReaderImpl::SAX_XERCES_DYNAMIC) == 0)
		return fautoValidation;
	else if (XMLString::compareIString(name, SAX2XMLReaderImpl::SAX_XERCES_REUSEVALIDATOR) == 0)
        return fReuseGrammar;
	else if (XMLString::compareIString(name, SAX2XMLReaderImpl::SAX_XERCES_REUSEGRAMMAR) == 0)
        return fReuseGrammar;
	else if (XMLString::compareIString(name, SAX2XMLReaderImpl::SAX_XERCES_SCHEMA) == 0)
        return getSchemaValidation();
   else
       throw SAXNotRecognizedException("Unknown Feature");
	return false;
}

void SAX2XMLReaderImpl::setProperty(const XMLCh* const name, void* value)
{
	if (fParseInProgress)
		throw SAXNotSupportedException("Property modification is not supported during parse.");

	throw SAXNotRecognizedException("Unknown Property");
	// unimplemented
}


void* SAX2XMLReaderImpl::getProperty(const XMLCh* const name) const
{
	throw SAXNotRecognizedException("Unknown Property");
	// unimplemented
	return 0;
}


// ---------------------------------------------------------------------------
//  SAX2XMLReaderImpl: Private getters and setters for conveniences
// ---------------------------------------------------------------------------

void SAX2XMLReaderImpl::setValidationScheme(const ValSchemes newScheme)
{
    if (newScheme == Val_Never)
        fScanner->setValidationScheme(XMLScanner::Val_Never);
    else if (newScheme == Val_Always)
        fScanner->setValidationScheme(XMLScanner::Val_Always);
    else
        fScanner->setValidationScheme(XMLScanner::Val_Auto);
}

void SAX2XMLReaderImpl::setDoNamespaces(const bool newState)
{
    fScanner->setDoNamespaces(newState);
}

bool SAX2XMLReaderImpl::getDoNamespaces() const
{
    return fScanner->getDoNamespaces();
}

void SAX2XMLReaderImpl::setSchemaValidation(const bool newState)
{
    fScanner->setSchemaValidation(newState);
}

bool SAX2XMLReaderImpl::getSchemaValidation() const
{
    return fScanner->getSchemaValidation();
}
