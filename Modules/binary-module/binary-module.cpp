#include <iostream>
#include <fstream>
#include "../../src/Include/Plugin.h"

namespace binary {
v8::Persistent<v8::FunctionTemplate> binary_array_template;

JSFUNCTION(concat) {
  GET_THIS_P(ByteBuffer, first);
  // Try to get the native byteArray from the argument
  ByteBuffer *second = static_cast<ByteBuffer *>(args[0]->ToObject()->GetPointerFromInternalField(0));
  first->push(second);
  return v8::Undefined();
}

JSFUNCTION(length) {
  GET_THIS_P(ByteBuffer, p);
  return JSINTEGER(p->getLength());
}

JSFUNCTION(codeAt) {
  if(!args[0]->IsUint32()) {
    THROW("ByteArray.codeAt: index must be an unsigned integer");
  }

  ARG_INTEGER(0, i);
  GET_THIS_P(ByteBuffer, p);
  return JSINTEGER(p->getByte(i));
}

JSFUNCTION(toAsciiString) {
  GET_THIS_P(ByteBuffer, p);  
  return JSSTRING(p->toAsciiString().c_str());
}

JSFUNCTION(readInt16) {
  if(!args[0]->IsInt32()) {
    THROW("ByteArray.readInt16: index must be an integer");
  }

  GET_THIS_P(ByteBuffer, p);
  ARG_INTEGER(0, i);

  return JSINTEGER(p->readInt16(i));
}

JSFUNCTION(write) {
  // Get the position. The user can provide a write position
  // as the second parameter, or he can leave it blank. In
  // that case 0 is chosen.
  GET_THIS_P(ByteBuffer, buffer);
  int position = 0;
  if(args.Length() == 2) {
    if(args[1]->IsInt32()) {
      position = args[1]->ToInteger()->Value();
    } else {
      THROW("ByteArray.write: invalid argument");
      return v8::Undefined();
    }
  }

  // Get the object to write (can be a string or a ByteArray)
  // and serialize it.
  if(args[0]->IsString()) {
    ARG_STRING(0, str);
    return JSINTEGER(buffer->write(position,(unsigned char *)str.c_str(), str.length()));
  } else if(args[0]->IsObject()) {
    v8::Handle<v8::Object> obj = v8::Handle<v8::Object>::Cast(args[0]->ToObject());
    if(binary_array_template->HasInstance(obj)) {
      ByteBuffer *obj_buffer = static_cast<ByteBuffer *>(obj->GetPointerFromInternalField(0));
      return JSINTEGER(buffer->write(0, obj_buffer));
    } else {
      THROW("ByteArray.write: invalid argument");
      return v8::Undefined();
    }
  } else {
    THROW("ByteArray.write: invalid argument");
    return v8::Undefined();
  }
}

JSCONSTRUCTOR_NOARGS(bin_ctor) {
  THROW("Don't use the binary constructor");
  return v8::Undefined();
}

JSDESTRUCTOR(binary_array_destructor) {
  if(parameter) {
    ByteBuffer *native = static_cast<ByteBuffer *>(parameter);
    delete native;
  }

  object.Dispose();
  object.Clear();
}

void construct_array(ByteBuffer *buffer, v8::Handle<v8::Array> arr) {
  int len = arr->Length();
  for(int i = 0; i < len; i++) {
    v8::Local<v8::Value> val = arr->Get(i);
    if(val->IsInt32()) {
      buffer->push((unsigned char)val->ToInteger()->Value());
    }
    if(val->IsObject()) {
      v8::Handle<v8::Object> obj = v8::Handle<v8::Object>::Cast(val);
      if(binary_array_template->HasInstance(obj)) {
        ByteBuffer *obj_buffer = reinterpret_cast<ByteBuffer *>(obj->GetPointerFromInternalField(0));
        buffer->push(obj_buffer);
      }
    }
    if(val->IsArray()) {
      construct_array(buffer, v8::Handle<v8::Array>::Cast(val));
    }
  }
}

JSCONSTRUCTOR(binary_array_ctor) {
  if(!args.IsConstructCall()) {
    THROW("ByteArray: must construct with new");
  }

  int len = args.Length();
  ByteBuffer *buffer = 0;
  switch(len) {
  case 0:
  {
    buffer = new ByteBuffer();
    STORE_THIS_P(buffer);
    break;
  }
  case 1:
  {
    if(args[0]->IsArray()) {
      buffer = new ByteBuffer();
      construct_array(buffer, v8::Handle<v8::Array>::Cast(args[0]));
    } else if(args[0]->IsInt32()) {
      buffer = new ByteBuffer((unsigned int)args[0]->ToInteger()->Value());
    } else if(args[0]->IsObject()) {
      buffer = new ByteBuffer();
      v8::Handle<v8::Object> obj = v8::Handle<v8::Object>::Cast(args[0]);
      if(binary_array_template->HasInstance(obj)) {
        ByteBuffer *obj_buffer = reinterpret_cast<ByteBuffer *>(obj->GetPointerFromInternalField(0));
        buffer->push(obj_buffer);
      } else {
        THROW("ByteArray: invalid type in constructor");
      }
    } else {
      THROW("ByteArray: invalid type in constructor");
    }
    STORE_THIS_P(buffer);
    break;
  }
  default:
    THROW("ByteArray: constructor accepts only one element");
  }

  // Add destructor
  v8::Persistent<v8::Value> per = v8::Persistent<v8::Value>::New(args.This());
  per.MakeWeak(buffer, binary_array_destructor);

  return args.This();
}

void init_binary_array(v8::Handle<v8::FunctionTemplate> templ,v8::Handle<v8::Object> module) {
  // Create the binary array class
  v8::Persistent<v8::FunctionTemplate> binary_array =
      v8::Persistent<v8::FunctionTemplate>::New(v8::FunctionTemplate::New(binary_array_ctor));
  binary_array->SetClassName(JSSTRING("byteArray"));
  binary_array_template = binary_array;

  // binaryArray inherits from binary
  binary_array->Inherit(templ);

  // Get an instance template and set the internal file count to one
  // to store a native pointer there
  v8::Handle<v8::ObjectTemplate> inst = binary_array->InstanceTemplate();
  inst->SetInternalFieldCount(1);

  // Get the prototype and set the functions
  v8::Handle<v8::ObjectTemplate> proto = binary_array->PrototypeTemplate();
  proto->Set(JSSTRING("readInt16"), v8::FunctionTemplate::New(readInt16));
  proto->Set(JSSTRING("write"), v8::FunctionTemplate::New(write));

  // Set the byteArray construction function
  module->Set(JSSTRING("byteArray"), binary_array->GetFunction());
}

ENTRY_POINT_NOHOST(module) {
  // Create the abstract binary class
  v8::Persistent<v8::FunctionTemplate> bin_obj =
      v8::Persistent<v8::FunctionTemplate>::New(v8::FunctionTemplate::New(bin_ctor));
  bin_obj->SetClassName(JSSTRING("binary"));

  // Get the prototype and set the functions for inheriting
  v8::Handle<v8::ObjectTemplate> proto = bin_obj->PrototypeTemplate();
  proto->Set(JSSTRING("concat"), v8::FunctionTemplate::New(concat));
  proto->Set(JSSTRING("length"), v8::FunctionTemplate::New(length));
  proto->Set(JSSTRING("codeAt"), v8::FunctionTemplate::New(codeAt));
  proto->Set(JSSTRING("toString"), v8::FunctionTemplate::New(toAsciiString));

  // binary contructor must not be used, but CommonJS says it must be present
  module->Set(JSSTRING("binary"), bin_obj->GetFunction());

  // Init the binaryArray (inherits from binary)
  init_binary_array(bin_obj, module);
}

// End of namespace

}
