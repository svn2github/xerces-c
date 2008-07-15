/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * $Id$
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/regx/RegularExpression.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/regx/Match.hpp>
#include <xercesc/util/regx/RangeToken.hpp>
#include <xercesc/util/regx/RegxDefs.hpp>
#include <xercesc/util/regx/XMLUniCharacter.hpp>
#include <xercesc/util/regx/ParserForXMLSchema.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/ParseException.hpp>
#include <xercesc/util/IllegalArgumentException.hpp>
#include <xercesc/framework/XMLBuffer.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/util/XMLInitializer.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/ValueStackOf.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Static member data initialization
// ---------------------------------------------------------------------------
const unsigned int RegularExpression::MARK_PARENS = 1;
const unsigned int RegularExpression::IGNORE_CASE = 2;
const unsigned int RegularExpression::SINGLE_LINE = 4;
const unsigned int RegularExpression::MULTIPLE_LINE = 8;
const unsigned int RegularExpression::EXTENDED_COMMENT = 16;
const unsigned int RegularExpression::USE_UNICODE_CATEGORY = 32;
const unsigned int RegularExpression::UNICODE_WORD_BOUNDARY = 64;
const unsigned int RegularExpression::PROHIBIT_HEAD_CHARACTER_OPTIMIZATION = 128;
const unsigned int RegularExpression::PROHIBIT_FIXED_STRING_OPTIMIZATION = 256;
const unsigned int RegularExpression::XMLSCHEMA_MODE = 512;
const unsigned int RegularExpression::SPECIAL_COMMA = 1024;
RangeToken*        RegularExpression::fWordRange = 0;

bool RegularExpression::matchIgnoreCase(const XMLInt32 ch1,
                                        const XMLInt32 ch2) const
{
    if (ch1 >= 0x10000)
    {
        XMLCh string1[2];
        XMLCh string2[2];

        RegxUtil::decomposeToSurrogates(ch1, string1[0], string1[1]);

        if (ch2 >= 0x10000)
        {
            RegxUtil::decomposeToSurrogates(ch2, string2[0], string2[1]);
        }
        else
        {
            // XMLString::compareNIString is broken, because it assume the
            // two strings must be of the same length.  Note that two strings
            // of different length could compare as equal, because there is no
            // guarantee that a Unicode code point that is encoded in UTF-16 as
            // a surrogate pair does not have a case mapping to a code point
            // that is not in the surrogate range.  Just to be safe, we pad the
            // shorter string with a space, which cannot hvae a case mapping.
            string2[0] = (XMLCh)ch2;
            string2[1] = chSpace;
        }

        return (0==XMLString::compareNIString(string1, string2, 2));
    }
    else if (ch2 >= 0x10000)
    {
        const XMLCh string1[2] = { (XMLCh)ch1, chSpace };
        XMLCh string2[2];

        RegxUtil::decomposeToSurrogates(ch2, string2[0], string2[1]);

        return (0==XMLString::compareNIString(string1, string2, 2));
    }
    else
    {
        const XMLCh  char1 = (XMLCh)ch1;
        const XMLCh  char2 = (XMLCh)ch2;

        return (0==XMLString::compareNIString(&char1, &char2, 1));
    }
}



// ---------------------------------------------------------------------------
//  RegularExpression::Context: Constructors and Destructor
// ---------------------------------------------------------------------------
RegularExpression::Context::Context(MemoryManager* const manager) :
    fAdoptMatch(false)
    , fStart(0)
    , fLimit(0)
    , fLength(0)
    , fSize(0)
    , fStringMaxLen(0)
    , fOffsets(0)
    , fMatch(0)
    , fString(0)
    , fOptions(0)
    , fMemoryManager(manager)
{
}

RegularExpression::Context::Context(Context* src) :
    fAdoptMatch(false)
    , fStart(src->fStart)
    , fLimit(src->fLimit)
    , fLength(src->fLength)
    , fSize(src->fSize)
    , fStringMaxLen(src->fStringMaxLen)
    , fOffsets(0)
    , fMatch(0)
    , fString(src->fString)
    , fOptions(src->fOptions)
    , fMemoryManager(src->fMemoryManager)
{
    if(src->fOffsets)
    {
        fOffsets = (int*) fMemoryManager->allocate(fSize* sizeof(int));
        for (int i = 0; i< fSize; i++)
            fOffsets[i] = src->fOffsets[i];
    }
    if(src->fMatch)
    {
        fMatch=new Match(*src->fMatch);
        fAdoptMatch=true;
    }
}

RegularExpression::Context& RegularExpression::Context::operator=(const RegularExpression::Context& other)
{
    if (this != &other)
    {
        fStart=other.fStart;
        fLimit=other.fLimit;
        fLength=other.fLength;
        fSize=other.fSize;
        fStringMaxLen=other.fStringMaxLen;
        fString=other.fString;
        fOptions=other.fOptions;
        if (fOffsets)
            fMemoryManager->deallocate(fOffsets);//delete [] fOffsets;
        fOffsets=0;
        if (fAdoptMatch)
            delete fMatch;
        fMatch=0;
        fAdoptMatch=false;

        fMemoryManager=other.fMemoryManager;
        if(other.fOffsets)
        {
            fOffsets = (int*) fMemoryManager->allocate(fSize* sizeof(int));
            for (int i = 0; i< fSize; i++)
                fOffsets[i] = other.fOffsets[i];
        }
        if(other.fMatch)
        {
            fMatch=new Match(*other.fMatch);
            fAdoptMatch=true;
        }
    }

    return *this;
}

RegularExpression::Context::~Context()
{
    if (fOffsets)
        fMemoryManager->deallocate(fOffsets);//delete [] fOffsets;

    if (fAdoptMatch)
        delete fMatch;
}

// ---------------------------------------------------------------------------
//  RegularExpression::Context: Public methods
// ---------------------------------------------------------------------------
void RegularExpression::Context::reset(const XMLCh* const string
                                       , const XMLSize_t stringLen
                                       , const XMLSize_t start
                                       , const XMLSize_t limit
                                       , const int noClosures
                                       , const unsigned int options)
{
    fString = string;
    fStringMaxLen = stringLen;
    fStart = start;
    fLimit = limit;
    fLength = fLimit - fStart;
    if (fAdoptMatch)
        delete fMatch;
    fMatch = 0;

    if (fSize != noClosures) {
        if (fOffsets)
            fMemoryManager->deallocate(fOffsets);//delete [] fOffsets;
        fOffsets = (int*) fMemoryManager->allocate(noClosures * sizeof(int));//new int[noClosures];
    }

    fSize = noClosures;
    fOptions = options;

    for (int i = 0; i< fSize; i++)
        fOffsets[i] = -1;
}

bool RegularExpression::Context::nextCh(XMLInt32& ch, XMLSize_t& offset,
                                        const short direction)
{
    ch = fString[offset];

    if (RegxUtil::isHighSurrogate(ch)) {
        if ((offset + 1 < fLimit) && (direction > 0) &&
            RegxUtil::isLowSurrogate(fString[offset+1])) {
            ch = RegxUtil::composeFromSurrogate(ch, fString[++offset]);
        }
        else
            return false;
	}
    else if (RegxUtil::isLowSurrogate(ch)) {
        if ((offset > 0) && (direction <= 0) &&
            RegxUtil::isHighSurrogate(fString[offset-1])) {
            ch = RegxUtil::composeFromSurrogate(fString[--offset], ch);
        }
        else
            return false;
    }

    return true;
}

// ---------------------------------------------------------------------------
//  RegularExpression: Constructors and Destructors
// ---------------------------------------------------------------------------

typedef JanitorMemFunCall<RegularExpression>    CleanupType;

RegularExpression::RegularExpression(const char* const pattern,
                                     MemoryManager* const manager)
    :fHasBackReferences(false),
     fFixedStringOnly(false),
     fNoGroups(0),
     fMinLength(0),
     fNoClosures(0),
     fOptions(0),
     fBMPattern(0),
     fPattern(0),
     fFixedString(0),
     fOperations(0),
     fTokenTree(0),
     fFirstChar(0),
     fOpFactory(manager),
     fTokenFactory(0),
     fMemoryManager(manager)
{
    CleanupType cleanup(this, &RegularExpression::cleanUp);

    try {

        XMLCh* tmpBuf = XMLString::transcode(pattern, fMemoryManager);
        ArrayJanitor<XMLCh> janBuf(tmpBuf, fMemoryManager);
        setPattern(tmpBuf);
    }
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}

RegularExpression::RegularExpression(const char* const pattern,
                                     const char* const options,
                                     MemoryManager* const manager)
    :fHasBackReferences(false),
     fFixedStringOnly(false),
     fNoGroups(0),
     fMinLength(0),
     fNoClosures(0),
     fOptions(0),
     fBMPattern(0),
     fPattern(0),
     fFixedString(0),
     fOperations(0),
     fTokenTree(0),
     fFirstChar(0),
     fOpFactory(manager),
     fTokenFactory(0),
     fMemoryManager(manager)
{
    CleanupType cleanup(this, &RegularExpression::cleanUp);

    try {

        XMLCh* tmpBuf = XMLString::transcode(pattern, fMemoryManager);
        ArrayJanitor<XMLCh> janBuf(tmpBuf, fMemoryManager);
        XMLCh* tmpOptions = XMLString::transcode(options, fMemoryManager);
        ArrayJanitor<XMLCh> janOps(tmpOptions, fMemoryManager);
        setPattern(tmpBuf, tmpOptions);
    }
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}


RegularExpression::RegularExpression(const XMLCh* const pattern,
                                     MemoryManager* const manager)
    :fHasBackReferences(false),
     fFixedStringOnly(false),
     fNoGroups(0),
     fMinLength(0),
     fNoClosures(0),
     fOptions(0),
     fBMPattern(0),
     fPattern(0),
     fFixedString(0),
     fOperations(0),
     fTokenTree(0),
     fFirstChar(0),
     fOpFactory(manager),
     fTokenFactory(0),
     fMemoryManager(manager)
{
    CleanupType cleanup(this, &RegularExpression::cleanUp);

    try {

        setPattern(pattern);
    }
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}

RegularExpression::RegularExpression(const XMLCh* const pattern,
                                     const XMLCh* const options,
                                     MemoryManager* const manager)
    :fHasBackReferences(false),
     fFixedStringOnly(false),
     fNoGroups(0),
     fMinLength(0),
     fNoClosures(0),
     fOptions(0),
     fBMPattern(0),
     fPattern(0),
     fFixedString(0),
     fOperations(0),
     fTokenTree(0),
     fFirstChar(0),
     fOpFactory(manager),
     fTokenFactory(0),
     fMemoryManager(manager)
{
    CleanupType cleanup(this, &RegularExpression::cleanUp);

    try {

        setPattern(pattern, options);
    }
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}

RegularExpression::~RegularExpression() {

    cleanUp();
}

// ---------------------------------------------------------------------------
//  RegularExpression: Setter methods
// ---------------------------------------------------------------------------

RegxParser* RegularExpression::getRegexParser(const int options, MemoryManager* const manager)
{
    // the following construct causes an error in an Intel 7.1 32 bit compiler for
    // red hat linux 7.2
    // (when an exception is thrown the wrong object is deleted)
    //RegxParser* regxParser = isSet(fOptions, XMLSCHEMA_MODE)
    //	? new (fMemoryManager) ParserForXMLSchema(fMemoryManager)
    //    : new (fMemoryManager) RegxParser(fMemoryManager);
    if (isSet(options, XMLSCHEMA_MODE))
        return new (manager) ParserForXMLSchema(manager);

    return new (manager) RegxParser(manager);
}

void RegularExpression::setPattern(const XMLCh* const pattern,
                                   const XMLCh* const options)
{

    fTokenFactory = new (fMemoryManager) TokenFactory(fMemoryManager);
    fOptions = parseOptions(options);
    fPattern = XMLString::replicate(pattern, fMemoryManager);

    RegxParser* regxParser=getRegexParser(fOptions, fMemoryManager);

    if (regxParser)
        regxParser->setTokenFactory(fTokenFactory);

    Janitor<RegxParser> janRegxParser(regxParser);
    fTokenTree = regxParser->parse(fPattern, fOptions);
    fNoGroups = regxParser->getNoParen();
    fHasBackReferences = regxParser->hasBackReferences();

    prepare();
}

// ---------------------------------------------------------------------------
//  RegularExpression: Matching methods
// ---------------------------------------------------------------------------
bool RegularExpression::matches(const char* const expression
                                , MemoryManager* const manager) const
{
    XMLCh* tmpBuf = XMLString::transcode(expression, manager);
    ArrayJanitor<XMLCh> janBuf(tmpBuf, manager);
    return matches(tmpBuf, 0, XMLString::stringLen(tmpBuf), 0, manager);
}

bool RegularExpression::matches(const char* const expression
                                , const XMLSize_t start, const XMLSize_t end
                                , MemoryManager* const manager) const
{

    XMLCh* tmpBuf = XMLString::transcode(expression, manager);
    ArrayJanitor<XMLCh> janBuf(tmpBuf, manager);
    return matches(tmpBuf, start, end, 0, manager);
}

bool RegularExpression::matches(const char* const expression
                                , Match* const match
                                , MemoryManager* const manager) const
{

    XMLCh* tmpBuf = XMLString::transcode(expression, manager);
    ArrayJanitor<XMLCh> janBuf(tmpBuf, manager);
    return matches(tmpBuf, 0, XMLString::stringLen(tmpBuf), match, manager);
}

bool RegularExpression::matches(const char* const expression, const XMLSize_t start
                                , const XMLSize_t end, Match* const pMatch
                                , MemoryManager* const manager) const
{

    XMLCh* tmpBuf = XMLString::transcode(expression, manager);
    ArrayJanitor<XMLCh> janBuf(tmpBuf, manager);
    return matches(tmpBuf, start, end, pMatch, manager);
}


// ---------------------------------------------------------------------------
//  RegularExpression: Matching methods - Wide char version
// ---------------------------------------------------------------------------
bool RegularExpression::matches(const XMLCh* const expression, MemoryManager* const manager) const
{
    return matches(expression, 0, XMLString::stringLen(expression), 0, manager);
}

bool RegularExpression::matches(const XMLCh* const expression
                                , const XMLSize_t start, const XMLSize_t end
                                , MemoryManager* const manager) const
{
    return matches(expression, start, end, 0, manager);
}

bool RegularExpression::matches(const XMLCh* const expression
                                , Match* const match
                                , MemoryManager* const manager) const
{
    return matches(expression, 0, XMLString::stringLen(expression), match, manager);
}

bool RegularExpression::matches(const XMLCh* const expression, const XMLSize_t start
                                , const XMLSize_t end, Match* const pMatch
                                , MemoryManager* const manager) const
{

    Context context(manager);
    XMLSize_t strLength = XMLString::stringLen(expression);

    context.reset(expression, strLength, start, end, fNoClosures, fOptions);

    bool adoptMatch = false;
    Match* lMatch = pMatch;

    if (lMatch != 0) {
        lMatch->setNoGroups(fNoGroups);
    }
    else if (fHasBackReferences) {
        lMatch = new (manager) Match(manager);
        lMatch->setNoGroups(fNoGroups);
        adoptMatch = true;
    }

    if (context.fAdoptMatch)
        delete context.fMatch;
    context.fMatch = lMatch;
    context.fAdoptMatch = adoptMatch;

    if (isSet(fOptions, XMLSCHEMA_MODE)) {

        int matchEnd = match(&context, fOperations, context.fStart, 1);

        if (matchEnd == (int)context.fLimit) {

            if (context.fMatch != 0) {

                context.fMatch->setStartPos(0, (int)context.fStart);
                context.fMatch->setEndPos(0, matchEnd);
            }
            return true;
        }

        return false;
    }

    /*
     * If the pattern has only fixed string, use Boyer-Moore
     */
    if (fFixedStringOnly) {

        int ret = fBMPattern->matches(expression, context.fStart, context.fLimit);
        if (ret >= 0) {

            if (context.fMatch != 0) {
                context.fMatch->setStartPos(0, ret);
                context.fMatch->setEndPos(0, (int)(ret + strLength));
            }
            return true;
        }
        return false;
    }

    /*
     * If the pattern contains a fixed string, we check with Boyer-Moore
     * whether the text contains the fixed string or not. If not found
     * return false
     */
    if (fFixedString != 0) {

        int ret = fBMPattern->matches(expression, context.fStart, context.fLimit);

        if (ret < 0) { // No match
            return false;
        }
    }

    // if the length is less than the minimum length, we cannot possibly match
    if(context.fLimit<fMinLength)
        return false;

    XMLSize_t limit = context.fLimit - fMinLength;
    XMLSize_t matchStart;
    int matchEnd = -1;

    /*
     * Check whether the expression start with ".*"
     */
    if (fOperations != 0 && (fOperations->getOpType() == Op::O_CLOSURE || fOperations->getOpType() == Op::O_FINITE_CLOSURE)
        && fOperations->getChild()->getOpType() == Op::O_DOT) {

        if (isSet(fOptions, SINGLE_LINE)) {
            matchStart = context.fStart;
            matchEnd = match(&context, fOperations, matchStart, 1);
        }
        else {
            bool previousIsEOL = true;

            for (matchStart=context.fStart; matchStart<=limit; matchStart++) {

                XMLCh ch = expression[matchStart];
                if (RegxUtil::isEOLChar(ch)) {
                    previousIsEOL = true;
                }
                else {

                    if (previousIsEOL) {
                        if (0 <= (matchEnd = match(&context, fOperations,
                                                   matchStart, 1)))
                            break;
                    }

                    previousIsEOL = false;
                }
            }
        }
    }
    else {
        /*
         *	Optimization against the first char
         */
        if (fFirstChar != 0) {
            bool ignoreCase = isSet(fOptions, IGNORE_CASE);
            RangeToken* range = fFirstChar;

            if (ignoreCase)
                range = fFirstChar->getCaseInsensitiveToken(fTokenFactory);

            for (matchStart=context.fStart; matchStart<=limit; matchStart++) {

                XMLInt32 ch;

                if (!context.nextCh(ch, matchStart, 1))
                    break;

                if (!range->match(ch))
                    continue;

                if (0 <= (matchEnd = match(&context,fOperations,matchStart,1)))
                    break;
            }
        }
        else {

            /*
             *	Straightforward matching
             */
            for (matchStart=context.fStart; matchStart<=limit; matchStart++) {

                if (0 <= (matchEnd = match(&context,fOperations,matchStart,1)))
                    break;
            }
        }
    }

    if (matchEnd >= 0) {

        if (context.fMatch != 0) {

            context.fMatch->setStartPos(0, (int)matchStart);
            context.fMatch->setEndPos(0, matchEnd);
        }
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
//  RegularExpression: Tokenize methods
// ---------------------------------------------------------------------------
RefArrayVectorOf<XMLCh>* RegularExpression::tokenize(const char* const expression,
                                                     MemoryManager* const manager) const
{

  XMLCh* tmpBuf = XMLString::transcode(expression, manager);
  ArrayJanitor<XMLCh> janBuf(tmpBuf, manager);
  return tokenize(tmpBuf, 0, XMLString::stringLen(tmpBuf), manager);
}

RefArrayVectorOf<XMLCh>* RegularExpression::tokenize(const char* const expression,
                                                     const XMLSize_t start, const XMLSize_t end,
                                                     MemoryManager* const manager) const
{

  XMLCh* tmpBuf = XMLString::transcode(expression, manager);
  ArrayJanitor<XMLCh> janBuf(tmpBuf, manager);
  return tokenize(tmpBuf, start, end, manager);
}



// ---------------------------------------------------------------------------
//  RegularExpression: Tokenize methods - Wide char version
// ---------------------------------------------------------------------------
RefArrayVectorOf<XMLCh>* RegularExpression::tokenize(const XMLCh* const expression,
                                                     MemoryManager* const manager) const
{
    return tokenize(expression, 0, XMLString::stringLen(expression), manager);
}

RefArrayVectorOf<XMLCh>* RegularExpression::tokenize(const XMLCh* const matchString,
                                                     const XMLSize_t start, const XMLSize_t end,
                                                     MemoryManager* const manager) const
{
    // check if matches zero length string - throw error if so
    if(matches(XMLUni::fgZeroLenString, manager)){
		ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Regex_RepPatMatchesZeroString, manager);
    }
      
    RefVectorOf<Match> *subEx = new (manager) RefVectorOf<Match>(10, true, manager);
    Janitor<RefVectorOf<Match> > janSubEx(subEx);

    allMatches(matchString, start, end, subEx, manager);

    RefArrayVectorOf<XMLCh> *tokens = new (manager) RefArrayVectorOf<XMLCh>(16, true, manager);
    XMLSize_t tokStart = start;

    unsigned int i = 0;
    for(; i < subEx->size(); ++i) {
        Match *match = subEx->elementAt(i);
        XMLSize_t matchStart = match->getStartPos(0);

        XMLCh *token = (XMLCh*)manager->allocate((matchStart + 1 - tokStart) * sizeof(XMLCh));
        XMLString::subString(token, matchString, tokStart, matchStart, manager);
        tokens->addElement(token);

        tokStart = match->getEndPos(0);
    }

    XMLCh *token = (XMLCh*)manager->allocate((end + 1 - tokStart) * sizeof(XMLCh));
    XMLString::subString(token, matchString, tokStart, end, manager);
    tokens->addElement(token);

    return tokens;
}

void RegularExpression::allMatches(const XMLCh* const matchString, const XMLSize_t start, const XMLSize_t end,
                                   RefVectorOf<Match> *subEx, MemoryManager* const manager) const
{
    Context context(manager);
    context.reset(matchString, XMLString::stringLen(matchString), start, end, fNoClosures, fOptions);

    context.fMatch = new (manager) Match(manager);
    context.fMatch->setNoGroups(fNoGroups);
    context.fAdoptMatch = true;

    XMLSize_t matchStart = start;
    while(matchStart <= end) {
        XMLSize_t matchEnd = match(&context, fOperations, matchStart, 1);
        if(matchEnd != (XMLSize_t)-1) {
            context.fMatch->setStartPos(0, matchStart);
            context.fMatch->setEndPos(0, matchEnd);

            subEx->addElement(context.fMatch);
        
            context.fMatch = new (manager) Match(*(context.fMatch));
            context.fAdoptMatch = true;

            matchStart = matchEnd;
        } else {
            ++matchStart;
        }
    }
}


// -----------------------------------------------------------------------
//  RegularExpression: Replace methods
// -----------------------------------------------------------------------
XMLCh* RegularExpression::replace(const char* const matchString,
                                  const char* const replaceString,
                                  MemoryManager* const manager) const
{

    XMLCh* tmpBuf = XMLString::transcode(matchString, manager);
    ArrayJanitor<XMLCh> janBuf(tmpBuf, manager);
    XMLCh* tmpBuf2 = XMLString::transcode(replaceString, manager);
    ArrayJanitor<XMLCh> janBuf2(tmpBuf2, manager);

    return replace(tmpBuf, tmpBuf2, 0, XMLString::stringLen(tmpBuf), manager);
}

XMLCh* RegularExpression::replace(const char* const matchString,
                                  const char* const replaceString,
                                  const XMLSize_t start, const XMLSize_t end,
                                  MemoryManager* const manager) const
{

    XMLCh* tmpBuf = XMLString::transcode(matchString, manager);
    ArrayJanitor<XMLCh> janBuf(tmpBuf, manager);
    XMLCh* tmpBuf2 = XMLString::transcode(replaceString, manager);
    ArrayJanitor<XMLCh> janBuf2(tmpBuf2, manager);

    return replace(tmpBuf, tmpBuf2, start, end, manager);
}


// ---------------------------------------------------------------------------
//  RegularExpression: Replace methods - Wide char version
// ---------------------------------------------------------------------------
XMLCh* RegularExpression::replace(const XMLCh* const matchString,
                                  const XMLCh* const replaceString,
                                  MemoryManager* const manager) const
{

    return replace(matchString, replaceString, 0,
                   XMLString::stringLen(matchString), manager);
}

XMLCh* RegularExpression::replace(const XMLCh* const matchString,
                                  const XMLCh* const replaceString,
                                  const XMLSize_t start, const XMLSize_t end,
                                  MemoryManager* const manager) const
{
    // check if matches zero length string - throw error if so
    if(matches(XMLUni::fgZeroLenString, manager)){
		ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Regex_RepPatMatchesZeroString, manager);
    }
      
    RefVectorOf<Match> *subEx = new (manager) RefVectorOf<Match>(10, true, manager);
    Janitor<RefVectorOf<Match> > janSubEx(subEx);

    allMatches(matchString, start, end, subEx, manager);

    XMLBuffer result(1023, manager);
    int tokStart = start;

    unsigned int i = 0;
    for(; i < subEx->size(); ++i) {
        Match *match = subEx->elementAt(i);
        int matchStart = match->getStartPos(0);

        if(matchStart > tokStart)
            result.append(matchString + tokStart, matchStart - tokStart);
        subInExp(replaceString, matchString, match, result, manager);

        tokStart = match->getEndPos(0);
    }

    if(end > tokStart)
        result.append(matchString + tokStart, end - tokStart);

    return XMLString::replicate(result.getRawBuffer(), manager);
}

/*
 * Helper for Replace. This method prepares the replacement string by substituting
 * in actual values for parenthesized sub expressions. 
 *
 * An error will be thrown if:
 *  1) there is chBackSlash not followed by a chDollarSign or chBackSlash
 *  2) there is an unescaped chDollarSign which is not followed by a digit
 *
 */
void RegularExpression::subInExp(const XMLCh* const repString, 
                                 const XMLCh* const origString, 
                                 const Match* subEx,
                                 XMLBuffer &result,
                                 MemoryManager* const manager) const
{
    int numSubExp = subEx->getNoGroups() - 1;

    for(const XMLCh *ptr = repString; *ptr != chNull; ++ptr) {
        if(*ptr == chDollarSign) {
            ++ptr;
      
            // check that after the $ is a digit
            if(!XMLString::isDigit(*ptr)) {
                // invalid replace string - $ must be followed by a digit
                ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Regex_InvalidRepPattern, manager);
            }

            int index = *ptr - chDigit_0;

            const XMLCh *dig = ptr + 1;
            while(XMLString::isDigit(*dig)) {
                int newIndex = index * 10 + (*dig - chDigit_0);
                if(newIndex > numSubExp) break;

                index = newIndex;
                ptr = dig;
                ++dig;
            }

            // now check that the index is legal
            if(index <= numSubExp) {
                int start = subEx->getStartPos(index);
                int end = subEx->getEndPos(index);

                // now copy the substring into the new string
                if(start < end) {
                    result.append(origString + start, end - start);
                }
            }

        } else {
            if(*ptr == chBackSlash) {
                ++ptr;

                // if you have a slash and then a character that's not a $ or /,
                // then it's an invalid replace string  
                if(*ptr != chDollarSign && *ptr != chBackSlash) {
                    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Regex_InvalidRepPattern, manager);
                }
            }

            result.append(*ptr);
        }
    }
}


// -----------------------------------------------------------------------
//  Static initialize and cleanup methods
// -----------------------------------------------------------------------
void
XMLInitializer::initializeRegularExpression()
{
    RegularExpression::staticInitialize(XMLPlatformUtils::fgMemoryManager);
}

void
XMLInitializer::terminateRegularExpression()
{
    RegularExpression::staticCleanup();
}

void
RegularExpression::staticInitialize(MemoryManager* memoryManager)
{
    fWordRange = TokenFactory::staticGetRange(fgUniIsWord, false);

    if (fWordRange == 0)
        ThrowXMLwithMemMgr1(RuntimeException, XMLExcepts::Regex_RangeTokenGetError, fgUniIsWord, memoryManager);
}

// ---------------------------------------------------------------------------
//  RegularExpression: Helpers methods
// ---------------------------------------------------------------------------
int RegularExpression::getOptionValue(const XMLCh ch) {

    int ret = 0;

    switch (ch) {

        case chLatin_i:
            ret = IGNORE_CASE;
            break;
        case chLatin_m:
            ret = MULTIPLE_LINE;
            break;
        case chLatin_s:
            ret = SINGLE_LINE;
            break;
        case chLatin_x:
            ret = EXTENDED_COMMENT;
            break;
        case chLatin_u:
            ret = USE_UNICODE_CATEGORY;
            break;
        case chLatin_w:
            ret = UNICODE_WORD_BOUNDARY;
            break;
        case chLatin_F:
            ret = PROHIBIT_FIXED_STRING_OPTIMIZATION;
            break;
        case chLatin_H:
            ret = PROHIBIT_HEAD_CHARACTER_OPTIMIZATION;
            break;
        case chLatin_X:
            ret = XMLSCHEMA_MODE;
            break;
        case chComma:
            ret = SPECIAL_COMMA;
            break;
        default:
            break;
    }

    return ret;
}

struct RE_RuntimeContext {
    const Op    *op_;
    XMLSize_t   offs_;

    RE_RuntimeContext(const Op *op, XMLSize_t offs) : op_(op), offs_(offs) { }
};

int RegularExpression::match(Context* const context, const Op* const operations,
                             XMLSize_t offset, const short direction) const
{
    ValueStackOf<RE_RuntimeContext>* opStack=NULL;
    Janitor<ValueStackOf<RE_RuntimeContext> > janStack(NULL);
    if(context->fLimit > 256)
    {
        opStack=new ValueStackOf<RE_RuntimeContext>(16, context->fMemoryManager);
        janStack.reset(opStack);
    }
    const Op* tmpOp = operations;
    bool ignoreCase = isSet(context->fOptions, IGNORE_CASE);
    int doReturn;

    while (tmpOp != 0) {
        // no one wants to return -5, only -1, 0, and greater
        doReturn = -5;

        if (offset > context->fLimit || offset < context->fStart)
            doReturn = -1;
        else
        {
            switch(tmpOp->getOpType()) {
                case Op::O_CHAR:
                    if (!matchChar(context, tmpOp->getData(), offset, direction, ignoreCase))
                        doReturn = -1;
                    else
                        tmpOp = tmpOp->getNextOp();
                    break;
                case Op::O_DOT:
                    if (!matchDot(context, offset, direction))
                        doReturn = -1;
                    else
                        tmpOp = tmpOp->getNextOp();
                    break;
                case Op::O_RANGE:
                case Op::O_NRANGE:
                    if (!matchRange(context, tmpOp, offset, direction, ignoreCase))
                        doReturn = -1;
                    else
                        tmpOp = tmpOp->getNextOp();
                    break;
                case Op::O_ANCHOR:
                    if (!matchAnchor(context, tmpOp->getData(), offset))
                        doReturn = -1;
                    else
                        tmpOp = tmpOp->getNextOp();
                    break;
                case Op::O_BACKREFERENCE:
                    if (!matchBackReference(context, tmpOp->getData(), offset,
                                            direction, ignoreCase))
                        doReturn = -1;
                    else
                        tmpOp = tmpOp->getNextOp();
                    break;
                case Op::O_STRING:
                    if (!matchString(context, tmpOp->getLiteral(), offset, direction,
                                     ignoreCase))
                        doReturn = -1;
                    else
                        tmpOp = tmpOp->getNextOp();
                    break;
                case Op::O_FINITE_CLOSURE:
                {
                    XMLInt32 id = tmpOp->getData();
                    // if id is not -1, it's a closure with a child token having a minumum length,
                    // where id is the index of the fOffsets array where its status is stored
                    if (id >= 0) {
                        int prevOffset = context->fOffsets[id];
                        if (prevOffset < 0 || prevOffset != (int)offset) {
                            context->fOffsets[id] = (int)offset;
                        }
                        else {
                            // the status didn't change, we haven't found other copies; move on to the next match
                            context->fOffsets[id] = -1;
                            tmpOp = tmpOp->getNextOp();
                            break;
                        }
                    }

                    // match the subitems until they do
                    int ret;
                    while((ret = match(context, tmpOp->getChild(), offset, direction)) != -1)
                    {
                        if(offset == (XMLSize_t)ret)
                            break;
                        offset = ret;
                    }

                    if (id >= 0) {
                        // loop has ended, reset the status for this closure
                        context->fOffsets[id] = -1;
                    }
                    tmpOp = tmpOp->getNextOp();
                }
                break;
                case Op::O_FINITE_NONGREEDYCLOSURE:
                {
                    int ret = match(context,tmpOp->getNextOp(),offset,direction);
                    if (ret >= 0)
                        doReturn = ret;
                    else
                    {
                        // match the subitems until they do
                        int ret;
                        while((ret = match(context, tmpOp->getChild(), offset, direction)) != -1)
                        {
                            if(offset == (XMLSize_t)ret)
                                break;
                            offset = ret;
                        }
                        tmpOp = tmpOp->getNextOp();
                    }
                }
                break;
                case Op::O_CLOSURE:
                {
                    XMLInt32 id = tmpOp->getData();
                    // if id is not -1, it's a closure with a child token having a minumum length,
                    // where id is the index of the fOffsets array where its status is stored
                    if (id >= 0) {
                        int prevOffset = context->fOffsets[id];
                        if (prevOffset < 0 || prevOffset != (int)offset) {
                            context->fOffsets[id] = (int)offset;
                        }
                        else {
                            // the status didn't change, we haven't found other copies; move on to the next match
                            context->fOffsets[id] = -1;
                            tmpOp = tmpOp->getNextOp();
                            break;
                        }
                    }

                    if(opStack!=NULL)
                    {
                        opStack->push(RE_RuntimeContext(tmpOp, offset));
                        tmpOp = tmpOp->getChild();
                    }
                    else
                    {
                        int ret = match(context, tmpOp->getChild(), offset, direction);
                        if (id >= 0) {
                            context->fOffsets[id] = -1;
                        }
                        if (ret >= 0)
                            doReturn = ret;
                        else
                            tmpOp = tmpOp->getNextOp();
                    }
                }
                break;
                case Op::O_QUESTION:
                {
                    if(opStack!=NULL)
                    {
                        opStack->push(RE_RuntimeContext(tmpOp, offset));
                        tmpOp = tmpOp->getChild();
                    }
                    else
                    {
                        int ret = match(context, tmpOp->getChild(), offset, direction);
                        if (ret >= 0)
                            doReturn = ret;
                        else
                            tmpOp = tmpOp->getNextOp();
                    }
                }
                break;
                case Op::O_NONGREEDYCLOSURE:
                case Op::O_NONGREEDYQUESTION:
                {
                    int ret = match(context,tmpOp->getNextOp(),offset,direction);
                    if (ret >= 0)
                        doReturn = ret;
                    else
                        tmpOp = tmpOp->getChild();
                }
                break;
                case Op::O_UNION:
                    doReturn = matchUnion(context, tmpOp, offset, direction);
                    break;
                case Op::O_CAPTURE:
                    if (context->fMatch != 0 && tmpOp->getData() != 0)
                        doReturn = matchCapture(context, tmpOp, offset, direction);
                    else
                        tmpOp = tmpOp->getNextOp();
                    break;
                case Op::O_LOOKAHEAD:
                    if (0 > match(context, tmpOp->getChild(), offset, 1))
                        doReturn = -1;
                    else
                        tmpOp = tmpOp->getNextOp();
                    break;
                case Op::O_NEGATIVELOOKAHEAD:
                    if (0 <= match(context, tmpOp->getChild(), offset, 1))
                        doReturn = -1;
                    else
                        tmpOp = tmpOp->getNextOp();
                    break;
                case Op::O_LOOKBEHIND:
                    if (0 > match(context, tmpOp->getChild(), offset, -1))
                        doReturn = -1;
                    else
                        tmpOp = tmpOp->getNextOp();
                    break;
                case Op::O_NEGATIVELOOKBEHIND:
                    if (0 <= match(context, tmpOp->getChild(), offset, -1))
                        doReturn = -1;
                    else
                        tmpOp = tmpOp->getNextOp();
                    break;
                case Op::O_INDEPENDENT:
                case Op::O_MODIFIER:
                {
                    int ret = (tmpOp->getOpType() == Op::O_INDEPENDENT)
                            ? match(context, tmpOp->getChild(), offset, direction)
                            : matchModifier(context, tmpOp, offset, direction);
                    if (ret < 0)
                        doReturn = ret;
                    else {
                        offset = ret;
                        tmpOp = tmpOp->getNextOp();
                    }
                }
                break;
                case Op::O_CONDITION:
                    if (tmpOp->getRefNo() >= fNoGroups)
                        doReturn = -1;
                    else
                    {
                        if (matchCondition(context, tmpOp, offset, direction))
                            tmpOp = tmpOp->getYesFlow();
                        else
                            if (tmpOp->getNoFlow() != 0)
                                tmpOp = tmpOp->getNoFlow();
                            else
                                tmpOp = tmpOp->getNextOp();
                    }
                    break;
            }
        }
        if (doReturn != -5) {
            if (opStack==NULL || opStack->size() == 0)
                return doReturn;
            RE_RuntimeContext ctx = opStack->pop();
            tmpOp = ctx.op_;
            offset = ctx.offs_;
            if (tmpOp->getOpType() == Op::O_CLOSURE) {
                XMLInt32 id = tmpOp->getData();
                if (id >= 0) {
                    // loop has ended, reset the status for this closure
                    context->fOffsets[id] = -1;
                }
            }
            if (tmpOp->getOpType() == Op::O_CLOSURE || tmpOp->getOpType() == Op::O_QUESTION) {
                if (doReturn >= 0)
                    return doReturn;
            }
            tmpOp = tmpOp->getNextOp();
        }
    }

    return (int)offset;
}

bool RegularExpression::matchChar(Context* const context,
                                  const XMLInt32 ch, XMLSize_t& offset,
                                  const short direction, const bool ignoreCase) const
{
    if(direction < 0 && offset==0)
        return false;

    XMLSize_t tmpOffset = direction > 0 ? offset : offset - 1;

    if (tmpOffset >= context->fLimit)
        return false;

    XMLInt32 strCh = 0;

    if (!context->nextCh(strCh, tmpOffset, direction))
        return false;

    bool match = ignoreCase ? matchIgnoreCase(ch, strCh)
		                    : (ch == strCh);
    if (!match)
        return false;

    offset = (direction > 0) ? ++tmpOffset : tmpOffset;

    return true;
}

bool RegularExpression::matchDot(Context* const context, XMLSize_t& offset,
                                 const short direction) const
{
    if(direction < 0 && offset==0)
        return false;

    XMLSize_t tmpOffset = direction > 0 ? offset : offset - 1;

    if (tmpOffset >= context->fLimit)
        return false;

    XMLInt32 strCh = 0;

    if (!context->nextCh(strCh, tmpOffset, direction))
        return false;

    if (!isSet(context->fOptions, SINGLE_LINE)) {

        if (direction > 0 && RegxUtil::isEOLChar(strCh))
            return false;

        if (direction <= 0 && !RegxUtil::isEOLChar(strCh) )
            return false;
    }

    offset = (direction > 0) ? ++tmpOffset : tmpOffset;
    return true;
}

bool RegularExpression::matchRange(Context* const context, const Op* const op,
                                   XMLSize_t& offset, const short direction,
                                   const bool ignoreCase) const
{
    if(direction < 0 && offset==0)
        return false;

    XMLSize_t tmpOffset = direction > 0 ? offset : offset - 1;

    if (tmpOffset >= context->fLimit)
        return false;

    XMLInt32 strCh = 0;

    if (!context->nextCh(strCh, tmpOffset, direction))
        return false;

    RangeToken* tok = (RangeToken *) op->getToken();
    bool match = false;

    if (ignoreCase) {
        tok = tok->getCaseInsensitiveToken(fTokenFactory);
    }

    match = tok->match(strCh);

    if (!match)
        return false;

    offset = (direction > 0) ? ++tmpOffset : tmpOffset;

    return true;
}

bool RegularExpression::matchAnchor(Context* const context, const XMLInt32 ch,
                                    const XMLSize_t offset) const
{
    switch ((XMLCh) ch) {
    case chLatin_A:
        if (offset != context->fStart)
            return false;
        break;
    case chLatin_B:
        if (context->fLength == 0)
            break;
        {
            wordType after = getWordType(context, context->fString, context->fStart,
                                         context->fLimit, offset);
            if (after == wordTypeIgnore
                || after == getPreviousWordType(context, context->fString,
                                                context->fStart,
                                                context->fLimit, offset))
                break;
        }
        return false;
    case chLatin_b:
        if (context->fLength == 0)
            return false;
        {
            wordType after = getWordType(context, context->fString, context->fStart,
                                         context->fLimit, offset);
            if (after == wordTypeIgnore
                || after == getPreviousWordType(context, context->fString,
                                                context->fStart,
                                                context->fLimit, offset))
                return false;
        }
        break;
    case chLatin_Z:
    case chDollarSign:
        if ( (XMLCh) ch == chDollarSign && isSet(context->fOptions, MULTIPLE_LINE)) {
            if (!(offset == context->fLimit || (offset < context->fLimit
                && RegxUtil::isEOLChar(context->fString[offset]))))
                return false;
        }
        else {

            if (!(offset == context->fLimit
                || (offset+1 == context->fLimit
                    && RegxUtil::isEOLChar(context->fString[offset]))
                || (offset+2 == context->fLimit
                    && context->fString[offset] == chCR
                    && context->fString[offset+1] == chLF)))
                return false;
        }
        break;
    case chLatin_z:
        if (offset != context->fLimit)
            return false;
        break;
    case chAt:
    case chCaret:
        if ( (XMLCh) ch == chCaret && !isSet(context->fOptions, MULTIPLE_LINE)) {

            if (offset != context->fStart)
                return false;
        }
        else {

            if (!(offset == context->fStart || (offset > context->fStart
                && RegxUtil::isEOLChar(context->fString[offset-1]))))
                return false;
        }
        break;
    case chOpenAngle:
        if (context->fLength == 0 || offset == context->fLimit)
            return false;

        if (getWordType(context, context->fString, context->fStart, context->fLimit,
                        offset) != wordTypeLetter
            || getPreviousWordType(context, context->fString, context->fStart,
                                   context->fLimit, offset) != wordTypeOther)
            return false;
        break;
    case chCloseAngle:
        if (context->fLength == 0 || offset == context->fStart)
            return false;

        if (getWordType(context, context->fString, context->fStart, context->fLimit,
                        offset) != wordTypeOther
            || getPreviousWordType(context, context->fString, context->fStart,
                                   context->fLimit, offset) != wordTypeLetter)
            return false;
        break;
    }

    return true;
}

bool RegularExpression::matchBackReference(Context* const context,
                                           const XMLInt32 refNo, XMLSize_t& offset,
                                           const short direction,
                                           const bool ignoreCase) const
{
    if (refNo <=0 || refNo >= fNoGroups)
        ThrowXMLwithMemMgr(IllegalArgumentException, XMLExcepts::Regex_BadRefNo, context->fMemoryManager);

    if (context->fMatch->getStartPos(refNo) < 0
        || context->fMatch->getEndPos(refNo) < 0)
        return false;

    int start = context->fMatch->getStartPos(refNo);
    int length = context->fMatch->getEndPos(refNo) - start;

    if(direction < 0 && (int)offset<length)
        return false;

    XMLSize_t tmpOffset = (direction > 0) ? offset : offset - length;

    if (int(context->fLimit - tmpOffset) < length)
        return false;

    bool match = ignoreCase
                            ? XMLString::regionIMatches(context->fString,(int)tmpOffset,
                                                        context->fString,start,length)
                            : XMLString::regionMatches(context->fString, (int)tmpOffset,
                                                       context->fString, start,length);

    if (!match)
        return false;

    offset = (direction > 0) ? offset + length : offset - length;
    return true;
}

bool RegularExpression::matchString(Context* const context,
                                    const XMLCh* const literal, XMLSize_t& offset,
                                    const short direction, const bool ignoreCase) const
{
    XMLSize_t length = XMLString::stringLen(literal);
    if(direction < 0 && offset<length)
        return false;

    XMLSize_t tmpOffset = (direction > 0) ? offset : offset - length;

    if (context->fLimit - tmpOffset < length)
        return false;

    bool match = ignoreCase
                            ? XMLString::regionIMatches(context->fString, (int)tmpOffset,
                                                        literal, 0, length)
                            : XMLString::regionMatches(context->fString, (int)tmpOffset,
                                                       literal, 0, length);

    if (match) {
        offset = direction > 0 ? offset + length : offset - length;
    }

    return match;
}

int RegularExpression::matchCapture(Context* const context, const Op* const op,
                                    XMLSize_t offset, const short direction) const
{
    // No check is made for nullness of fMatch as the function is only called if
    // fMatch is not null.
    XMLInt32 index = op->getData();
    int save = (index > 0) ? context->fMatch->getStartPos(index)
                           : context->fMatch->getEndPos(-index);

    if (index > 0) {

        context->fMatch->setStartPos(index, (int)offset);
        int ret = match(context, op->getNextOp(), offset, direction);
        if (ret < 0)
            context->fMatch->setStartPos(index, save);
            return ret;
    }

    context->fMatch->setEndPos(-index, (int)offset);
    int ret = match(context, op->getNextOp(), offset, direction);
    if (ret < 0)
        context->fMatch->setEndPos(-index, save);
    return ret;
}

int RegularExpression::matchUnion(Context* const context,
                                   const Op* const op, XMLSize_t offset,
                                   const short direction) const
{
    unsigned int opSize = op->getSize();

    Context bestResultContext;
    int bestResult=-1;
    for(unsigned int i=0; i < opSize; i++) {
        Context tmpContext(context);
        int ret = match(&tmpContext, op->elementAt(i), offset, direction);
        if (ret >= 0 && (XMLSize_t)ret <= context->fLimit && ret>bestResult)
        {
            bestResult=ret;
            bestResultContext=tmpContext;
            // exit early, if we reached the end of the string
            if((XMLSize_t)ret == context->fLimit)
                break;
        }
    }
    if(bestResult!=-1)
        *context=bestResultContext;
    return bestResult;
}


bool RegularExpression::matchCondition(Context* const context,
                                              const Op* const op, XMLSize_t offset,
                                              const short direction) const
{

    int refNo = op->getRefNo();
    if ( refNo > 0)
        return (context->fMatch->getStartPos(refNo) >= 0
                && context->fMatch->getEndPos(refNo) >= 0);

    return (0 <= match(context, op->getConditionFlow(), offset, direction));
}

int RegularExpression::parseOptions(const XMLCh* const options)
{

    if (options == 0)
        return 0;

    int opts = 0;
    XMLSize_t length = XMLString::stringLen(options);

    for (XMLSize_t i=0; i < length; i++) {

        int v = getOptionValue(options[i]);

        if (v == 0)
            ThrowXMLwithMemMgr1(ParseException, XMLExcepts::Regex_UnknownOption, options, fMemoryManager);

        opts |= v;
    }

    return opts;
}

void RegularExpression::compile(const Token* const token) {

    if (fOperations != 0)
        return;

    fNoClosures = 0;
    fOperations = compile(token, 0, false);
}

Op* RegularExpression::compile(const Token* const token, Op* const next,
                               const bool reverse)
{

    Op* ret = 0;

    const Token::tokType tokenType = token->getTokenType();

    switch(tokenType) {
    case Token::T_DOT:
    case Token::T_CHAR:
    case Token::T_ANCHOR:
    case Token::T_RANGE:
    case Token::T_NRANGE:
    case Token::T_STRING:
    case Token::T_BACKREFERENCE:
    case Token::T_EMPTY:
        ret = compileSingle(token, next, tokenType);
        break;
    case Token::T_CONCAT:
        ret = compileConcat(token, next, reverse);
        break;
    case Token::T_UNION:
        ret = compileUnion(token, next, reverse);
        break;
    case Token::T_CLOSURE:
    case Token::T_NONGREEDYCLOSURE:
        ret = compileClosure(token, next, reverse, tokenType);
        break;
    case Token::T_PAREN:
        ret = compileParenthesis(token, next, reverse);
        break;
    case Token::T_LOOKAHEAD:
    case Token::T_NEGATIVELOOKAHEAD:
        ret = compileLook(token, next, false, tokenType);
        break;
    case Token::T_LOOKBEHIND:
    case Token::T_NEGATIVELOOKBEHIND:
        ret = compileLook(token, next, true, tokenType);
        break;
    case Token::T_INDEPENDENT:
    case Token::T_MODIFIERGROUP:
        ret = compileLook(token, next, reverse, tokenType);
        break;
    case Token::T_CONDITION:
        ret = compileCondition(token, next, reverse);
        break;
    default:
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Regex_UnknownTokenType, fMemoryManager);
        break; // this line to be deleted
    }

    return ret;
}

/*
 * Prepares for matching. This method is called during construction.
 */
void RegularExpression::prepare() {

    compile(fTokenTree);

    fMinLength = fTokenTree->getMinLength();
    fFirstChar = 0;

    if (!isSet(fOptions, PROHIBIT_HEAD_CHARACTER_OPTIMIZATION) &&
        !isSet(fOptions, XMLSCHEMA_MODE))							{

        RangeToken* rangeTok = fTokenFactory->createRange();
        Token::firstCharacterOptions result = fTokenTree->analyzeFirstCharacter(rangeTok, fOptions, fTokenFactory);

        if (result == Token::FC_TERMINAL) {

            rangeTok->compactRanges();
            fFirstChar = rangeTok;
        }

        rangeTok->createMap();

    	if (isSet(fOptions, IGNORE_CASE))
        {
            rangeTok->getCaseInsensitiveToken(fTokenFactory);
        }
    }

    if (fOperations != 0 && fOperations->getNextOp() == 0 &&
        (fOperations->getOpType() == Op::O_STRING ||
         fOperations->getOpType() == Op::O_CHAR) &&
         !isSet(fOptions, IGNORE_CASE) )                      {

        fFixedStringOnly = true;

        if (fOperations->getOpType() == Op::O_STRING) {
            fMemoryManager->deallocate(fFixedString);//delete [] fFixedString;
            fFixedString = XMLString::replicate(fOperations->getLiteral(), fMemoryManager);
        }
        else{

            XMLInt32 ch = fOperations->getData();

            if ( ch >= 0x10000) { // add as constant
                fMemoryManager->deallocate(fFixedString);//delete [] fFixedString;
                fFixedString = RegxUtil::decomposeToSurrogates(ch, fMemoryManager);
            }
            else {

                XMLCh* dummyStr = (XMLCh*) fMemoryManager->allocate(2 * sizeof(XMLCh));//new XMLCh[2];
                dummyStr[0] = (XMLCh) fOperations->getData();
                dummyStr[1] = chNull;
                fMemoryManager->deallocate(fFixedString);//delete [] fFixedString;
                fFixedString = dummyStr;
            }
        }

        fBMPattern = new (fMemoryManager) BMPattern(fFixedString, 256,
                                                    isSet(fOptions, IGNORE_CASE), fMemoryManager);
    }
    else if (!isSet(fOptions, XMLSCHEMA_MODE) &&
             !isSet(fOptions, PROHIBIT_FIXED_STRING_OPTIMIZATION) &&
             !isSet(fOptions, IGNORE_CASE)) {

        int fixedOpts = 0;
        Token* tok = fTokenTree->findFixedString(fOptions, fixedOpts);

        fMemoryManager->deallocate(fFixedString);//delete [] fFixedString;

        fFixedString = (tok == 0) ? 0
            : XMLString::replicate(tok->getString(), fMemoryManager);

        if (fFixedString != 0 && XMLString::stringLen(fFixedString) < 2) {

            fMemoryManager->deallocate(fFixedString);//delete [] fFixedString;
            fFixedString = 0;
        }

        if (fFixedString != 0) {

            fBMPattern = new (fMemoryManager) BMPattern(fFixedString, 256,
                                                        isSet(fixedOpts, IGNORE_CASE), fMemoryManager);
        }
    }
}

RegularExpression::wordType RegularExpression::getCharType(Context* const context, const XMLCh ch) const
{
    if (!isSet(context->fOptions, UNICODE_WORD_BOUNDARY)) {

        if (isSet(context->fOptions, USE_UNICODE_CATEGORY)) {

            if (fWordRange == 0) {

                fWordRange = fTokenFactory->getRange(fgUniIsWord);
                    if (fWordRange == 0)
                        ThrowXMLwithMemMgr1(RuntimeException, XMLExcepts::Regex_RangeTokenGetError, fgUniIsWord, context->fMemoryManager);
            }

            return fWordRange->match(ch) ? wordTypeLetter : wordTypeOther;
        }

        return RegxUtil::isWordChar(ch) ? wordTypeLetter : wordTypeIgnore;
    }

    switch (XMLUniCharacter::getType(ch)) {
    case XMLUniCharacter::UPPERCASE_LETTER:
    case XMLUniCharacter::LOWERCASE_LETTER:
    case XMLUniCharacter::TITLECASE_LETTER:
    case XMLUniCharacter::MODIFIER_LETTER:
    case XMLUniCharacter::OTHER_LETTER:
    case XMLUniCharacter::LETTER_NUMBER:
    case XMLUniCharacter::DECIMAL_DIGIT_NUMBER:
    case XMLUniCharacter::OTHER_NUMBER:
    case XMLUniCharacter::COMBINING_SPACING_MARK:
        return wordTypeLetter;
    case XMLUniCharacter::FORMAT:
    case XMLUniCharacter::NON_SPACING_MARK:
    case XMLUniCharacter::ENCLOSING_MARK:
        return wordTypeIgnore;
    case XMLUniCharacter::CONTROL:
        switch (ch) {
        case chHTab:
        case chLF:
        case chVTab:
        case chFF:
        case chCR:
            return wordTypeOther;
        default:
            return wordTypeIgnore;
        }
    }

    return wordTypeOther;
}

bool RegularExpression::doTokenOverlap(const Op* op, Token* token)
{
    if(op->getOpType()==Op::O_RANGE)
    {
        RangeToken* t1=(RangeToken*)op->getToken();
        switch(token->getTokenType())
        {
        case Token::T_CHAR:
            return t1->match(token->getChar());
        case Token::T_STRING:
            return t1->match(*token->getString());
        case Token::T_RANGE:
            {
                try
                {
                    RangeToken tempRange(Token::T_RANGE, fMemoryManager);
                    tempRange.mergeRanges(t1);
                    tempRange.intersectRanges((RangeToken*)token);
                    return !tempRange.empty();
                }
                catch(RuntimeException&)
                {
                }
                break;
            }
        default:
            break;
        }
        return true;
    }

    XMLInt32 ch=0;
    if(op->getOpType()==Op::O_CHAR)
        ch=op->getData();
    else if(op->getOpType()==Op::O_STRING)
        ch=*op->getLiteral();

    if(ch!=0)
    {
        switch(token->getTokenType())
        {
        case Token::T_CHAR:
            return token->getChar()==ch;
        case Token::T_STRING:
            return *token->getString()==ch;
        case Token::T_RANGE:
        case Token::T_NRANGE:
            return ((RangeToken*)token)->match(ch);
        default:
            break;
        }
    }
    // in any other case, there is the chance that they overlap
    return true;
}

XERCES_CPP_NAMESPACE_END

/**
  *	End of file RegularExpression.cpp
  */

