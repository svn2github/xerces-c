/*
 * Copyright 2002,2004 The Apache Software Foundation.
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

#ifndef __TRANSCODER
#define __TRANSCODER

/* Needed on some windows machines---since MS plays funny
   games with the header files under C++ */
extern "C" {

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

/* Get rid of free and malloc defined by perl */
#undef free
#undef malloc

}


#include "xercesc/util/TransService.hpp"
#include "xercesc/util/PlatformUtils.hpp"

XERCES_CPP_NAMESPACE_BEGIN

class Transcoder {

private:
  static Transcoder* _instance;

protected:

  Transcoder();
  XMLTranscoder* UTF8_TRANSCODER;

public:

  static Transcoder* getInstance();

  ~Transcoder();

  SV* XMLString2Local(const XMLCh*);
  XMLCh* Local2XMLString(SV*);
};

XERCES_CPP_NAMESPACE_END

#endif /* __TRANSCODER */
