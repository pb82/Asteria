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

#include "Include/ArrayEncoder.h"

/** ArrayEncoder: Take a JavaScript Array (v8::Array) and serialize it into a
  * message so that it can be sent over a network socket. Information about
  * types, order and nesting is preserved. It's possible to send mixed arrays
  * as well as trees.
  */

ArrayEncoder::ArrayEncoder() { }

void ArrayEncoder::encodeArray(Event *packet, const v8::Handle<v8::Array> array) {
  // Before each array, it's length is stored.
  // This way nesting information is preserved.
  packet->add_array_length(array->Length());

  for(uint32_t cur = 0; cur < array->Length(); cur++) {
    innerEncodeArray(packet, array->Get(cur));
  }
}

void ArrayEncoder::innerEncodeArray(Event *packet, v8::Handle<v8::Value> val) {

  if(val->IsString()) {
    std::string string_data = JsUtils::toStdString(val->ToString());
    packet->add_str_array(string_data);
    packet->add_array_data(Event_DataType_DT_STRING);
    return;
  }

  else if (val->IsUint32()) {
    packet->add_uint32_array(val->Uint32Value());
    packet->add_array_data(Event_DataType_DT_UINT32);
    return;
  }

  else if (val->IsInt32()) {
    packet->add_int32_array(val->Int32Value());
    packet->add_array_data(Event_DataType_DT_INT32);
    return;
  }

  else if (val->IsNumber()) {
    packet->add_double_array(val->NumberValue());
    packet->add_array_data(Event_DataType_DT_NUMBER);
    return;
  }

  else if (val->IsBoolean()) {
    packet->add_bool_array(val->BooleanValue());
    packet->add_array_data(Event_DataType_DT_BOOL);
    return;
  }

  else if (val->IsFunction()) {
    v8::Handle<v8::Function> fun = v8::Handle<v8::Function>::Cast(val);
    std::string function = JsUtils::toStdString(fun->ToString());
    packet->add_str_array(function);
    packet->add_array_data(Event_DataType_DT_FUNCTION);
    return;
  }

  else if (val->IsArray()) {
    v8::Handle<v8::Array> array = v8::Handle<v8::Array>::Cast(val);
    packet->add_array_data(Event_DataType_DT_ARRAY);
    packet->add_array_length(array->Length());

    for(uint32_t cur = 0; cur < array->Length(); cur++) {
      innerEncodeArray(packet, array->Get(cur));
    }
    return;
  }
}
