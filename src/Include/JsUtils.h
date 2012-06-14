#ifndef JSUTILS_H
#define JSUTILS_H

#include <string>
#include <fstream>
#include <cassert>
#include <iostream>

#include "../../google/v8/v8.h"

#include "Macros.h"
#include "PrettyPrinter.h"

class JsUtils {
public:
  static void forceGarbageCollection() {
    /**
      * http://athile.net/library/blog/?p=1333
      */
    for (unsigned int i = 0; i < 4096; ++i) {
      if (v8::V8::IdleNotification())
        break;
      }
  }

  static std::string toStdString(v8::Handle<v8::String> str) {
    char buffer[str->Utf8Length()];
    str->WriteUtf8(buffer, str->Utf8Length());
    buffer[str->Utf8Length()] = '\0';
    return std::string(buffer);
  }

  static std::string makeSelfEvaluating(std::string &source) {
    std::string leftParen("(");
    leftParen.append(source);
    leftParen.append(")();");
    return leftParen;
  }

  static std::string makeEvalStatement(std::string &source) {
    std::string leftParen("eval(");
    leftParen.append(source);
    leftParen.append(");");
    return leftParen;
  }

  static v8::Handle<v8::Value> makeFunction(std::string &source) {
    std::string final = makeEvalStatement(source);

    v8::HandleScope scope;
    v8::Handle<v8::String> sc = v8::String::New(final.c_str(), final.length());
    v8::Handle<v8::Script> script = v8::Script::Compile(sc);
    if(!script.IsEmpty()) {
      return scope.Close(script->Run());
    } else {
      return v8::Undefined();
    }
  }

  static bool loadSource(const char *path, std::string *source, bool ignoreFirstLine=false) {
    assert(path);
    assert(source);
    source->clear();

    std::string line;
    bool firstline = ignoreFirstLine ? true : false;
    std::ifstream in(path);
    if(in.is_open()) {
      while(in) {
        std::getline(in, line);
        if(firstline) {
          firstline = false;
          continue;
        }

        source->append(line);
        source->append("\n");
      }
    } else {
      return false;
    }

    return true;
  }  
};

#endif // JSUTILS_H
