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

#ifndef SCRIPTHANDLER_H
#define SCRIPTHANDLER_H

#include <dlfcn.h>

#include "../../google/v8/v8.h"

#include "Macros.h"
#include "Process.h"
#include "JsUtils.h"
#include "ModuleCache.h"
#include "ArrayEncoder.h"
#include "ArrayDecoder.h"
#include "ModuleFinder.h"
#include "PrettyPrinter.h"
#include "ConsoleLogger.h"
#include "ModuleInterface.h"
#include "Shared/ByteBuffer.h"

#define _GET_THIS_P(name) ScriptHandler *name = reinterpret_cast<ScriptHandler *>(v8::External::Unwrap(args.Data()))
#define _ARG_STRING(arg, name) std::string name = JsUtils::toStdString(args[arg]->ToString())
#define GET_PID(name) int32_t name = args.Holder()->Get(String::New("pid"))->ToInt32()->Value();
#define GET_DESC(name) std::string name = JsUtils::toStdString(args[0]->ToString())

// Entry point for shared library modules
typedef void install_module(v8::Handle<v8::Object> obj, ModuleInterface *host);

class ScriptHandler : public Process, public ModuleInterface
{
public:
  ScriptHandler(int socket, int argc, char *argv[], const char *name);
  ~ScriptHandler();

  void run();
  void stop();

private:
  void initEnvironment();
  void initJavaScript();
  void initArgv(v8::Local<v8::Object> dest);
  void initConcurrentObject(v8::Local<v8::Object> dest);

  void handleRunAsMaster(Event *ev);
  void handleRunAsSlave(Event * ev);
  void handleRun(std::string &source);

  bool executeString(std::string text, std::string *resultString);

  ModuleCache moduleCache;
  ModuleFinder moduleFinder;

  v8::Persistent<v8::Context> context;
  v8::Handle<v8::ObjectTemplate> global;

  // Builtin functions (they are in the global namespace).
  static v8::Handle<v8::Value> staticPrint(const v8::Arguments &args);
  static v8::Handle<v8::Value> staticGetVersion(const v8::Arguments &);
  static v8::Handle<v8::Value> staticRequire(const v8::Arguments &args);
  static v8::Handle<v8::Value> staticRequireCheck(const v8::Arguments &args);

  v8::Handle<v8::Value> module_require(std::string descriptor);
  v8::Handle<v8::Value> module_require_check(std::string descriptor);

  // Require functionality for javascript file and shared libraries
  void loadSharedLib(std::string &name, v8::Handle<v8::Object> dest, bool *success);
  void requireJs(std::string name, std::string path, v8::Handle<v8::Object> module, bool *success);

  // Concurrency
  static v8::Handle<v8::Value> staticSpawn(const v8::Arguments& args);
  static v8::Handle<v8::Value> staticAlive(const v8::Arguments& args);
  static v8::Handle<v8::Value> staticKill(const v8::Arguments& args);
  static v8::Handle<v8::Value> staticJoin(const v8::Arguments& args);
  static v8::Handle<v8::Value> staticSend(const v8::Arguments& args);
  static v8::Handle<v8::Value> staticRecv(const v8::Arguments& args);
  static v8::Handle<v8::Value> staticSendProxy(const v8::Arguments& args);
  static v8::Handle<v8::Value> staticConcurrentThrow(const v8::Arguments& args);
  static v8::Handle<v8::Value> staticHardwareConcurrency(const v8::Arguments& args);
  static v8::Handle<v8::Value> futureValueGetter(v8::Local<v8::String> prop, const v8::AccessorInfo& info);
  static v8::Handle<v8::Value> futureCtor(const v8::Arguments& args);

  v8::Handle<v8::Value> spawn(const v8::Arguments& args);
  v8::Handle<v8::Value> alive(const v8::Arguments& args);
  v8::Handle<v8::Value> pkill(const v8::Arguments& args);
  v8::Handle<v8::Value> join(const v8::Arguments& args);
  v8::Handle<v8::Value> psend(const v8::Arguments& args, uint32_t *requestId = 0);
  v8::Handle<v8::Value> recv(const v8::Arguments&);
  v8::Handle<v8::Value> recvImpl(Event *message, std::string *error=0);
  v8::Handle<v8::Value> recvFuture(std::string &sender_name, pid_t sender_pid, uint32_t request_id, std::string *error);
  v8::Handle<v8::Value> concurrentThrow(const v8::Arguments& args);
};

#endif // SCRIPTHANDLER_H
