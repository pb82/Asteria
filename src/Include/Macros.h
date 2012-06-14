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

#ifndef MACROS_H
#define MACROS_H

#include "JsUtils.h"
#include "ModuleInterface.h"

// Module entry point
// ------------------
#define ENTRY_POINT(handle) extern "C" void install_module(v8::Handle<v8::Object> handle, ModuleInterface *host)
#define ENTRY_POINT_NOHOST(handle) extern "C" void install_module(v8::Handle<v8::Object> handle, ModuleInterface *)
// ------------------


// Get and set pointers to a native object
// ---------------------------------------
#define GET_THIS_P(type,name) type *name = static_cast<type *>(args.This()->GetPointerFromInternalField(0));
#define GET_ARGS_P(idx,type,name) type *name = static_cast<type *>(args[idx]->ToObject()->GetPointerFromInternalField(0));
#define STORE_ARGS_P(idx,p) args[idx]->ToObject()->SetPointerInInternalField(0, p)
#define STORE_THIS_P(p) args.This()->SetPointerInInternalField(0, p)
// ---------------------------------------


// Standard throw js exception and assertion
// ---------------------------
#define THROW(message) v8::ThrowException(v8::Exception::Error(v8::String::New(message)))
#define THROW_AND_RET(message) return v8::ThrowException(v8::Exception::Error(v8::String::New(message)));
#define JS_ASSERT(condition, exception,ret) if(!(condition)) {\
                                        v8::ThrowException(v8::String::New(exception)); return ret; }
// ---------------------------


// Cast c++ values to v8 values
// ----------------------------
#define JSSTRING(x) v8::String::New(x)
#define JSNUMBER(x) v8::Number::New(x)
#define JSINTEGER(x) v8::Integer::New(x)
#define JSUINT32(x) v8::Uint32::New(x)
#define JSINT32(x) v8::Int32::New(x)
#define JSBOOL(x) v8::Boolean::New(x)
// ----------------------------


// Js function signatures
// ----------------------
#define JSCONSTRUCTOR(name) v8::Handle<v8::Value> name(const v8::Arguments& args)
#define JSCONSTRUCTOR_NOARGS(name) v8::Handle<v8::Value> name(const v8::Arguments&)
#define JSDESTRUCTOR(x) void x(v8::Persistent<v8::Value> object, void *parameter)
#define JSDESTRUCTOR_NONATIVE(x) void x(v8::Persistent<v8::Value> object, void *)
#define JSFUNCTION(name) v8::Handle<v8::Value> name(const v8::Arguments& args)
#define JSFUNCTION_NOARGS(name) v8::Handle<v8::Value> name(const v8::Arguments&)
// ----------------------


// Get c++ variables out of a js argument list
// -------------------------------------------
#define ARG_STRING(arg, name) std::string name = JsUtils::toStdString(args[arg]->ToString())
#define ARG_DOUBLE(arg, name) double name = args[arg]->NumberValue()
#define ARG_INTEGER(arg, name) int32_t name = args[arg]->Int32Value()
#define ARG_INT32(arg, name) int32_t name = args[arg]->Int32Value()
#define ARG_UINT8(arg, name) uint8_t name = (uint8_t)args[arg]->IntegerValue()
#define ARG_UINT32(arg, name) uint32_t name = (uint32_t)args[arg]->Uint32Value()
#define ARG_BOOL(arg, name) bool name = (uint32_t)args[arg]->BooleanValue()
// -------------------------------------------


// C++ types to string
#define CPP_BOOL_TO_CHARS(b) b ? "true" : "false"

#endif // MACROS_H
