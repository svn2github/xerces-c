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
 * Revision 1.2  2000/08/07 18:21:27  jpolast
 * change SAX_EXPORT module to SAX2_EXPORT
 *
 * Revision 1.1  2000/08/02 18:02:34  jpolast
 * initial checkin of sax2 implementation
 * submitted by Simon Fell (simon@fell.com)
 * and Joe Polastre (jpolast@apache.org)
 *
 *
 */


#ifndef DEFAULTHANDLER_HPP
#define DEFAULTHANDLER_HPP

#include <sax2/ContentHandler.hpp>
#include <sax/DTDHandler.hpp>
#include <sax/EntityResolver.hpp>
#include <sax/ErrorHandler.hpp>
#include <sax/SAXParseException.hpp>

class Locator;
class Attributes;

/**
  * Default base class for SAX2 handlers.
  *
  * <p>This class implements the default behaviour for four SAX2
  * interfaces: EntityResolver, DTDHandler, Sax2DocumentHandler,
  * and ErrorHandler.</p>
  *
  * <p>Application writers can extend this class when they need to
  * implement only part of an interface; parser writers can
  * instantiate this class to provide default handlers when the
  * application has not supplied its own.</p>
  *
  * <p>Note that the use of this class is optional.</p>
  *
  * @see EntityResolver#EntityResolver
  * @see DTDHandler#DTDHandler
  * @see ContentHandler#ContentHandler
  * @see ErrorHandler#ErrorHandler
  */

class SAX2_EXPORT DefaultHandler :

    public EntityResolver, 
	public DTDHandler, 
	public ContentHandler, 
	public ErrorHandler
{
public:
  /** @name Default handlers for the DocumentHandler interface */
    //@{
  /**
    * Receive notification of character data inside an element.
    *
    * <p>By default, do nothing.  Application writers may override this
    * method to take specific actions for each chunk of character data
    * (such as adding the data to a node or buffer, or printing it to
    * a file).</p>
    *
    * @param chars The characters.
    * @param length The number of characters to use from the
    *               character array.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see DocumentHandler#characters
    */
    virtual void characters
    (
        const   XMLCh* const    chars
        , const unsigned int    length
    );

  /**
    * Receive notification of the end of the document.
    *
    * <p>By default, do nothing.  Application writers may override this
    * method in a subclass to take specific actions at the beginning
    * of a document (such as finalising a tree or closing an output
    * file).</p>
    *
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see DocumentHandler#endDocument
    */
    virtual void endDocument();

  /**
    * Receive notification of the end of an element.
    *
    * <p>By default, do nothing.  Application writers may override this
    * method in a subclass to take specific actions at the end of
    * each element (such as finalising a tree node or writing
    * output to a file).</p>
    *
    * @param uri The URI of the asscioated namespace for this element
	* @param localname The local part of the element name
	* @param qname The QName of this element
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see DocumentHandler#endElement
    */
    virtual void endElement
	(
		const XMLCh* const uri, 
		const XMLCh* const localname, 
		const XMLCh* const qname
	);

  /**
    * Receive notification of ignorable whitespace in element content.
    *
    * <p>By default, do nothing.  Application writers may override this
    * method to take specific actions for each chunk of ignorable
    * whitespace (such as adding data to a node or buffer, or printing
    * it to a file).</p>
    *
    * @param chars The whitespace characters.
    * @param length The number of characters to use from the
    *               character array.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see DocumentHandler#ignorableWhitespace
    */
    virtual void ignorableWhitespace
    (
        const   XMLCh* const    chars
        , const unsigned int    length
    );

  /**
    * Receive notification of a processing instruction.
    *
    * <p>By default, do nothing.  Application writers may override this
    * method in a subclass to take specific actions for each
    * processing instruction, such as setting status variables or
    * invoking other methods.</p>
    *
    * @param target The processing instruction target.
    * @param data The processing instruction data, or null if
    *             none is supplied.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see DocumentHandler#processingInstruction
    */
    virtual void processingInstruction
    (
        const   XMLCh* const    target
        , const XMLCh* const    data
    );

    /**
    * Reset the Docuemnt object on its reuse
    *
    * @see DocumentHandler#resetDocument
    */
    virtual void resetDocument();
    //@}

    /** @name Default implementation of DocumentHandler interface */

    //@{
  /**
    * Receive a Locator object for document events.
    *
    * <p>By default, do nothing.  Application writers may override this
    * method in a subclass if they wish to store the locator for use
    * with other document events.</p>
    *
    * @param locator A locator for all SAX document events.
    * @see DocumentHandler#setDocumentLocator
    * @see Locator
    */
    virtual void setDocumentLocator(const Locator* const locator);

  /**
    * Receive notification of the beginning of the document.
    *
    * <p>By default, do nothing.  Application writers may override this
    * method in a subclass to take specific actions at the beginning
    * of a document (such as allocating the root node of a tree or
    * creating an output file).</p>
    *
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see DocumentHandler#startDocument
    */
    virtual void startDocument();

  /**
    * Receive notification of the start of an element.
    *
    * <p>By default, do nothing.  Application writers may override this
    * method in a subclass to take specific actions at the start of
    * each element (such as allocating a new tree node or writing
    * output to a file).</p>
    *
    * @param uri The URI of the asscioated namespace for this element
	* @param localname the local part of the element name
	* @param qname the QName of this element
    * @param attributes The specified or defaulted attributes.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see DocumentHandler#startElement
    */
    virtual void startElement
    (
        const   XMLCh* const    uri,
        const   XMLCh* const    localname,
        const   XMLCh* const    qname
        ,       Attributes&		attrs
    );

  /**
    * Receive notification of the start of an namespace prefix mapping.
    *
    * <p>By default, do nothing.  Application writers may override this
    * method in a subclass to take specific actions at the start of
    * each namespace prefix mapping.</p>
    *
	* NOTE : The current Implementation of SAX2Parser will _NEVER_ call this
	*
    * @param prefix The namespace prefix used
    * @param uri The namespace URI used.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see DocumentHandler#startPrefixMapping
    */
	virtual void startPrefixMapping
	( 
		const	XMLCh* const	prefix,
		const	XMLCh* const	uri
	) ;

  /**
    * Receive notification of the end of an namespace prefix mapping.
    *
    * <p>By default, do nothing.  Application writers may override this
    * method in a subclass to take specific actions at the end of
    * each namespace prefix mapping.</p>
    *
	* NOTE : The current Implementation of SAX2Parser will _NEVER_ call this
	*
    * @param prefix The namespace prefix used
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see DocumentHandler#endPrefixMapping
    */
	virtual void endPrefixMapping
	(
		const	XMLCh* const	prefix
	) ;

  /**
    * Receive notification of a skipped entity
    *
    * <p>The parser will invoke this method once for each entity
	* skipped.  All processors may skip external entities,
	* depending on the values of the features:<br>
	* http://xml.org/sax/features/external-general-entities<br>
	* http://xml.org/sax/features/external-parameter-entities</p>
    *
	* <p>Introduced with SAX2</p>
	*
    * @param name The name of the skipped entity.  If it is a parameter entity, 
	* the name will begin with %, and if it is the external DTD subset, 
	* it will be the string [dtd].
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    */
	virtual void skippedEntity
	(
		const	XMLCh* const	name
	) ;

    //@}

    /** @name Default implementation of the EntityResolver interface. */

    //@{
  /**
    * Resolve an external entity.
    *
    * <p>Always return null, so that the parser will use the system
    * identifier provided in the XML document.  This method implements
    * the SAX default behaviour: application writers can override it
    * in a subclass to do special translations such as catalog lookups
    * or URI redirection.</p>
    *
    * @param publicId The public identifer, or null if none is
    *                 available.
    * @param systemId The system identifier provided in the XML 
    *                 document.
    * @return The new input source, or null to require the
    *         default behaviour.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see EntityResolver#resolveEntity
    */
    virtual InputSource* resolveEntity
    (
        const   XMLCh* const    publicId
        , const XMLCh* const    systemId
    );

    //@}

    /** @name Default implementation of the ErrorHandler interface */
    //@{
   /**
    * Receive notification of a recoverable parser error.
    *
    * <p>The default implementation does nothing.  Application writers
    * may override this method in a subclass to take specific actions
    * for each error, such as inserting the message in a log file or
    * printing it to the console.</p>
    *
    * @param exception The warning information encoded as an exception.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see ErrorHandler#warning
    * @see SAXParseException#SAXParseException
    */
    virtual void error(const SAXParseException& exception);

  /**
    * Report a fatal XML parsing error.
    *
    * <p>The default implementation throws a SAXParseException.
    * Application writers may override this method in a subclass if
    * they need to take specific actions for each fatal error (such as
    * collecting all of the errors into a single report): in any case,
    * the application must stop all regular processing when this
    * method is invoked, since the document is no longer reliable, and
    * the parser may no longer report parsing events.</p>
    *
    * @param e The error information encoded as an exception.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see ErrorHandler#fatalError
    * @see SAXParseException#SAXParseException
    */
    virtual void fatalError(const SAXParseException& exception);

  /**
    * Receive notification of a parser warning.
    *
    * <p>The default implementation does nothing.  Application writers
    * may override this method in a subclass to take specific actions
    * for each warning, such as inserting the message in a log file or
    * printing it to the console.</p>
    *
    * @param e The warning information encoded as an exception.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see ErrorHandler#warning
    * @see SAXParseException#SAXParseException
    */
    virtual void warning(const SAXParseException& exception);

    /**
    * Reset the Error handler object on its reuse
    *
    * @see ErrorHandler#resetErrors
    */
    virtual void resetErrors();

    //@}


    /** @name Default implementation of DTDHandler interface. */
    //@{

  /**
    * Receive notification of a notation declaration.
    *
    * <p>By default, do nothing.  Application writers may override this
    * method in a subclass if they wish to keep track of the notations
    * declared in a document.</p>
    *
    * @param name The notation name.
    * @param publicId The notation public identifier, or null if not
    *                 available.
    * @param systemId The notation system identifier.
    * @see DTDHandler#notationDecl
    */
    virtual void notationDecl
    (
        const   XMLCh* const    name
        , const XMLCh* const    publicId
        , const XMLCh* const    systemId
    );

    /**
    * Reset the DTD object on its reuse
    *
    * @see DTDHandler#resetDocType
    */
    virtual void resetDocType();

  /**
    * Receive notification of an unparsed entity declaration.
    *
    * <p>By default, do nothing.  Application writers may override this
    * method in a subclass to keep track of the unparsed entities
    * declared in a document.</p>
    *
    * @param name The entity name.
    * @param publicId The entity public identifier, or null if not
    *                 available.
    * @param systemId The entity system identifier.
    * @param notationName The name of the associated notation.
    * @see DTDHandler#unparsedEntityDecl
    */
    virtual void unparsedEntityDecl
    (
        const   XMLCh* const    name
        , const XMLCh* const    publicId
        , const XMLCh* const    systemId
        , const XMLCh* const    notationName
    );
    //@}
};


// ---------------------------------------------------------------------------
//  HandlerBase: Inline default implementations
// ---------------------------------------------------------------------------
inline void DefaultHandler::characters(const   XMLCh* const    chars
                                       ,const   unsigned int    length)
{
}

inline void DefaultHandler::endDocument()
{
}

inline void DefaultHandler::endElement(const	XMLCh* const uri
										, const XMLCh* const localname
										, const XMLCh* const qname)
{
}

inline void DefaultHandler::error(const SAXParseException& exception)
{
}

inline void DefaultHandler::fatalError(const SAXParseException& exception)
{
    throw exception;
}

inline void
DefaultHandler::ignorableWhitespace( const   XMLCh* const	chars
                                    , const unsigned int	length)
{
}

inline void DefaultHandler::notationDecl(  const   XMLCh* const name
											, const XMLCh* const publicId
											, const XMLCh* const systemId)
{
}

inline void
DefaultHandler::processingInstruction( const   XMLCh* const target
										, const XMLCh* const data)
{
}

inline void DefaultHandler::resetErrors()
{
}

inline void DefaultHandler::resetDocument()
{
}

inline void DefaultHandler::resetDocType()
{
}

inline InputSource*
DefaultHandler::resolveEntity( const   XMLCh* const publicId
								, const XMLCh* const systemId)
{
    return 0;
}

inline void
DefaultHandler::unparsedEntityDecl(const   XMLCh* const name
									, const XMLCh* const publicId
									, const XMLCh* const systemId
									, const XMLCh* const notationName)
{
}

inline void DefaultHandler::setDocumentLocator(const Locator* const locator)
{
}

inline void DefaultHandler::startDocument()
{
}

inline void
DefaultHandler::startElement(  const     XMLCh* const  uri
								, const   XMLCh* const  localname
								, const   XMLCh* const  qname
								,       Attributes&		attrs
)
{
}

inline void DefaultHandler::warning(const SAXParseException& exception)
{
}

inline void DefaultHandler::startPrefixMapping ( const	XMLCh* const	prefix
												,const	XMLCh* const	uri)
{
}

inline void DefaultHandler::endPrefixMapping ( const	XMLCh* const	prefix )
{
}

inline void DefaultHandler::skippedEntity ( const	XMLCh* const	name )
{
}

#endif // ! DEFAULTHANDLER_HPP
