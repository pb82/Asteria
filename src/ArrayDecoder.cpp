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

#include "Include/ArrayDecoder.h"

/** ArrayDecoder: Get a JavaScript Array (v8::Array) from a message.
  * The message stores information about types and nesting. The data
  * itself is stored in the various type arrays (strings in str_array,
  * booleans in bool_array...) of the message type (Event).
  */

using namespace v8;
using namespace google::protobuf;

ArrayDecoder::ArrayDecoder() { }

Handle<Array> ArrayDecoder::decodeArray(Event *message) {
  lengthIndex = 0;
  uint32Index = 0;
  stringIndex = 0;
  doubleIndex = 0;
  int32Index = 0;
  typeIndex = 0;
  boolIndex = 0;

  return innerDecodeArray(message);
}

Handle<Array> ArrayDecoder::innerDecodeArray(Event *message) {
  Handle<Array> tmpArray = Array::New();
  uint32_t index = 0;

  // Empty array
  if((message->array_length_size() <= 0)  ||
     (message->array_data_size() <= 0)) {
    return tmpArray;
  }

  uint32_t nextLength = message->array_length().Get(lengthIndex);
  lengthIndex++;

  while(index < nextLength) {
    Event_DataType nextType = (Event_DataType)message->array_data().Get(typeIndex);
    typeIndex++;

    switch(nextType) {
      case Event_DataType_DT_STRING:
        {
          std::string nextString = message->str_array().Get(stringIndex++);
          tmpArray->Set(index, String::New(nextString.c_str()));
          break;
        }
      case Event_DataType_DT_UINT32:
        {
          uint32_t nextUint = message->uint32_array().Get(uint32Index++);
          tmpArray->Set(index, v8::Uint32::New(nextUint));
          break;
        }
      case Event_DataType_DT_INT32:
        {
          int32_t nextInt = message->int32_array().Get(int32Index++);
          tmpArray->Set(index, v8::Int32::New(nextInt));
          break;
        }
      case Event_DataType_DT_BOOL:
        {
          bool nextBool = message->bool_array().Get(boolIndex++);
          tmpArray->Set(index, v8::Boolean::New(nextBool));
          break;
        }
      case Event_DataType_DT_NUMBER:
        {
          double nextDouble = message->double_array().Get(doubleIndex++);
          tmpArray->Set(index, v8::Number::New(nextDouble));
          break;
        }
      case Event_DataType_DT_FUNCTION:
        {
          std::string nextFunction = message->str_array().Get(stringIndex++);
          tmpArray->Set(index, JsUtils::makeFunction(nextFunction));
          break;
        }
      case Event_DataType_DT_ARRAY:
        {
          tmpArray->Set(index, innerDecodeArray(message));
          break;
        }
      default:
        break;
    }
    index++;
  }
  return tmpArray;
}
