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
 * Revision 1.2  2000/03/30 02:00:09  abagchi
 * Initial checkin of working code with Copyright Notice
 *
 */

#include "stdafx.h"
#include "xml4com.h"
#include "XMLDOMElement.h"
#include "XMLDOMAttribute.h"
#include "XMLDOMText.h"
#include "XMLDOMCDATASection.h"
#include "XMLDOMEntityReference.h"
#include "XMLDOMEntity.h"
#include "XMLDOMProcessingInstruction.h"
#include "XMLDOMComment.h"
#include "XMLDOMDocument.h"
#include "XMLDOMDocumentType.h"
#include "XMLDOMDocumentFragment.h"
#include "XMLDOMNotation.h"
#include "XMLDOMUtil.h"

const TCHAR* g_DomNodeName[] = 
{	
	_T("invalid"),
	_T("element"),
	_T("attribute"),
	_T("text"),
	_T("cdatasection"),
	_T("entityreference"),
	_T("entity"),
	_T("processinginstruction"),
	_T("comment"),
	_T("document"),
	_T("documenttype"),
	_T("documentfragment"),
	_T("notation")
};
 
const int g_DomNodeNameSize = sizeof(g_DomNodeName) / sizeof(TCHAR*);

void GetText(const DOM_Node& node, _bstr_t &text)
{
	DOM_Node::NodeType type = static_cast<DOM_Node::NodeType> (node.getNodeType());
		
	if (DOM_Node::DOCUMENT_TYPE_NODE		  == type || 
		DOM_Node::NOTATION_NODE				  == type)
		return;
	
	if (DOM_Node::CDATA_SECTION_NODE		  == type || 
		DOM_Node::COMMENT_NODE				  == type ||
		DOM_Node::PROCESSING_INSTRUCTION_NODE == type || 
		DOM_Node::TEXT_NODE					  == type) {
		text += node.getNodeValue().rawBuffer();
		return;
	}

	DOM_NodeList l =  node.getChildNodes();
	int length = l.getLength();
	if (length > 0) { 
		for (int i = 0; i < length; ++i) 
			GetText(l.item(i), text);
	}
	else
		text += node.getNodeValue().rawBuffer();
}

HRESULT wrapNode(IXMLDOMDocument *pDoc, DOM_Node& node, REFIID iid, LPVOID *pVal)
{
	HRESULT hr = S_OK;
	if (NULL == pVal)
		return E_POINTER;

	*pVal = NULL;
	short type = node.getNodeType();

	// the way we are constructing the wrappers is kind of fishy but oh well...
	// the various IBM DOM wrapper classes don't ever add any members or have
	// any v-tables so what we are doing should be safe.  There isn't any other
	// way as far as I can tell to do this....

	switch(type)
	{
	case DOM_Node::ELEMENT_NODE:
	{
		CXMLDOMElementObj *pObj = NULL;
		hr = CXMLDOMElementObj::CreateInstance(&pObj);
		if (S_OK != hr) 
			return hr;
	
		pObj->AddRef();
		pObj->SetOwnerDoc(pDoc);

		try
		{
			pObj->element = *(static_cast<DOM_Element*> (&node));
		}
		catch(...)
		{
			pObj->Release(); 
			return E_FAIL;
		}
	
		hr = pObj->QueryInterface(iid, pVal);
		if (S_OK != hr) 
			*pVal = NULL;

		pObj->Release();
		break;
	}
	case DOM_Node::ATTRIBUTE_NODE:
	{
		CXMLDOMAttributeObj *pObj = NULL;
		hr = CXMLDOMAttributeObj::CreateInstance(&pObj);
		if (S_OK != hr) 
			return hr;
	
		pObj->AddRef();
		pObj->SetOwnerDoc(pDoc);

		try
		{
			pObj->attr = *(static_cast<DOM_Attr*> (&node));
		}
		catch(...)
		{
			pObj->Release(); 
			return E_FAIL;
		}
	
		hr = pObj->QueryInterface(iid, pVal);
		if (S_OK != hr) 
			*pVal = NULL;

		pObj->Release();
		break;
	}
	case DOM_Node::TEXT_NODE:
	{
		CXMLDOMTextObj *pObj = NULL;
		hr = CXMLDOMTextObj::CreateInstance(&pObj);
		if (S_OK != hr) 
			return hr;
	
		pObj->AddRef();
		pObj->SetOwnerDoc(pDoc);

		try
		{
			pObj->text = *(static_cast<DOM_Text*> (&node));
		}
		catch(...)
		{
			pObj->Release(); 
			return E_FAIL;
		}
	
		hr = pObj->QueryInterface(iid, pVal);
		if (S_OK != hr) 
			*pVal = NULL;

		pObj->Release();
		break;
	}
	case DOM_Node::CDATA_SECTION_NODE:
	{
		CXMLDOMCDATASectionObj *pObj = NULL;
		hr = CXMLDOMCDATASectionObj::CreateInstance(&pObj);
		if (S_OK != hr) 
			return hr;
	
		pObj->AddRef();
		pObj->SetOwnerDoc(pDoc);

		try
		{
			pObj->cdataSection = *(static_cast<DOM_CDATASection*> (&node));
		}
		catch(...)
		{
			pObj->Release(); 
			return E_FAIL;
		}
	
		hr = pObj->QueryInterface(iid, pVal);
		if (S_OK != hr) 
			*pVal = NULL;

		pObj->Release();
		break;
	}
	case DOM_Node::ENTITY_REFERENCE_NODE:
	{
		CXMLDOMEntityReferenceObj *pObj = NULL;
		hr = CXMLDOMEntityReferenceObj::CreateInstance(&pObj);
		if (S_OK != hr) 
			return hr;
	
		pObj->AddRef();
		pObj->SetOwnerDoc(pDoc);

		try
		{
			pObj->entityReference = *(static_cast<DOM_EntityReference*> (&node));
		}
		catch(...)
		{
			pObj->Release(); 
			return E_FAIL;
		}
	
		hr = pObj->QueryInterface(iid, pVal);
		if (S_OK != hr) 
			*pVal = NULL;

		pObj->Release();
		break;
	}
	case DOM_Node::ENTITY_NODE:
	{
		CXMLDOMEntityObj *pObj = NULL;
		hr = CXMLDOMEntityObj::CreateInstance(&pObj);
		if (S_OK != hr) 
			return hr;
	
		pObj->AddRef();
		pObj->SetOwnerDoc(pDoc);
		
		try
		{
			pObj->entity = *(static_cast<DOM_Entity*> (&node));
		}
		catch(...)
		{
			pObj->Release(); 
			return E_FAIL;
		}
	
		hr = pObj->QueryInterface(iid, pVal);
		if (S_OK != hr) 
			*pVal = NULL;

		pObj->Release();
		break;
	}
	case DOM_Node::PROCESSING_INSTRUCTION_NODE:
	{
		CXMLDOMProcessingInstructionObj *pObj = NULL;
		hr = CXMLDOMProcessingInstructionObj::CreateInstance(&pObj);
		if (S_OK != hr) 
			return hr;
	
		pObj->AddRef();
		pObj->SetOwnerDoc(pDoc);

		try
		{
			pObj->processingInstruction = *(static_cast<DOM_ProcessingInstruction*> (&node));
		}
		catch(...)
		{
			pObj->Release(); 
			return E_FAIL;
		}
	
		hr = pObj->QueryInterface(iid, pVal);
		if (S_OK != hr) 
			*pVal = NULL;

		pObj->Release();
		break;
	}
	case DOM_Node::COMMENT_NODE:
	{
		CXMLDOMCommentObj *pObj = NULL;
		hr = CXMLDOMCommentObj::CreateInstance(&pObj);
		if (S_OK != hr) 
			return hr;
	
		pObj->AddRef();
		pObj->SetOwnerDoc(pDoc);

		try
		{
			pObj->comment = *(static_cast<DOM_Comment*> (&node));
		}
		catch(...)
		{
			pObj->Release(); 
			return E_FAIL;
		}
	
		hr = pObj->QueryInterface(iid, pVal);
		if (S_OK != hr) 
			*pVal = NULL;

		pObj->Release();
		break;
	}
	case DOM_Node::DOCUMENT_NODE:
	{
		CXMLDOMDocumentObj *pObj = NULL;
		hr = CXMLDOMDocumentObj::CreateInstance(&pObj);
		if (S_OK != hr) 
			return hr;
	
		pObj->AddRef();
		pObj->SetOwnerDoc(pDoc);

		try
		{
			pObj->m_Document = *(static_cast<DOM_Document*> (&node));
		}
		catch(...)
		{
			pObj->Release(); 
			return E_FAIL;
		}
	
		hr = pObj->QueryInterface(iid, pVal);
		if (S_OK != hr) 
			*pVal = NULL;

		pObj->Release();
		break;
	}
	case DOM_Node::DOCUMENT_TYPE_NODE:
	{
		CXMLDOMDocumentTypeObj *pObj = NULL;
		hr = CXMLDOMDocumentTypeObj::CreateInstance(&pObj);
		if (S_OK != hr) 
			return hr;
	
		pObj->AddRef();
		pObj->SetOwnerDoc(pDoc);

		try
		{
			pObj->documentType = *(static_cast<DOM_DocumentType*> (&node));
		}
		catch(...)
		{
			pObj->Release(); 
			return E_FAIL;
		}
	
		hr = pObj->QueryInterface(iid, pVal);
		if (S_OK != hr) 
			*pVal = NULL;

		pObj->Release();
		break;
	}
	case DOM_Node::DOCUMENT_FRAGMENT_NODE:
	{
		CXMLDOMDocumentFragmentObj *pObj = NULL;
		hr = CXMLDOMDocumentFragmentObj::CreateInstance(&pObj);
		if (S_OK != hr) 
			return hr;
	
		pObj->AddRef();
		pObj->SetOwnerDoc(pDoc);

		try
		{
			pObj->documentFragment = *(static_cast<DOM_DocumentFragment*> (&node));
		}
		catch(...)
		{
			pObj->Release(); 
			return E_FAIL;
		}
	
		hr = pObj->QueryInterface(iid, pVal);
		if (S_OK != hr) 
			*pVal = NULL;

		pObj->Release();
		break;
	}
	case DOM_Node::NOTATION_NODE:
	{
		CXMLDOMNotationObj *pObj = NULL;
		hr = CXMLDOMNotationObj::CreateInstance(&pObj);
		if (S_OK != hr) 
			return hr;
	
		pObj->AddRef();
		pObj->SetOwnerDoc(pDoc);

		try
		{
			pObj->notation = *(static_cast<DOM_Notation*> (&node));
		}
		catch(...)
		{
			pObj->Release(); 
			return E_FAIL;
		}
	
		hr = pObj->QueryInterface(iid, pVal);
		if (S_OK != hr) 
			*pVal = NULL;

		pObj->Release();
		break;
	}
	default:
		hr = E_NOTIMPL;
		break;
	}

	return hr;
}

void GetXML(const DOM_Node &node, _bstr_t &text)
{
	DOM_Node::NodeType type = static_cast<DOM_Node::NodeType> (node.getNodeType());
	
	if (DOM_Node::TEXT_NODE == type) {
		_bstr_t value = node.getNodeValue().rawBuffer();
		if (value.length() > 0) 
			text += value; 

		return;
	}

	_bstr_t tagName = node.getNodeName().rawBuffer();

	text += _T("<");
	text += tagName;

	DOM_NamedNodeMap attrs = node.getAttributes();

	int length = 0;
	if (attrs != 0) {
		length = attrs.getLength();
		for (int i=0; i < length; ++i) {
			DOM_Node attr = attrs.item(i);
			text += _T(" ");
			text += attr.getNodeName().rawBuffer();
			text += _T("=\"");
			text += attr.getNodeValue().rawBuffer();
			text += _T("\"");
		}
	}

	DOM_NodeList childs = node.getChildNodes();
	length = childs.getLength();
	if (length > 0) {
		text += _T(">");
		for (int i=0; i < length; ++i) {
			DOM_Node child = childs.item(i);
			GetXML(child,text);
		}
		text += _T("</");
		text += tagName;
		text += _T(">");
	}
	else
		text += _T("/>");
}
