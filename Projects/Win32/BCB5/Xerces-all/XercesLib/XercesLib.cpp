//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
//   Important note about DLL memory management when your DLL uses the
//   static version of the RunTime Library:
//
//   If your DLL exports any functions that pass String objects (or structs/
//   classes containing nested Strings) as parameter or function results,
//   you will need to add the library MEMMGR.LIB to both the DLL project and
//   any other projects that use the DLL.  You will also need to use MEMMGR.LIB
//   if any other projects which use the DLL will be perfomring new or delete
//   operations on any non-TObject-derived classes which are exported from the
//   DLL. Adding MEMMGR.LIB to your project will change the DLL and its calling
//   EXE's to use the BORLNDMM.DLL as their memory manager.  In these cases,
//   the file BORLNDMM.DLL should be deployed along with your DLL.
//
//   To avoid using BORLNDMM.DLL, pass string information using "char *" or
//   ShortString parameters.
//
//   If your DLL uses the dynamic version of the RTL, you do not need to
//   explicitly add MEMMGR.LIB as this will be done implicitly for you
//---------------------------------------------------------------------------
USERC("..\..\..\..\..\src\util\Platforms\Win32\Version.rc");
USEUNIT("..\..\..\..\..\src\dom\AttrImpl.cpp");
USEUNIT("..\..\..\..\..\src\dom\AttrMapImpl.cpp");
USEUNIT("..\..\..\..\..\src\dom\AttrNSImpl.cpp");
USEUNIT("..\..\..\..\..\src\dom\CDATASectionImpl.cpp");
USEUNIT("..\..\..\..\..\src\dom\CharacterDataImpl.cpp");
USEUNIT("..\..\..\..\..\src\dom\ChildNode.cpp");
USEUNIT("..\..\..\..\..\src\dom\CommentImpl.cpp");
USEUNIT("..\..\..\..\..\src\dom\DeepNodeListImpl.cpp");
USEUNIT("..\..\..\..\..\src\dom\DocumentFragmentImpl.cpp");
USEUNIT("..\..\..\..\..\src\dom\DocumentImpl.cpp");
USEUNIT("..\..\..\..\..\src\dom\DocumentTypeImpl.cpp");
USEUNIT("..\..\..\..\..\src\dom\DOM_Attr.cpp");
USEUNIT("..\..\..\..\..\src\dom\DOM_CDATASection.cpp");
USEUNIT("..\..\..\..\..\src\dom\DOM_CharacterData.cpp");
USEUNIT("..\..\..\..\..\src\dom\DOM_Comment.cpp");
USEUNIT("..\..\..\..\..\src\dom\DOM_Document.cpp");
USEUNIT("..\..\..\..\..\src\dom\DOM_DocumentFragment.cpp");
USEUNIT("..\..\..\..\..\src\dom\DOM_DocumentType.cpp");
USEUNIT("..\..\..\..\..\src\dom\DOM_DOMException.cpp");
USEUNIT("..\..\..\..\..\src\dom\DOM_DOMImplementation.cpp");
USEUNIT("..\..\..\..\..\src\dom\DOM_Element.cpp");
USEUNIT("..\..\..\..\..\src\dom\DOM_Entity.cpp");
USEUNIT("..\..\..\..\..\src\dom\DOM_EntityReference.cpp");
USEUNIT("..\..\..\..\..\src\dom\DOM_NamedNodeMap.cpp");
USEUNIT("..\..\..\..\..\src\dom\DOM_Node.cpp");
USEUNIT("..\..\..\..\..\src\dom\DOM_NodeFilter.cpp");
USEUNIT("..\..\..\..\..\src\dom\DOM_NodeIterator.cpp");
USEUNIT("..\..\..\..\..\src\dom\DOM_NodeList.cpp");
USEUNIT("..\..\..\..\..\src\dom\DOM_Notation.cpp");
USEUNIT("..\..\..\..\..\src\dom\DOM_ProcessingInstruction.cpp");
USEUNIT("..\..\..\..\..\src\dom\DOM_Range.cpp");
USEUNIT("..\..\..\..\..\src\dom\DOM_RangeException.cpp");
USEUNIT("..\..\..\..\..\src\dom\DOM_Text.cpp");
USEUNIT("..\..\..\..\..\src\dom\DOM_TreeWalker.cpp");
USEUNIT("..\..\..\..\..\src\dom\DOM_XMLDecl.cpp");
USEUNIT("..\..\..\..\..\src\dom\DomMemDebug.cpp");
USEUNIT("..\..\..\..\..\src\dom\DOMString.cpp");
USEUNIT("..\..\..\..\..\src\dom\DStringPool.cpp");
USEUNIT("..\..\..\..\..\src\dom\ElementDefinitionImpl.cpp");
USEUNIT("..\..\..\..\..\src\dom\ElementImpl.cpp");
USEUNIT("..\..\..\..\..\src\dom\ElementNSImpl.cpp");
USEUNIT("..\..\..\..\..\src\dom\EntityImpl.cpp");
USEUNIT("..\..\..\..\..\src\dom\EntityReferenceImpl.cpp");
USEUNIT("..\..\..\..\..\src\dom\NamedNodeMapImpl.cpp");
USEUNIT("..\..\..\..\..\src\dom\NodeIDMap.cpp");
USEUNIT("..\..\..\..\..\src\dom\NodeImpl.cpp");
USEUNIT("..\..\..\..\..\src\dom\NodeIteratorImpl.cpp");
USEUNIT("..\..\..\..\..\src\dom\NodeListImpl.cpp");
USEUNIT("..\..\..\..\..\src\dom\NodeVector.cpp");
USEUNIT("..\..\..\..\..\src\dom\NotationImpl.cpp");
USEUNIT("..\..\..\..\..\src\dom\ParentNode.cpp");
USEUNIT("..\..\..\..\..\src\dom\ProcessingInstructionImpl.cpp");
USEUNIT("..\..\..\..\..\src\dom\RangeImpl.cpp");
USEUNIT("..\..\..\..\..\src\dom\RefCountedImpl.cpp");
USEUNIT("..\..\..\..\..\src\dom\TextImpl.cpp");
USEUNIT("..\..\..\..\..\src\dom\TreeWalkerImpl.cpp");
USEUNIT("..\..\..\..\..\src\dom\XMLDeclImpl.cpp");
USEUNIT("..\..\..\..\..\src\framework\LocalFileInputSource.cpp");
USEUNIT("..\..\..\..\..\src\framework\MemBufInputSource.cpp");
USEUNIT("..\..\..\..\..\src\framework\StdInInputSource.cpp");
USEUNIT("..\..\..\..\..\src\framework\URLInputSource.cpp");
USEUNIT("..\..\..\..\..\src\framework\XMLAttDef.cpp");
USEUNIT("..\..\..\..\..\src\framework\XMLAttr.cpp");
USEUNIT("..\..\..\..\..\src\framework\XMLBuffer.cpp");
USEUNIT("..\..\..\..\..\src\framework\XMLBufferMgr.cpp");
USEUNIT("..\..\..\..\..\src\framework\XMLContentModel.cpp");
USEUNIT("..\..\..\..\..\src\framework\XMLElementDecl.cpp");
USEUNIT("..\..\..\..\..\src\framework\XMLEntityDecl.cpp");
USEUNIT("..\..\..\..\..\src\framework\XMLFormatter.cpp");
USEUNIT("..\..\..\..\..\src\framework\XMLNotationDecl.cpp");
USEUNIT("..\..\..\..\..\src\framework\XMLRecognizer.cpp");
USEUNIT("..\..\..\..\..\src\framework\XMLValidator.cpp");
USEUNIT("..\..\..\..\..\src\internal\ElemStack.cpp");
USEUNIT("..\..\..\..\..\src\internal\ReaderMgr.cpp");
USEUNIT("..\..\..\..\..\src\internal\VecAttributesImpl.cpp");
USEUNIT("..\..\..\..\..\src\internal\VecAttrListImpl.cpp");
USEUNIT("..\..\..\..\..\src\internal\XMLReader.cpp");
USEUNIT("..\..\..\..\..\src\internal\XMLScanner.cpp");
USEUNIT("..\..\..\..\..\src\internal\XMLScanner2.cpp");
USEUNIT("..\..\..\..\..\src\parsers\DOMParser.cpp");
USEUNIT("..\..\..\..\..\src\parsers\SAX2XMLReaderImpl.cpp");
USEUNIT("..\..\..\..\..\src\parsers\SAXParser.cpp");
USEUNIT("..\..\..\..\..\src\sax\Dummy.cpp");
USEUNIT("..\..\..\..\..\src\sax\InputSource.cpp");
USEUNIT("..\..\..\..\..\src\sax\SAXException.cpp");
USEUNIT("..\..\..\..\..\src\sax\SAXParseException.cpp");
USEUNIT("..\..\..\..\..\src\sax2\sax2Dummy.cpp");
USEUNIT("..\..\..\..\..\src\util\Base64.cpp");
USEUNIT("..\..\..\..\..\src\util\BinFileInputStream.cpp");
USEUNIT("..\..\..\..\..\src\util\BinInputStream.cpp");
USEUNIT("..\..\..\..\..\src\util\BinMemInputStream.cpp");
USEUNIT("..\..\..\..\..\src\util\BitSet.cpp");
USEUNIT("..\..\..\..\..\src\util\HashPtr.cpp");
USEUNIT("..\..\..\..\..\src\util\HashXMLCh.cpp");
USEUNIT("..\..\..\..\..\src\util\HeaderDummy.cpp");
USEUNIT("..\..\..\..\..\src\util\HexBin.cpp");
USEUNIT("..\..\..\..\..\src\util\KVStringPair.cpp");
USEUNIT("..\..\..\..\..\src\util\MsgLoaders\Win32\Win32MsgLoader.cpp");
USEUNIT("..\..\..\..\..\src\util\Mutexes.cpp");
USEUNIT("..\..\..\..\..\src\util\NetAccessors\WinSock\BinHTTPURLInputStream.cpp");
USEUNIT("..\..\..\..\..\src\util\NetAccessors\WinSock\WinSockNetAccessor.cpp");
USEUNIT("..\..\..\..\..\src\util\Platforms\Win32\Win32PlatformUtils.cpp");
USEUNIT("..\..\..\..\..\src\util\PlatformUtils.cpp");
USEUNIT("..\..\..\..\..\src\util\QName.cpp");
USEUNIT("..\..\..\..\..\src\util\regx\ASCIIRangeFactory.cpp");
USEUNIT("..\..\..\..\..\src\util\regx\BlockRangeFactory.cpp");
USEUNIT("..\..\..\..\..\src\util\regx\BMPattern.cpp");
USEUNIT("..\..\..\..\..\src\util\regx\CharToken.cpp");
USEUNIT("..\..\..\..\..\src\util\regx\ClosureToken.cpp");
USEUNIT("..\..\..\..\..\src\util\regx\ConcatToken.cpp");
USEUNIT("..\..\..\..\..\src\util\regx\ConditionToken.cpp");
USEUNIT("..\..\..\..\..\src\util\regx\Match.cpp");
USEUNIT("..\..\..\..\..\src\util\regx\ModifierToken.cpp");
USEUNIT("..\..\..\..\..\src\util\regx\Op.cpp");
USEUNIT("..\..\..\..\..\src\util\regx\OpFactory.cpp");
USEUNIT("..\..\..\..\..\src\util\regx\ParenToken.cpp");
USEUNIT("..\..\..\..\..\src\util\regx\ParserForXMLSchema.cpp");
USEUNIT("..\..\..\..\..\src\util\regx\RangeFactory.cpp");
USEUNIT("..\..\..\..\..\src\util\regx\RangeToken.cpp");
USEUNIT("..\..\..\..\..\src\util\regx\RangeTokenMap.cpp");
USEUNIT("..\..\..\..\..\src\util\regx\RegularExpression.cpp");
USEUNIT("..\..\..\..\..\src\util\regx\RegxParser.cpp");
USEUNIT("..\..\..\..\..\src\util\regx\RegxUtil.cpp");
USEUNIT("..\..\..\..\..\src\util\regx\StringToken.cpp");
USEUNIT("..\..\..\..\..\src\util\regx\Token.cpp");
USEUNIT("..\..\..\..\..\src\util\regx\TokenFactory.cpp");
USEUNIT("..\..\..\..\..\src\util\regx\UnicodeRangeFactory.cpp");
USEUNIT("..\..\..\..\..\src\util\regx\UnionToken.cpp");
USEUNIT("..\..\..\..\..\src\util\regx\XMLRangeFactory.cpp");
USEUNIT("..\..\..\..\..\src\util\regx\XMLUniCharacter.cpp");
USEUNIT("..\..\..\..\..\src\util\StringPool.cpp");
USEUNIT("..\..\..\..\..\src\util\Transcoders\Win32\Win32TransService.cpp");
USEUNIT("..\..\..\..\..\src\util\TransService.cpp");
USEUNIT("..\..\..\..\..\src\util\XML256TableTranscoder.cpp");
USEUNIT("..\..\..\..\..\src\util\XML88591Transcoder.cpp");
USEUNIT("..\..\..\..\..\src\util\XMLASCIITranscoder.cpp");
USEUNIT("..\..\..\..\..\src\util\XMLNumber.cpp");
USEUNIT("..\..\..\..\..\src\util\XMLBigDecimal.cpp");
USEUNIT("..\..\..\..\..\src\util\XMLBigInteger.cpp");
USEUNIT("..\..\..\..\..\src\util\XMLChTranscoder.cpp");
USEUNIT("..\..\..\..\..\src\util\XMLAbstractDoubleFloat.cpp");
USEUNIT("..\..\..\..\..\src\util\XMLDouble.cpp");
USEUNIT("..\..\..\..\..\src\util\XMLDateTime.cpp");
USEUNIT("..\..\..\..\..\src\util\XMLEBCDICTranscoder.cpp");
USEUNIT("..\..\..\..\..\src\util\XMLException.cpp");
USEUNIT("..\..\..\..\..\src\util\XMLFloat.cpp");
USEUNIT("..\..\..\..\..\src\util\XMLIBM1140Transcoder.cpp");
USEUNIT("..\..\..\..\..\src\util\XMLString.cpp");
USEUNIT("..\..\..\..\..\src\util\XMLStringTokenizer.cpp");
USEUNIT("..\..\..\..\..\src\util\XMLUCSTranscoder.cpp");
USEUNIT("..\..\..\..\..\src\util\XMLUni.cpp");
USEUNIT("..\..\..\..\..\src\util\XMLUri.cpp");
USEUNIT("..\..\..\..\..\src\util\XMLURL.cpp");
USEUNIT("..\..\..\..\..\src\util\XMLUTF16Transcoder.cpp");
USEUNIT("..\..\..\..\..\src\util\XMLUTF8Transcoder.cpp");
USEUNIT("..\..\..\..\..\src\util\XMLWin1252Transcoder.cpp");
USEUNIT("..\..\..\..\..\src\validators\common\AllContentModel.cpp");
USEUNIT("..\..\..\..\..\src\validators\common\CMAny.cpp");
USEUNIT("..\..\..\..\..\src\validators\common\CMBinaryOp.cpp");
USEUNIT("..\..\..\..\..\src\validators\common\CMUnaryOp.cpp");
USEUNIT("..\..\..\..\..\src\validators\common\ContentLeafNameTypeVector.cpp");
USEUNIT("..\..\..\..\..\src\validators\common\ContentSpecNode.cpp");
USEUNIT("..\..\..\..\..\src\validators\common\DFAContentModel.cpp");
USEUNIT("..\..\..\..\..\src\validators\common\GrammarResolver.cpp");
USEUNIT("..\..\..\..\..\src\validators\common\MixedContentModel.cpp");
USEUNIT("..\..\..\..\..\src\validators\common\SimpleContentModel.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\AbstractNumericFacetValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\AbstractNumericValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\AbstractStringValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\AnySimpleTypeDatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\AnyURIDatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\Base64BinaryDatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\BooleanDatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\DatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\DatatypeValidatorFactory.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\DecimalDatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\DoubleDatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\ENTITYDatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\FloatDatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\HexBinaryDatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\IDDatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\IDREFDatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\ListDatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\NOTATIONDatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\QNameDatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\NameDatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\NCNameDatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\StringDatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\UnionDatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\DateTimeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\DateTimeDatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\DateDatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\TimeDatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\DayDatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\MonthDatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\MonthDayDatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\YearDatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\YearMonthDatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\datatype\DurationDatatypeValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\DTD\DTDAttDef.cpp");
USEUNIT("..\..\..\..\..\src\validators\DTD\DTDAttDefList.cpp");
USEUNIT("..\..\..\..\..\src\validators\DTD\DTDElementDecl.cpp");
USEUNIT("..\..\..\..\..\src\validators\DTD\DTDGrammar.cpp");
USEUNIT("..\..\..\..\..\src\validators\DTD\DTDScanner.cpp");
USEUNIT("..\..\..\..\..\src\validators\DTD\DTDValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\ComplexTypeInfo.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\GeneralAttributeCheck.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\NamespaceScope.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\SchemaAttDef.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\SchemaAttDefList.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\SchemaElementDecl.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\SchemaGrammar.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\SchemaInfo.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\SchemaSymbols.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\SchemaValidator.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\SubstitutionGroupComparator.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\TraverseSchema.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\XercesAttGroupInfo.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\XercesElementWildcard.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\XercesGroupInfo.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\XUtil.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\identity\FieldActivator.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\identity\FieldValueMap.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\identity\IC_Field.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\identity\IC_Key.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\identity\IC_KeyRef.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\identity\IC_Selector.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\identity\IC_Unique.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\identity\IdentityConstraint.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\identity\ValueStore.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\identity\ValueStoreCache.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\identity\XercesXPath.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\identity\XPathMatcher.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\identity\XPathMatcherStack.cpp");
USEUNIT("..\..\..\..\..\src\validators\schema\identity\XPathSymbols.cpp");
//---------------------------------------------------------------------------
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void*)
{
	return 1;
}
//---------------------------------------------------------------------------
