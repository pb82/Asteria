/* Copyright (c) 2011 Peter Braun
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:

 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef ARRAYDECODER_H
#define ARRAYDECODER_H

#include "Macros.h"
#include "JsUtils.h"
#include "../Serialization/Event.pb.h"

class ArrayDecoder
{
public:
  ArrayDecoder();
  v8::Handle<v8::Array> decodeArray(Event *message);

private:
  v8::Handle<v8::Array> innerDecodeArray(Event *message);
  std::string makeEvalStatement(std::string &source);

  uint32_t lengthIndex;
  uint32_t uint32Index;
  uint32_t stringIndex;
  uint32_t doubleIndex;
  uint32_t int32Index;
  uint32_t typeIndex;
  uint32_t boolIndex;
};

#endif // ARRAYDECODER_H
