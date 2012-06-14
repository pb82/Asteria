#include <cstdio>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <dirent.h>

#include "../../src/Include/Plugin.h"

#define MAX_LINE_LEN 1024

namespace fs {

v8::Persistent<v8::Object> binary;

// JavaScript File-Object descrutcor
JSDESTRUCTOR_NONATIVE(file_dtor) {
  object.Dispose();
  object.Clear();
}

// JavaScript File-Object Constructor
JSCONSTRUCTOR(file_ctor) {
  if(!args.IsConstructCall()) {
    THROW("File: must construct with new");
  }

  int len = args.Length();
  std::string path;
  std::string mode;

  switch(len) {
  case 1:
    // If only one argument (the path) is given,
    // assume read mode
    if(!args[0]->IsString()) {
      THROW("File: argument must be a string");
    } else {
      path = JsUtils::toStdString(args[0]->ToString());
      mode = std::string("r");
    }
    break;
  case 2:
    if(!args[0]->IsString() && args[1]->IsString()) {
      THROW("File: both arguments must be strings");
    } else {
      path = JsUtils::toStdString(args[0]->ToString());
      mode = JsUtils::toStdString(args[1]->ToString());
    }
    break;
  default:
    THROW("File: unrecognized constructor call");
    break;
  }

  FILE *f = fopen(path.c_str(), mode.c_str());
  if(f) {
    STORE_THIS_P(f);
    // Add destructor
    v8::Persistent<v8::Value> per = v8::Persistent<v8::Value>::New(args.This());
    per.MakeWeak(0, file_dtor);

    // Add properties
    args.This()->Set(v8::String::New("path"), v8::String::New(path.c_str()));
    args.This()->Set(v8::String::New("mode"), v8::String::New(mode.c_str()));
  } else {
    THROW("File: could not open file");
  }

  return args.This();
}

JSFUNCTION(close) {
  GET_THIS_P(FILE, f);
  if(f) {
    int ret = fclose(f);
    // Prevent actions on closed
    // file
    STORE_THIS_P(0);    
    return JSINTEGER(ret);
  } else {
    THROW("File.close: File is not open");
    return v8::Undefined();
  }
}

JSFUNCTION(rewind) { 
  GET_THIS_P(FILE, f);
  if(f) {
    rewind(f);
  } else {
    THROW("File.rewind: File is not open");
  }
  return v8::Undefined();
}

JSFUNCTION(eof) {
  GET_THIS_P(FILE, f);
  if(f) {
    return JSBOOL(feof(f) != 0);
  } else {
    THROW("File.eof: File is not open");
  }
  return v8::Undefined();
}


JSFUNCTION(size) {
  GET_THIS_P(FILE, f);
  if(f) {
    std::string path = JsUtils::toStdString(args.This()->Get(v8::String::New("path"))->ToString());
    struct stat st;

    stat(path.c_str(), &st);
    return JSINTEGER(st.st_size);
  } else {
    THROW("File.size: File is not open");
    return v8::Undefined();
  }
}

JSFUNCTION(flush) {
  GET_THIS_P(FILE, f);
  if(f) {    
    fflush(f);
  } else {
    THROW("File.flush: File is not open");
  }
  return v8::Undefined();
}

JSFUNCTION(refDelete) {
  GET_THIS_P(FILE, f);
  if(f) {
    std::string path = JsUtils::toStdString(args.This()->Get(v8::String::New("path"))->ToString());
    fclose(f);
    STORE_THIS_P(0);
    return JSINTEGER(remove(path.c_str()));
  } else {
    THROW("File.delete: file is not open");
    return v8::Undefined();
  }
}

JSFUNCTION(refRename) {
  if(!args[0]->IsString()) {
    THROW("File.rename: invalid argument");
     return v8::Undefined();
  }

  GET_THIS_P(FILE, f);
  if(f) {    
    std::string oldFile = JsUtils::toStdString(args.This()->Get(v8::String::New("path"))->ToString());
    std::string mode = JsUtils::toStdString(args.This()->Get(v8::String::New("mode"))->ToString());
    ARG_STRING(0, newFile);

    fclose(f);
    STORE_THIS_P(0);
    int result = rename(oldFile.c_str(), newFile.c_str());
    f = fopen(newFile.c_str(), mode.c_str());
    if(f) {
      STORE_THIS_P(f);
      args.This()->Set(v8::String::New("path"), v8::String::New(newFile.c_str()));
      return JSINTEGER(result);
    } else {
      THROW("could not reopen file");
      return v8::Undefined();
    }
  } else {
    THROW("file is not open");
    return v8::Undefined();
  }
}

JSFUNCTION(lines) {
  GET_THIS_P(FILE, f);
  if(f) {
    v8::Handle<v8::Array> array = v8::Array::New();
    int i = 0;
    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof line, f) != 0)
    {
      array->Set(i, v8::String::New(line));
      i++;
    }
    return array;
  } else {
    THROW("File.lines: File is not open");
    return v8::Undefined();
  }
}

JSFUNCTION(readLine) {
  GET_THIS_P(FILE, f);
  if(f) {
    char line[MAX_LINE_LEN];
    int length = MAX_LINE_LEN;
    char *l = line;
    if(getline(&l, (size_t *)&length, f) == -1) {
      return v8::Undefined();
    } else {
      return JSSTRING(line);
    }

    return JSSTRING(line);
  } else {
    THROW("File.readLine: File is not open");
    return v8::Undefined();
  }
}

JSFUNCTION(read) {
  int size = 0;
  if(args[0]->IsInt32()) {
    // User provided a length to read
    size = args[0]->ToInteger()->Value();
  } else {
    // User didn't provide a length arg. Read
    // the file as a whole
    struct stat st;
    std::string path = JsUtils::toStdString(args.This()->Get(v8::String::New("path"))->ToString());
    stat(path.c_str(), &st);
    size = st.st_size;
  }
  GET_THIS_P(FILE, f);
  if(f) {
    // Create a byteArray instance
    v8::Handle<v8::Function> fun = v8::Handle<v8::Function>::Cast(binary->Get(JSSTRING("byteArray")));
    v8::Handle<v8::Value> argv[1];
    v8::Handle<v8::Object> obj = fun->CallAsConstructor(0, argv)->ToObject();

    ByteBuffer *buffer = static_cast<ByteBuffer *>(obj->GetPointerFromInternalField(0));
    buffer->readFromFile(size, f);
    return obj;
  } else {
    THROW("File.read: File is not open");
    return v8::Undefined();
  }
}

JSFUNCTION(write) {
  GET_THIS_P(FILE, f);
  if(f) {
    if(args[0]->IsString()) {
      std::string str = JsUtils::toStdString(args[0]->ToString());
      return JSINTEGER(fwrite((void *)str.c_str(), 1, str.length(), f));
    } else {
      // Try to get the native byteArray pointer from the argument object
      ByteBuffer *buffer = static_cast<ByteBuffer *>(args[0]->ToObject()->GetPointerFromInternalField(0));

      if(buffer) {
        return JSINTEGER(buffer->writeToFile(f));
      } else {
        THROW("File.write: Buffer corrupted");
        return v8::Undefined();
      }
    }
  } else {
    THROW("File.write: File is not open");    
    return v8::Undefined();
  }
}

JSFUNCTION(exists) {
  if(args[0]->IsString()) {
    ARG_STRING(0, path);
    return JSBOOL(access(path.c_str(), F_OK) != -1);
  } else {
    THROW("exists: Invalid argument");
    return v8::Undefined();
  }
}

JSFUNCTION(deleteFile) {
  if(args[0]->IsString()) {
    ARG_STRING(0, path);
    return JSINTEGER(remove(path.c_str()));
  } else {
    THROW("delete: Invalid argument");
    return v8::Undefined();
  }
}

JSFUNCTION(renameFile) {
  if(args[0]->IsString() && args[1]->IsString()) {
    ARG_STRING(0, f1);
    ARG_STRING(1, f2);
    return JSINTEGER(rename(f1.c_str(), f2.c_str()));
  } else {
    THROW("Invalid arguments");
    return v8::Undefined();
  }
}

/** Entry point
  * this method must exist in every external module and
  * is called by the module loader. Do all your export
  * here.
  */

ENTRY_POINT(module) {
  // Require binary
  binary = v8::Persistent<v8::Object>::New(v8::Handle<v8::Object>::Cast(host->module_require(std::string("binary"))));

  // Create the js file class
  v8::Persistent<v8::FunctionTemplate> file_obj =
      v8::Persistent<v8::FunctionTemplate>::New(v8::FunctionTemplate::New(file_ctor));
  file_obj->SetClassName(JSSTRING("file"));

  // Get an instance template and set the internal file count to one
  // to store a native pointer there
  v8::Handle<v8::ObjectTemplate> inst = file_obj->InstanceTemplate();
  inst->SetInternalFieldCount(1);

  // Get the prototype object of the js file class
  v8::Handle<v8::ObjectTemplate> proto = file_obj->PrototypeTemplate();
  proto->Set(JSSTRING("close"), v8::FunctionTemplate::New(close));
  proto->Set(JSSTRING("rewind"), v8::FunctionTemplate::New(rewind));
  proto->Set(JSSTRING("size"), v8::FunctionTemplate::New(size));
  proto->Set(JSSTRING("flush"), v8::FunctionTemplate::New(flush));
  proto->Set(JSSTRING("read"), v8::FunctionTemplate::New(read));
  proto->Set(JSSTRING("lines"), v8::FunctionTemplate::New(lines));
  proto->Set(JSSTRING("write"), v8::FunctionTemplate::New(write));
  proto->Set(JSSTRING("delete"), v8::FunctionTemplate::New(refDelete));
  proto->Set(JSSTRING("readLine"), v8::FunctionTemplate::New(readLine));
  proto->Set(JSSTRING("eof"), v8::FunctionTemplate::New(eof));
  proto->Set(JSSTRING("rename"), v8::FunctionTemplate::New(refRename));
  proto->Set(JSSTRING("move"), v8::FunctionTemplate::New(refRename));

  // Set the module-global functions
  module->Set(v8::String::New("exists"), v8::FunctionTemplate::New(exists)->GetFunction());
  module->Set(v8::String::New("delete"), v8::FunctionTemplate::New(deleteFile)->GetFunction());
  module->Set(v8::String::New("rename"), v8::FunctionTemplate::New(renameFile)->GetFunction());
  module->Set(v8::String::New("move"), v8::FunctionTemplate::New(renameFile)->GetFunction());

  // Add the file construction function to the module
  module->Set(JSSTRING("file"), file_obj->GetFunction());  
}

// End of namespace

}
