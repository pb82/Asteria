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

#include "Include/PrettyPrinter.h"

using namespace v8;

// Taken from Google's Shell example.
void PrettyPrinter::prettyPrintException(v8::TryCatch *try_catch, std::string *ex_string) {
  ex_string->clear();
  HandleScope handle_scope;
  String::Utf8Value exception(try_catch->Exception());
  const char* exception_string = toCString(exception);
  Handle<Message> message = try_catch->Message();
  if (message.IsEmpty()) {
    ex_string->append(exception_string);
    ex_string->append("\n");
  } else {
    String::Utf8Value filename(message->GetScriptResourceName());
    const char* filename_string = toCString(filename);
    int linenum = message->GetLineNumber();
    ex_string->append(filename_string);
    ex_string->append(":");
    ex_string->append(intToString(linenum));
    ex_string->append(": ");
    ex_string->append(exception_string);
    ex_string->append("\n");
    String::Utf8Value sourceline(message->GetSourceLine());
    const char* sourceline_string = toCString(sourceline);
    ex_string->append(sourceline_string);
    int start = message->GetStartColumn();
    for (int i = 0; i < start; i++) {
      ex_string->append(" ");
    }
    int end = message->GetEndColumn();
    for (int i = start; i < end; i++) {
      ex_string->append("^");
    }
    ex_string->append("\n");
    String::Utf8Value stack_trace(try_catch->StackTrace());
    if (stack_trace.length() > 0) {
      const char* stack_trace_string = toCString(stack_trace);
      ex_string->append(stack_trace_string);
      ex_string->append("\n");
    }
  }
}

const char* PrettyPrinter::toCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "";
}

std::string PrettyPrinter::intToString(int number) {
  std::stringstream ss; ss << number;
  return ss.str();
}
