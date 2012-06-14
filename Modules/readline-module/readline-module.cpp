#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "Plugin.h"

namespace readline_module {

/**
 * This module is a stub.
 * Currently it only provides the abolute minimum functionality
 * for having a more comfortable repl.
 */

JSFUNCTION(js_readline) {
  const char *prompt = 0;
  if(args[0]->IsString()) {
    prompt = JsUtils::toStdString(args[0]->ToString()).c_str();
  }

  char *line = 0;
  if(prompt) {
    line = readline(prompt);
  } else {
    line = readline("");
  }

  if(line && *line) {
    v8::Handle<v8::String> str = v8::String::New(line);
    free(line);
    return str;
  } else {
    return v8::String::New("");
  }
}

JSFUNCTION(js_add_history) {
  JS_ASSERT(args.Length() == 1,   "add_history: need exactly one argument.", v8::Undefined());
  JS_ASSERT(args[0]->IsString(),  "add_history: argument must be a string.", v8::Undefined());

  const char *line = JsUtils::toStdString(args[0]->ToString()).c_str();
  add_history(line);
  return v8::Undefined();
}

ENTRY_POINT_NOHOST(module) {
  module->Set(v8::String::New("readline"), v8::FunctionTemplate::New(js_readline)->GetFunction());
  module->Set(v8::String::New("add_history"), v8::FunctionTemplate::New(js_add_history)->GetFunction());
}

// End of namespace

}
