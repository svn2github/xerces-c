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
 * Revision 1.2  2001/05/03 18:17:30  knoaman
 * Some design changes:
 * o Changed the TokenFactory from a single static instance, to a
 *    normal class. Each RegularExpression object will have its own
 *    instance of TokenFactory, and that instance will be passed to
 *    other classes that need to use a TokenFactory to create Token
 *    objects (with the exception of RangeTokenMap).
 * o Added a new class RangeTokenMap to map a the different ranges
 *    in a given category to a specific RangeFactory object. In the old
 *    design RangeFactory had dual functionality (act as a Map, and as
 *    a factory for creating RangeToken(s)). The RangeTokenMap will
 *    have its own copy of the TokenFactory. There will be only one
 *    instance of the RangeTokenMap class, and that instance will be
 *    lazily deleted when XPlatformUtils::Terminate is called.
 *
 * Revision 1.1  2001/03/02 19:26:43  knoaman
 * Schema: Regular expression handling part II
 *
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <util/regx/ParserForXMLSchema.hpp>
#include <util/regx/TokenFactory.hpp>
#include <util/regx/RangeToken.hpp>
#include <util/regx/TokenInc.hpp>
#include <util/regx/RegxDefs.hpp>
#include <util/ParseException.hpp>

// ---------------------------------------------------------------------------
//  ParserForXMLSchema: Constructors and Destructors
// ---------------------------------------------------------------------------
ParserForXMLSchema::ParserForXMLSchema()
    : RegxParser()
{

}

ParserForXMLSchema::~ParserForXMLSchema() {

}

// ---------------------------------------------------------------------------
//  ParserForXMLSchema: Parsing/Processing methods
// ---------------------------------------------------------------------------
Token* ParserForXMLSchema::processCaret() {

	processNext();
	return getTokenFactory()->createChar(chCaret);
}

Token* ParserForXMLSchema::processDollar() {

	processNext();
	return getTokenFactory()->createChar(chDollarSign);
}

Token* ParserForXMLSchema::processPlus(Token* const tok) {

	processNext();
    return getTokenFactory()->createConcat(tok, 
		                       getTokenFactory()->createClosure(tok));
}

Token* ParserForXMLSchema::processStar(Token* const tok) {

    processNext();
    return getTokenFactory()->createClosure(tok);
}

Token* ParserForXMLSchema::processQuestion(Token* const tok) {

    processNext();

    TokenFactory* tokFactory = getTokenFactory();
    Token* retTok = tokFactory->createUnion();
	retTok->addChild(tok, tokFactory);
	retTok->addChild(tokFactory->createToken(Token::EMPTY), tokFactory);
	return retTok;
}

Token* ParserForXMLSchema::processParen() {

    processNext();
    Token* retTok = getTokenFactory()->createParenthesis(parseRegx(), 0);

    if (getState() != T_RPAREN) {
        ThrowXML(ParseException, XMLExcepts::Parser_Factor1);
    }

    processNext();
    return retTok;
}

RangeToken* ParserForXMLSchema::parseCharacterClass(const bool useNRange) {

	setParseContext(S_INBRACKETS);
    processNext();

    RangeToken* base = 0;
    RangeToken* tok = 0;
	bool isNRange = false;

	if (getState() == T_CHAR && getCharData() == chCaret) {

        isNRange = true;
        processNext();
        base = getTokenFactory()->createRange();
        base->addRange(0, Token::UTF16_MAX);
        tok = getTokenFactory()->createRange();
	}
	else {
        tok= getTokenFactory()->createRange();
	}

    int type;
    bool firstLoop = true;

    while ( (type = getState()) != T_EOF) {

        // single range | from-to-range | subtraction
        if (type == T_CHAR && getCharData() == chCloseSquare && !firstLoop) {

            if (isNRange) {

                base->subtractRanges(tok);
				tok = base;
            }
			break;
        }

        XMLInt32 ch = getCharData();
        bool     end = false;

        if (type == T_BACKSOLIDUS) {

            switch(ch) {
			case chLatin_d:
			case chLatin_D:
			case chLatin_w:
			case chLatin_W:
			case chLatin_s:
			case chLatin_S:
                {
				    tok->mergeRanges(getTokenForShorthand(ch)); 
                    end = true;
                }
                break;
            case chLatin_i:
            case chLatin_I:
            case chLatin_c:
            case chLatin_C:
                {
                    ch = processCInCharacterClass(tok, ch);
					if (ch < 0) {
					    end = true;
                    }
                }
                break;
            case chLatin_p:
            case chLatin_P:
                {
                    int start = getOffset();
                    RangeToken* tok2 = processBacksolidus_pP(ch);

                    if (tok2 == 0) {
						ThrowXML(ParseException,XMLExcepts::Parser_Atom5);
                    }

					tok->mergeRanges(tok2);
					end = true;
                }
                break;
			default:
				ch = decodeEscaped();
			}
        } // end if T_BACKSOLIDUS
        else if (type == T_XMLSCHEMA_CC_SUBTRACTION && !firstLoop) {

            if (isNRange) {

			    base->subtractRanges(tok);
                tok = base;
            }

			RangeToken* rangeTok = parseCharacterClass(false);
			tok->subtractRanges(rangeTok);

            if (getState() != T_CHAR || getCharData() != chCloseSquare) {
                ThrowXML(ParseException,XMLExcepts::Parser_CC5);
            }
            break;
        } // end if T_XMLSCHEMA...

        processNext();

        if (!end) {

            if (type == T_CHAR) {

				if (ch == chOpenSquare)
					ThrowXML(ParseException,XMLExcepts::Parser_CC6);

				if (ch == chCloseSquare)
                    ThrowXML(ParseException,XMLExcepts::Parser_CC7);
            }

			if (getState() != T_CHAR || getCharData() != chDash) {
				tok->addRange(ch, ch);
			}
			else {

				processNext();
                if ((type = getState()) == T_EOF)
					ThrowXML(ParseException,XMLExcepts::Parser_CC2);

				if (type == T_CHAR && getCharData() == chCloseSquare) {

					tok->addRange(ch, ch);
					tok->addRange(chDash, chDash);
				}
				else if (type == T_XMLSCHEMA_CC_SUBTRACTION) {
					tok->addRange(ch, ch);
					tok->addRange(chDash, chDash);
				}
				else {

					XMLInt32 rangeEnd = getCharData();

					if (type == T_CHAR) {

						if (rangeEnd == chOpenSquare)
							ThrowXML(ParseException,XMLExcepts::Parser_CC6);

						if (rangeEnd == chCloseSquare)
							ThrowXML(ParseException,XMLExcepts::Parser_CC7);
					}

					if (type == T_BACKSOLIDUS) {
						rangeEnd = decodeEscaped();
					}

					processNext();
					tok->addRange(ch, rangeEnd);
				}
			}
        }
		firstLoop = false;
    }

	if (getState() == T_EOF)
		ThrowXML(ParseException,XMLExcepts::Parser_CC2);

	tok->sortRanges();
	tok->compactRanges();
	setParseContext(S_NORMAL);
	processNext();

	return tok;
}

XMLInt32 ParserForXMLSchema::processCInCharacterClass(RangeToken* const tok,
                                                      const XMLInt32 ch)
{
	tok->mergeRanges(getTokenForShorthand(ch));
	return -1;
}

// ---------------------------------------------------------------------------
//  ParserForXMLSchema: Getter methods
// ---------------------------------------------------------------------------
Token* ParserForXMLSchema::getTokenForShorthand(const XMLInt32 ch) {

    switch(ch) {
    case chLatin_d:
        return getTokenFactory()->getRange(fgXMLDigit);
    case chLatin_D:
        return getTokenFactory()->getRange(fgXMLDigit, true);
    case chLatin_w:
        return getTokenFactory()->getRange(fgXMLWord);
    case chLatin_W:
        return getTokenFactory()->getRange(fgXMLWord, true);
    case chLatin_s:
        return getTokenFactory()->getRange(fgXMLSpace);
    case chLatin_S:
        return getTokenFactory()->getRange(fgXMLSpace, true);
    case chLatin_c:
        return getTokenFactory()->getRange(fgXMLNameChar);
    case chLatin_C:
        return getTokenFactory()->getRange(fgXMLNameChar, true);
    case chLatin_i:
        return getTokenFactory()->getRange(fgXMLInitialNameChar);
    case chLatin_I:
        return getTokenFactory()->getRange(fgXMLInitialNameChar, true);
    }

    return 0;
}

// ---------------------------------------------------------------------------
//  ParserForXMLSchema: Helper methods
// ---------------------------------------------------------------------------
XMLInt32 ParserForXMLSchema::decodeEscaped() {

	if (getState() != T_BACKSOLIDUS)
		ThrowXML(ParseException,XMLExcepts::Parser_Next1);;

	XMLInt32 ch = getCharData();

	switch (ch) {
	case chLatin_n:
		ch = chLF;
		break;
	case chLatin_r:
		ch = chCR;
		break;
	case chLatin_t:
		ch = chHTab;
		break;
	case chLatin_e:
	case chLatin_f:
	case chLatin_x:
	case chLatin_u:
	case chLatin_v:
		ThrowXML(ParseException,XMLExcepts::Parser_Process1);
	case chLatin_A:
	case chLatin_Z:
	case chLatin_z:
		ThrowXML(ParseException,XMLExcepts::Parser_Descape5);
	}

	return ch;
}

/**
  *	End of file ParserForXMLSchema.cpp
  */
