/*
 * The Apache Software License, Version 1.1
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
 * $Log$
 * Revision 1.4  2001/05/11 13:26:51  tng
 * Copyright update.
 *
 * Revision 1.3  2001/05/03 18:17:52  knoaman
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
 * Revision 1.2  2001/03/22 13:23:34  knoaman
 * Minor modifications to eliminate compiler warnings.
 *
 * Revision 1.1  2001/03/02 19:23:00  knoaman
 * Schema: Regular expression handling part I
 *
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <util/regx/TokenFactory.hpp>
#include <util/regx/TokenInc.hpp>
#include <util/regx/XMLRangeFactory.hpp>
#include <util/regx/ASCIIRangeFactory.hpp>
#include <util/regx/UnicodeRangeFactory.hpp>
#include <util/regx/BlockRangeFactory.hpp>
#include <util/regx/RangeTokenMap.hpp>
#include <util/regx/RegxDefs.hpp>


// ---------------------------------------------------------------------------
//  TokenFactory: Constructors and Destructor
// ---------------------------------------------------------------------------
TokenFactory::TokenFactory() :
    fRangeInitialized(0)
    , fTokens(new RefVectorOf<Token> (16, true))
    , fEmpty(0)
    , fLineBegin(0)
    , fLineBegin2(0)
    , fLineEnd(0)
    , fStringBegin(0)
    , fStringEnd(0)
    , fStringEnd2(0)
    , fWordEdge(0)
    , fNotWordEdge(0)
    , fWordEnd(0)
    , fWordBegin(0)
    , fDot(0)
    , fCombiningChar(0)
    , fGrapheme(0)
{

}

TokenFactory::~TokenFactory() {

	delete fTokens;
	fTokens = 0;
}

// ---------------------------------------------------------------------------
//  TokenFactory - Factory methods
// ---------------------------------------------------------------------------
Token* TokenFactory::createToken(const unsigned short tokType) {

	if (tokType == Token::EMPTY && fEmpty != 0)
		return fEmpty;

	Token* tmpTok = new Token(tokType);

	if (tokType == Token::EMPTY) {
		fEmpty = tmpTok;
    }

	fTokens->addElement(tmpTok);

	return tmpTok;
}


ParenToken* TokenFactory::createLook(const unsigned short tokType,
									 Token* const token) {

	ParenToken* tmpTok = new ParenToken(tokType, token, 0);

	fTokens->addElement(tmpTok);
	return tmpTok;
}

ParenToken* TokenFactory::createParenthesis(Token* const token,
											const int noGroups) {

	ParenToken* tmpTok = new ParenToken(Token::PAREN, token, noGroups);

	fTokens->addElement(tmpTok);
	return tmpTok;
}

ClosureToken* TokenFactory::createClosure(Token* const token,
										  bool isNonGreedy) {

	ClosureToken* tmpTok = isNonGreedy ? new ClosureToken(Token::NONGREEDYCLOSURE, token)
									   : new ClosureToken(Token::CLOSURE, token);
	
	fTokens->addElement(tmpTok);
	return tmpTok;
}

ConcatToken* TokenFactory::createConcat(Token* const token1,
                                        Token* const token2) {

    ConcatToken* tmpTok = new ConcatToken(token1, token2);
	
    fTokens->addElement(tmpTok);
    return tmpTok;
}

UnionToken* TokenFactory::createUnion(const bool isConcat) {

	UnionToken* tmpTok = isConcat ? new UnionToken(Token::CONCAT)
								  : new UnionToken(Token::UNION);

	fTokens->addElement(tmpTok);
	return tmpTok;
}

RangeToken* TokenFactory::createRange(const bool isNegRange){


	RangeToken* tmpTok = isNegRange ? new RangeToken(Token::NRANGE)
								   : new RangeToken(Token::RANGE);

	fTokens->addElement(tmpTok);
	return tmpTok;

	return 0;
}

CharToken* TokenFactory::createChar(const XMLUInt32 ch, const bool isAnchor) {

	CharToken* tmpTok = isAnchor ? new CharToken(Token::ANCHOR, ch)
								: new CharToken(Token::CHAR, ch);

	fTokens->addElement(tmpTok);
	return tmpTok;
}

StringToken* TokenFactory::createBackReference(const int noRefs) {

	StringToken* tmpTok = new StringToken(Token::BACKREFERENCE, 0, noRefs);

	fTokens->addElement(tmpTok);
	return tmpTok;
}

StringToken* TokenFactory::createString(const XMLCh* const literal) {

	StringToken* tmpTok = new StringToken(Token::STRING, literal, 0);

	fTokens->addElement(tmpTok);
	return tmpTok;
}

ModifierToken* TokenFactory::createModifierGroup(Token* const child,
                                                 const int add,
                                                 const int mask) {

	ModifierToken* tmpTok = new ModifierToken(child, add, mask);

	fTokens->addElement(tmpTok);
	return tmpTok;
}

ConditionToken* TokenFactory::createCondition(const int refNo,
                                              Token* const condition,
                                              Token* const yesFlow,
                                              Token* const noFlow) {

	ConditionToken* tmpTok = new ConditionToken(refNo, condition, yesFlow,
                                                noFlow);
	fTokens->addElement(tmpTok);
	return tmpTok;
}

// ---------------------------------------------------------------------------
//  TokenFactory - Getter methods
// ---------------------------------------------------------------------------
RangeToken* TokenFactory::getRange(const XMLCh* const keyword,
                                   const bool complement) {

    if (!fRangeInitialized) {
		initializeRegistry();
	}

	return RangeTokenMap::instance()->getRange(keyword, complement);
}

Token* TokenFactory::getLineBegin() {

	if (fLineBegin == 0)
        fLineBegin = createChar(chCaret, true);

    return fLineBegin;
}

Token* TokenFactory::getLineBegin2() {

	if (fLineBegin2 == 0)
        fLineBegin2 = createChar(chAt, true);

    return fLineBegin2;
}

Token* TokenFactory::getLineEnd() {

	if (fLineEnd == 0)
        fLineEnd = createChar(chDollarSign, true);

    return fLineEnd;
}

Token* TokenFactory::getStringBegin() {

	if (fStringBegin == 0)
        fStringBegin = createChar(chLatin_A, true);

    return fStringBegin;
}

Token* TokenFactory::getStringEnd() {

		if (fStringEnd == 0)
        fStringEnd = createChar(chLatin_z, true);

    return fStringEnd;
}

Token* TokenFactory::getStringEnd2() {

	if (fStringEnd2 == 0)
        fStringEnd2 = createChar(chLatin_Z, true);

    return fStringEnd2;
}

Token* TokenFactory::getWordEdge() {

	if (fWordEdge == 0)
        fWordEdge = createChar(chLatin_b, true);

    return fWordEdge;
}

Token* TokenFactory::getNotWordEdge(){

	if (fNotWordEdge == 0)
        fNotWordEdge = createChar(chLatin_B, true);

    return fNotWordEdge;
}

Token* TokenFactory::getWordBegin() {

	if (fWordBegin == 0)
        fWordBegin = createChar(chOpenAngle, true);

    return fWordBegin;
}

Token* TokenFactory::getWordEnd() {

	if (fWordEnd == 0)
        fWordEnd = createChar(chCloseAngle, true);

    return fWordEnd;
}

Token* TokenFactory::getDot() {

	if (fDot == 0)
        fDot = createToken(Token::DOT);

    return fDot;
}

Token* TokenFactory::getCombiningCharacterSequence() {

	if (fCombiningChar == 0) {

		Token* foo = createClosure(getRange(fgUniMark)); // \pM*
		foo = createConcat(getRange(fgUniMark, true), foo); // \PM + \pM*
		fCombiningChar = foo;
	}

	return fCombiningChar;
}



//    static final String viramaString =


Token* TokenFactory::getGraphemePattern() {

	if (fGrapheme == 0) {

        Token* base_char = createRange();  // [{ASSIGNED}]-[{M},{C}]
        base_char->mergeRanges(getRange(fgUniAssigned));
        base_char->subtractRanges(getRange(fgUniMark));
        base_char->subtractRanges(getRange(fgUniControl));

        Token* virama = createRange();
		virama->addRange(0x094D, 0x094D);
		virama->addRange(0x09CD, 0x09CD);
		virama->addRange(0x0A4D, 0x0A4D);
		virama->addRange(0x0ACD, 0x0ACD);
		virama->addRange(0x0B4D, 0x0B4D);
		virama->addRange(0x0BCD, 0x0BCD);
		virama->addRange(0x0C4D, 0x0C4D);
		virama->addRange(0x0CCD, 0x0CCD);
		virama->addRange(0x0D4D, 0x0D4D);
		virama->addRange(0x0E3A, 0x0E3A);
		virama->addRange(0x0F84, 0x0F84);

        Token* combiner_wo_virama = createRange();
        combiner_wo_virama->mergeRanges(getRange(fgUniMark));
        combiner_wo_virama->addRange(0x1160, 0x11FF); // hangul_medial and hangul_final
        combiner_wo_virama->addRange(0xFF9F, 0xFF9F); // extras

        Token* left = TokenFactory::createUnion();       // base_char?
        left->addChild(base_char, this);
        left->addChild(createToken(Token::EMPTY), this);

        Token* foo = createUnion();
        foo->addChild(TokenFactory::createConcat(virama,getRange(fgUniLetter)), this);
        foo->addChild(combiner_wo_virama, this);

        foo = createClosure(foo);
        foo = createConcat(left, foo);

        fGrapheme = foo;
	}

	return fGrapheme;
}

// ---------------------------------------------------------------------------
//  TokenFactory - Helper methods
// ---------------------------------------------------------------------------
void TokenFactory::initializeRegistry() {

	XMLMutexLock lockInit(&fMutex);

    if (fRangeInitialized)
        return;

    RangeTokenMap::instance()->initializeRegistry();

	    // Add categories
    RangeTokenMap::instance()->addCategory(fgXMLCategory);
    RangeTokenMap::instance()->addCategory(fgASCIICategory);
    RangeTokenMap::instance()->addCategory(fgUnicodeCategory);
    RangeTokenMap::instance()->addCategory(fgBlockCategory);

	// Add xml range factory
    RangeFactory* rangeFact = new XMLRangeFactory();
    RangeTokenMap::instance()->addRangeMap(fgXMLCategory, rangeFact);
    rangeFact->initializeKeywordMap();

    // Add ascii range factory
    rangeFact = new ASCIIRangeFactory();
    RangeTokenMap::instance()->addRangeMap(fgASCIICategory, rangeFact);
    rangeFact->initializeKeywordMap();

    // Add unicode range factory
    rangeFact = new UnicodeRangeFactory();
    RangeTokenMap::instance()->addRangeMap(fgUnicodeCategory, rangeFact);
    rangeFact->initializeKeywordMap();

    // Add block range factory
    rangeFact = new BlockRangeFactory();
    RangeTokenMap::instance()->addRangeMap(fgBlockCategory, rangeFact);
    rangeFact->initializeKeywordMap();

    fRangeInitialized = true;
}

/*
#if defined (XML_USE_ICU_TRANSCODER)
   #include <unicode/unicode.h>
#endif

#include <stdio.h>
void TokenFactory::printUnicode() {

#if defined (XML_USE_ICU_TRANSCODER)
    //
    //  Write it out to a temp file to be read back into this source later.
    //
	printf("Printing\n");
	//sprintf(msg, "Printing\n");
    FILE* outFl = fopen("table.out", "wt+");
    fprintf(outFl, "const XMLByte fgUniCharsTable[0x10000] =\n{    ");
    for (unsigned int index = 0; index <= 0xFFFF; index += 16)
    {
        fprintf(outFl
                , "    , 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n"
                , (unsigned int)Unicode::getType(index)
                , (unsigned int)Unicode::getType(index+1)
                , (unsigned int)Unicode::getType(index+2)
                , (unsigned int)Unicode::getType(index+3)
                , (unsigned int)Unicode::getType(index+4)
                , (unsigned int)Unicode::getType(index+5)
                , (unsigned int)Unicode::getType(index+6)
                , (unsigned int)Unicode::getType(index+7)
				, (unsigned int)Unicode::getType(index+8)
                , (unsigned int)Unicode::getType(index+9)
                , (unsigned int)Unicode::getType(index+10)
                , (unsigned int)Unicode::getType(index+11)
				, (unsigned int)Unicode::getType(index+12)
                , (unsigned int)Unicode::getType(index+13)
                , (unsigned int)Unicode::getType(index+14)
                , (unsigned int)Unicode::getType(index+15));
    }
    fprintf(outFl, "};\n");

    fclose(outFl);
#endif
}
*/

/**
  * End of file TokenFactory.cpp
  */