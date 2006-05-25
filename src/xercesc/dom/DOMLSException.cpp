/*
 * Copyright 2001-2002,2004 The Apache Software Foundation.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
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

#include "DOMLSException.hpp"
#include <xercesc/util/XMLDOMMsg.hpp>

XERCES_CPP_NAMESPACE_BEGIN

DOMLSException::DOMLSException()
: DOMException()
, code((LSExceptionCode) 0)
{      
}

DOMLSException::DOMLSException(    LSExceptionCode              exCode
                                 ,       short                  messageCode
                                 ,       MemoryManager* const   memoryManager)
: DOMException(exCode, messageCode?messageCode:XMLDOMMsg::DOMLSEXCEPTION_ERRX+exCode-DOMLSException::PARSE_ERR+1, memoryManager)
, code(exCode)
{  
}

DOMLSException::DOMLSException(const DOMLSException &other)
: DOMException(other)
, code(other.code)
{
}


DOMLSException::~DOMLSException()
{
}

XERCES_CPP_NAMESPACE_END

