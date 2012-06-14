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

#include "Include/ScriptHandler.h"

using namespace v8;

/**
  * TODO:
  * This file is a mess. Refactor, split up, more documentation.
  */

ScriptHandler::ScriptHandler(int socket, int argc, char *argv[], const char *name) : Process(socket, argc, argv) {
  process_name = std::string(name);
}

ScriptHandler::~ScriptHandler() {
  context.Dispose();
}

void ScriptHandler::stop() {
  Process::stop();
}

// -------------------------------------------------------------------------------------------------
// <Main loop>
// -------------------------------------------------------------------------------------------------

void ScriptHandler::run() {
  Process::run(false);

  initEnvironment();
  initJavaScript();

  Context::Scope scope(context);  
  HandleScope local_sc;

  // Init built-in namespaces and objects
  initArgv(Context::GetCurrent()->Global());
  initConcurrentObject(Context::GetCurrent()->Global());

  Event *ev = 0;
  bool terminate = false;
  while(!terminate) {
    if(popEvent(&ev)) {
      switch(ev->type()) {
      case Event_EventType_ET_SUICIDE:
        terminate = true;
        delete ev;
        continue;
      case Event_EventType_ET_RUN_AS_MASTER:
        handleRunAsMaster(ev);
        break;
      case Event_EventType_ET_RUN_AS_SLAVE:
        handleRunAsSlave(ev);
        break;
      default:
        break;
      }

      delete ev;
    } else {
      boost::this_thread::sleep(boost::posix_time::milliseconds(1));
    }
  }
}

void ScriptHandler::handleRunAsMaster(Event *ev) {
  std::string source = ev->str_data();
  handleRun(source);
}

void ScriptHandler::handleRunAsSlave(Event *ev) {
  std::string raw = ev->str_data();
  std::string source = JsUtils::makeSelfEvaluating(raw);
  handleRun(source);
}

void ScriptHandler::handleRun(std::string &source) {
  std::string result;
  if(!executeString(source, &result)) {
    log_error(result.c_str());
  }
  pushResult(result, process_name);
}

// -------------------------------------------------------------------------------------------------
// </Main loop>
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
// <Initialization>
// -------------------------------------------------------------------------------------------------

// Set the module search path.
// The standard path is always /opt/asteria/modules
void ScriptHandler::initEnvironment() {
  setenv("JS_MODULE_SEARCH_PATH", "/opt/asteria/modules", 1);
}

void ScriptHandler::initJavaScript() {
  // Manually set the stack limit to prevent resouce conflicts
  // when multiple processes are running. Found this at:
  // http://fw.hardijzer.nl/?p=97

  ResourceConstraints rc;
  uint32_t here;
  rc.set_stack_limit(&here - (512 * 1024) / sizeof(uint32_t*));
  SetResourceConstraints(&rc);

  HandleScope handle_scope;
  global = ObjectTemplate::New();

  // Bind the require, include and print functions
  global->Set(String::New("print"), FunctionTemplate::New(staticPrint));
  global->Set(String::New("getVersion"), FunctionTemplate::New(staticGetVersion));
  global->Set(String::New("require"), FunctionTemplate::New(staticRequire, External::New(this)));
  global->Set(String::New("checkModule"), FunctionTemplate::New(staticRequireCheck, External::New(this)));

  context = Context::New(NULL, global);
}

void ScriptHandler::initArgv(v8::Local<v8::Object> dest) {
  Local<Array> argvArray;
  argvArray = v8::Array::New(argc);
  for(int i = 0; i < argc; i++) {
    argvArray->Set(i, v8::String::New(argv[i]));
  }

  dest->Set(v8::String::New("argv"), argvArray);
}

void ScriptHandler::initConcurrentObject(Local<Object> dest) {
  Handle<Object> concurrent = Object::New();

  concurrent->Set(String::New("name"), String::New(process_name.c_str()));
  concurrent->Set(String::New("spawn"), v8::FunctionTemplate::New(staticSpawn, External::New(this))->GetFunction());
  concurrent->Set(String::New("isAlive"), v8::FunctionTemplate::New(staticAlive, External::New(this))->GetFunction());
  concurrent->Set(String::New("kill"), v8::FunctionTemplate::New(staticKill, External::New(this))->GetFunction());
  concurrent->Set(String::New("join"), v8::FunctionTemplate::New(staticJoin, External::New(this))->GetFunction());
  concurrent->Set(String::New("send"), v8::FunctionTemplate::New(staticSend, External::New(this))->GetFunction());
  concurrent->Set(String::New("sendForProxy"), v8::FunctionTemplate::New(staticSendProxy, External::New(this))->GetFunction());
  concurrent->Set(String::New("receive"), v8::FunctionTemplate::New(staticRecv, External::New(this))->GetFunction());
  concurrent->Set(String::New("throw"), v8::FunctionTemplate::New(staticConcurrentThrow, External::New(this))->GetFunction());
  concurrent->Set(String::New("getHardwareConcurrency"), v8::FunctionTemplate::New(staticHardwareConcurrency)->GetFunction());

  dest->Set(v8::String::New("concurrent"), concurrent);
}

// -------------------------------------------------------------------------------------------------
// </Initialization>
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
// <Execution>
// -------------------------------------------------------------------------------------------------

bool ScriptHandler::executeString(std::string text, std::string *resultString) {
  HandleScope scope;
  TryCatch try_catch;

  resultString->clear();

  // Compile the source.
  Handle<String> source = String::New(text.c_str(), text.length());
  Handle<Script> script = Script::Compile(source, v8::String::New(argv ? argv[1] : "repl"));

  if(script.IsEmpty()) {
    // Compilation error.
    PrettyPrinter::prettyPrintException(&try_catch, resultString);
    return false;
  } else {

    Handle<Value> result = script->Run();

    if(result.IsEmpty()) {
      // Runtime error      
      PrettyPrinter::prettyPrintException(&try_catch, resultString);
      return false;
    } else {
      // Success.
      // Return the result.
      if(!result->IsUndefined()) {
        Handle<String> strResult = result->ToString();
        resultString->append(JsUtils::toStdString(strResult));
        return true;
      } else {
        // Undefined is a valid result.
        resultString->append("<undefined>");
        return true;
      }
    }
  }
}

// -------------------------------------------------------------------------------------------------
// </Execution>
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
// <Implementation of Require (the module system)>
// -------------------------------------------------------------------------------------------------

Handle<Value> ScriptHandler::staticRequire(const Arguments &args) {  
  JS_ASSERT(args.Length() == 1, "require: need exactly one argument.", v8::Undefined());
  JS_ASSERT(args[0]->IsString(), "require: argument must be a string.", v8::Undefined());
  _GET_THIS_P(This);

  Handle<Value> obj = This->module_require(JsUtils::toStdString(args[0]->ToString()));
  if(obj->IsUndefined()) {
    THROW_AND_RET("require: module not found.");
  }

  return obj;
}

Handle<Value> ScriptHandler::staticRequireCheck(const v8::Arguments &args) {
  JS_ASSERT(args.Length() == 1, "require.check: need exactly one argument.", v8::Undefined());
  JS_ASSERT(args[0]->IsString(), "require.check: argument must be a string.", v8::Undefined());
  _GET_THIS_P(This);

  return This->module_require_check(JsUtils::toStdString(args[0]->ToString()));
}

Handle<Value> ScriptHandler::module_require_check(std::string descriptor) {
  return v8::Boolean::New(moduleFinder.lookup(descriptor).type != As_T_Invalid);
}

Handle<Value> ScriptHandler::module_require(std::string descriptor) {
  HandleScope scope;
  Handle<Object> module = v8::Object::New();
  as_module_t mod = moduleFinder.lookup(descriptor);
  bool success = false;

  if(moduleCache.moduleInCache(mod.key)) {
    return moduleCache.getModule(mod.key);
  } else {
    switch(mod.type) {
    case As_T_JavaScript:
      requireJs(mod.key, mod.path, module, &success);
      break;
    case As_T_SharedLib:
      loadSharedLib(mod.path, module, &success);
      break;
    default:
      return v8::Undefined();
    }
  }

  if(success) {
    moduleCache.putModule(mod.key, module);
  }

  // If require or shared lib loading failed, 'undefined' is
  // returned here
  return scope.Close(module);
}

void ScriptHandler::requireJs(std::string name, std::string path, Handle<Object> module, bool *success) {
  // Read the contents of the file
  assert(success);
  *success = false;

  std::string content;
  if(JsUtils::loadSource(path.c_str(), &content)) {

    /** Create a standalone execution environment for the module.
      * We push a new context on the stack, install the standard functions and
      * the exports and execute the script in that context.
      * Probably there's a better way to do it...
      */

    Handle<ObjectTemplate> moduleGlobal = ObjectTemplate::New();
    moduleGlobal->Set(String::New("require"), FunctionTemplate::New(staticRequire, External::New(this)));
    moduleGlobal->Set(String::New("print"), FunctionTemplate::New(staticPrint, External::New(this)));
    moduleGlobal->Set(String::New("checkModule"), FunctionTemplate::New(staticRequireCheck, External::New(this)));

    Handle<Context> moduleContext = Context::New(NULL, moduleGlobal);
    Context::Scope scope(moduleContext);

    Local<String> source = String::New(content.c_str());
    Local<Script> script = Script::New(source, String::New(path.c_str()));

    if(!script.IsEmpty()) {

      Local<Object> exports = Object::New();
      v8::Context::GetCurrent()->Global()->Set(String::New("exports"), exports);

      // Export system and concurrent namespaces to the new module
      initArgv(Context::GetCurrent()->Global());
      initConcurrentObject(Context::GetCurrent()->Global());

      // Run the script in it's isolated context and catch eventual errors
      TryCatch tc;
      Local<Value> result = script->Run();
      if(result.IsEmpty()) {
        std::string error;
        PrettyPrinter::prettyPrintException(&tc, &error);
        THROW(error.c_str());
        return;
      }

      // Collect the exported properties and append it to the module object
      Local<Array> properties = exports->GetPropertyNames();
      for(unsigned int i = 0; i < properties->Length(); i++) {
        module->Set(properties->Get(i)->ToString(), exports->Get(properties->Get(i)->ToString()));
      }

      // Prepare the module object (containing id)
      Local<Object> moduleObject = Object::New();
      moduleObject->Set(String::New("id"), String::New(name.c_str()));
      module->Set(String::New("module"), moduleObject);
      *success = true;
    }
  }
}

// Load a shared library and call it's install function
void ScriptHandler::loadSharedLib(std::string &name, v8::Handle<v8::Object> dest, bool *success) {
  assert(success);
  *success = false;

  void *handle = dlopen(name.c_str(), RTLD_NOW | RTLD_GLOBAL);
  if(!handle) {
    v8::ThrowException(v8::String::New(dlerror()));
    return;
  }

  // Reset errors
  dlerror();

  install_module* install = (install_module *) dlsym(handle, "install_module");
  const char* dlsym_error = dlerror();
  if (dlsym_error) {
    v8::ThrowException(v8::String::New(dlsym_error));
    return;
  }

  install(dest, this);
  *success = true;
  return;
}

// -------------------------------------------------------------------------------------------------
// </Implementation of Require (the module system)>
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
// <Concurrency>
// -------------------------------------------------------------------------------------------------

Handle<Value> ScriptHandler::staticSpawn(const v8::Arguments &args) {
  JS_ASSERT(args.Length() > 0, "concurrent.spawn: no arguments given.", v8::Undefined());
  JS_ASSERT(args.Length() <= 2, "concurrent.spawn: invalid number of arguments.", v8::Undefined());
  JS_ASSERT(args[0]->IsFunction(), "concurrent.spwan: first argument must be a function.", v8::Undefined());

  if(args.Length() == 2 && args[1]->IsString() && (args[1]->ToString()->Length() == 0)) {
    THROW_AND_RET("concurrent.spawn: invalid process name.");
  }

  _GET_THIS_P(This);
  return This->spawn(args);
}

Handle<Value> ScriptHandler::staticAlive(const v8::Arguments &args) {
  JS_ASSERT(args.Length() == 1, "concurrent.alive: exactly one argument is required.", v8::Undefined());
  JS_ASSERT(args[0]->IsUint32() || args[0]->IsString(), "concurrent.alive: argument mus be a string or an unsigned integer.", v8::Undefined());

  _GET_THIS_P(This);
  return This->alive(args);
}

Handle<Value> ScriptHandler::staticKill(const v8::Arguments &args) {
  JS_ASSERT(args.Length() == 1, "concurrent.kill: exactly one argument is required.", v8::Undefined());
  JS_ASSERT(args[0]->IsUint32() || args[0]->IsString(), "concurrent.kill: argument mus be a string or an unsigned integer.", v8::Undefined());

  _GET_THIS_P(This);
  return This->pkill(args);
}

Handle<Value> ScriptHandler::staticJoin(const v8::Arguments &args) {
  JS_ASSERT(args.Length() == 1, "concurrent.join: exactly one argument is required.", v8::Undefined());
  JS_ASSERT(args[0]->IsUint32() || args[0]->IsString(), "concurrent.join: argument mus be a string or an unsigned integer.", v8::Undefined());

  _GET_THIS_P(This);
  return This->join(args);
}

Handle<Value> ScriptHandler::staticSend(const v8::Arguments &args) {
  JS_ASSERT(args.Length() >= 2, "concurrent.send: at least a receiver and an argument required.", v8::Undefined());
  JS_ASSERT(args[0]->IsUint32() || args[0]->IsString(), "concurrent.send: first argument must be a receiver pid or name.", v8::Undefined());

  if(args[0]->IsString() && (args[0]->ToString()->Length() == 0)) {
    THROW_AND_RET("concurrent.send: invalid receiver name.");
  }

  _GET_THIS_P(This);
  return This->psend(args);
}

Handle<Value> ScriptHandler::futureValueGetter(v8::Local<v8::String>, const v8::AccessorInfo &info) {
  ScriptHandler *This = static_cast<ScriptHandler *>(info.Holder()->GetPointerFromInternalField(0));
  if(This) {
    Handle<Value> jsSenderName = info.Holder()->Get(String::New("sender"));
    Handle<Value> jsSenderPid = info.Holder()->Get(String::New("senderPid"));

    std::string nativeSenderName = JsUtils::toStdString(jsSenderName->ToString());
    pid_t nativeSenderPid = jsSenderPid->Uint32Value();
    uint32_t requestId = info.Holder()->GetInternalField(1)->Uint32Value();

    // The process might have thrown an exception with concurrent.throw
    std::string error = "";
    v8::Handle<v8::Object> futureValue = v8::Handle<v8::Object>::Cast(This->recvFuture(nativeSenderName, nativeSenderPid, requestId, &error));
    if(error.length() > 0) {
      info.Holder()->Delete(String::New("value"));
      info.Holder()->Set(String::New("value"), v8::Undefined());

      THROW_AND_RET(error.c_str());
    }

    /** When we fetch the value for the first time, we replace the
      * accessor call by a reference to the value-object. By doing so we
      * ensure that successive requests of the future's value always return the
      * actual value and don't call the accessor again.
      */

    info.Holder()->Delete(String::New("value"));
    info.Holder()->Set(String::New("value"), futureValue->Get(String::New("value")));
    return futureValue->Get(String::New("value"));
  } else { return v8::Undefined(); }
}

Handle<Value> ScriptHandler::staticSendProxy(const v8::Arguments &args) {
  JS_ASSERT(args.Length() >= 2, "concurrent.sendForProxy: at least a receiver and an argument required.", v8::Undefined());
  JS_ASSERT(args[0]->IsUint32() || args[0]->IsString(), "concurrent.sendForProxy: first argument must be a receiver pid or name.", v8::Undefined());

  if(args[0]->IsString() && (args[0]->ToString()->Length() == 0)) {
    THROW_AND_RET("concurrent.sendForProxy: invalid receiver name.");
  }

  _GET_THIS_P(This);

  uint32_t requestId;
  This->psend(args, &requestId);

  Persistent<FunctionTemplate> futureObj = Persistent<FunctionTemplate>::New(FunctionTemplate::New(ScriptHandler::futureCtor));

  futureObj->SetClassName(JSSTRING("future"));
  Handle<ObjectTemplate> instanceTemplate = futureObj->InstanceTemplate();
  instanceTemplate->SetInternalFieldCount(2);

  Handle<Object>instance = instanceTemplate->NewInstance();
  instance->SetPointerInInternalField(0, This);
  instance->SetInternalField(1, v8::Uint32::New(requestId));

  if(args[0]->IsUint32()) {
    instance->Set(String::New("sender"), v8::Undefined());
    instance->Set(String::New("senderPid"), args[0]);
  } else if (args[0]->IsString()) {
    instance->Set(String::New("sender"), args[0]);
    instance->Set(String::New("senderPid"), v8::Undefined());
  }

  instance->SetAccessor(String::New("value"), ScriptHandler::futureValueGetter);
  return instance;
}

Handle<Value> ScriptHandler::staticRecv(const v8::Arguments &args) {
  _GET_THIS_P(This);
  return This->recv(args);
}

Handle<Value> ScriptHandler::staticConcurrentThrow(const Arguments &args) {
  JS_ASSERT(args.Length() >= 2, "concurrent.throw: at least a receiver and an argument required.", v8::Undefined());
  JS_ASSERT(args[0]->IsUint32() || args[0]->IsString(), "concurrent.throw: first argument must be a receiver pid or name.", v8::Undefined());

  _GET_THIS_P(This);
  return This->concurrentThrow(args);
}

Handle<Value> ScriptHandler::recv(const v8::Arguments &) {
  Event *message;
  waitForData(&message);
  return recvImpl(message);
}

Handle<Value> ScriptHandler::recvFuture(std::string &sender_name, pid_t sender_pid, uint32_t request_id, std::string *error) {
  Event *data;
  waitForDataFrom(&data, sender_pid, sender_name, request_id);
  return recvImpl(data, error);
}

Handle<Value> ScriptHandler::recvImpl(Event *message, std::string *error) {
  HandleScope scope;
  Handle<Object> result = Object::New();

  result->Set(String::New("sender"), String::New(message->sender_name().c_str()));
  result->Set(String::New("senderPid"), v8::Uint32::New(message->sender_pid()));

  // Data type dispatch
  switch(message->data_type()) {
  case Event_DataType_DT_INVALID:
    delete message;
    THROW_AND_RET("concurrent.receive: Invalid data type received.");
  case Event_DataType_DT_STRING:
    result->Set(String::New("value"), String::New(message->str_data().c_str()));
    break;
  case Event_DataType_DT_UINT32:
    result->Set(String::New("value"), Uint32::New(message->uint32_data()));
    break;
  case Event_DataType_DT_INT32:
    result->Set(String::New("value"), Int32::New(message->int32_data()));
    break;
  case Event_DataType_DT_BOOL:
    result->Set(String::New("value"), Boolean::New(message->bool_data()));
    break;
  case Event_DataType_DT_NUMBER:
    result->Set(String::New("value"), Number::New(message->double_data()));
    break;
  case Event_DataType_DT_FUNCTION:
    {
      /** Compile the function on the fly and return it.
        * NOTE: the function will not be available in the global
        * scope (which is good).
        */
      std::string source = message->str_data();
      result->Set(String::New("value"), JsUtils::makeFunction(source));
      break;
    }
  case Event_DataType_DT_BINARY:
    {
      // Create a binary buffer object on the fly.
      v8::Persistent<v8::Object> binary = v8::Persistent<v8::Object>::New(v8::Handle<v8::Object>::Cast(module_require(std::string("binary"))));
      v8::Handle<v8::Function> fun = v8::Handle<v8::Function>::Cast(binary->Get(JSSTRING("byteArray")));
      v8::Handle<v8::Value> argv[1];
      v8::Handle<v8::Object> obj = fun->CallAsConstructor(0, argv)->ToObject();

      ByteBuffer *buffer = static_cast<ByteBuffer *>(obj->GetPointerFromInternalField(0));
      buffer->push((unsigned char *)message->binary_data().c_str(), message->binary_data().length());
      result->Set(String::New("value"), obj);
      break;
    }
  case Event_DataType_DT_ARRAY:
   {
      ArrayDecoder decoder;
      result->Set(String::New("value"), decoder.decodeArray(message));
      break;
   }
  case Event_DataType_DT_EXCEPTION:
    {
      if(error) {
        error->append(message->str_data());
      }
      break;
    }
  default:
    if(error) {
      error->append("concurrent.receive: Unknown data type received.");
    }
    break;
  }

  delete message;
  return scope.Close(result);
}

Handle<Value> ScriptHandler::psend(const v8::Arguments &args, uint32_t *requestId) {
  int numberOfArgs = args.Length();

  std::string receiverName = args[0]->IsString() ? JsUtils::toStdString(args[0]->ToString()) : std::string("");
  pid_t receiverPid = args[0]->IsUint32() ? args[0]->Uint32Value() : 0;

  if(receiverName.length() == 0 && receiverPid <= 0) {
    THROW_AND_RET("concurrent.send: invalid receiver.");
  }

  for(int currentArg = 1; currentArg < numberOfArgs; currentArg++) {
    // Data type dispatch
    if(args[currentArg]->IsString()) {
      _ARG_STRING(currentArg, message);
      pushStringData(message, receiverPid, receiverName, process_name);
      continue;
    }

    else if(args[currentArg]->IsUint32()) {
      ARG_UINT32(currentArg, message);
      pushUint32Data(message, receiverPid, receiverName, process_name);
      continue;
    }

    else if(args[currentArg]->IsInt32()) {
      ARG_INT32(currentArg, message);
      pushInt32Data(message, receiverPid, receiverName, process_name);
      continue;
    }

    else if(args[currentArg]->IsNumber()) {
      ARG_DOUBLE(currentArg, message);
      pushDoubleData(message, receiverPid, receiverName, process_name);
      continue;
    }

    else if(args[currentArg]->IsFunction()) {      
      v8::Handle<v8::Function> fun = v8::Handle<v8::Function>::Cast(args[currentArg]);
      std::string function = JsUtils::toStdString(fun->ToString());
      pushFunctionData(function, receiverPid, receiverName, process_name);
      continue;
    }

    else if (args[currentArg]->IsBoolean()) {
      ARG_BOOL(currentArg, message);
      pushBoolData(message, receiverPid, receiverName, process_name);
      continue;
    }

    else if (args[currentArg]->IsArray()) {
      ArrayEncoder encoder;
      Event *message = new Event;
      Handle<Array> array = Handle<Array>::Cast(args[currentArg]);
      encoder.encodeArray(message, array);
      pushArrayData(message, receiverPid, receiverName, process_name);
      continue;
    }

    else if (args[currentArg]->IsObject()) {
      GET_ARGS_P(currentArg, ByteBuffer, buffer);
      if(buffer) {
        pushBinaryData((const char *)buffer->getData(), buffer->getLength(), receiverPid, receiverName, process_name);
        continue;
      } else {
        THROW_AND_RET("send: object type not supported.");
      }
    }
  }

  Event *reply;
  waitFor(&reply, Event_EventType_ET_GENERAL_ACK);
  bool success = reply->bool_data();
  if(requestId) {
    *requestId = reply->request_id();
  }

  // reply contains now the address of an event on the
  // heap, so we must delete it.
  delete reply;

  return v8::Boolean::New(success);
}

Handle<Value> ScriptHandler::join(const v8::Arguments &args) {
  while(alive(args)->BooleanValue()) {
    boost::this_thread::sleep(boost::posix_time::millisec(1));
  }
  return v8::Undefined();
}

Handle<Value> ScriptHandler::pkill(const v8::Arguments &args) {
  Event *ev = new Event;
  ev->set_type(Event_EventType_ET_KILL_REQUEST);

  if(args[0]->IsString()) {
    ev->set_str_data(JsUtils::toStdString(args[0]->ToString()));
  } else {
    ev->set_uint32_data(args[0]->Uint32Value());
  } pushEvent(ev);

  return v8::Undefined();
}

Handle<Value> ScriptHandler::spawn(const v8::Arguments &args) {
  v8::Handle<v8::Function> fun = v8::Handle<v8::Function>::Cast(args[0]);
  std::string function = JsUtils::toStdString(fun->ToString());

  std::string childName = args[1]->IsString()
      ? JsUtils::toStdString(args[1]->ToString())
      : std::string("");

  std::string host = std::string("");

  pushSpawnMessage(host, childName, function, process_name);

  Event *reply;

  // After a call to 'waitFor' reply contains the address of an
  // event in the heap and it must be deleted.
  waitFor(&reply, Event_EventType_ET_GENERAL_ACK);
  if(reply->bool_data()) {
    // On success return the pid of the new child.
    uint32_t pid = reply->uint32_data();
    delete reply;

    return v8::Uint32::New(pid);
  } else {
    // TODO: more specific error message.
    delete reply;
    THROW_AND_RET("concurrent.spawn: failed to create process.");
  }
}

Handle<Value> ScriptHandler::alive(const v8::Arguments &args) {
  Event *ev = new Event;
  ev->set_type(Event_EventType_ET_ALIVE_REQUEST);

  // The process can be resolved by pid or by name
  if(args[0]->IsString()) {
    // Name
    ev->set_str_data(JsUtils::toStdString(args[0]->ToString()));
  } else {
    // Pid
    ev->set_uint32_data(args[0]->Uint32Value());
  }

  pushEvent(ev);
  Event *reply;
  waitFor(&reply, Event_EventType_ET_GENERAL_ACK);
  bool success = reply->bool_data();
  delete reply;

  return v8::Boolean::New(success);
}

Handle<Value> ScriptHandler::concurrentThrow(const Arguments &args) {
  std::string receiverName = args[0]->IsString() ? JsUtils::toStdString(args[0]->ToString()) : std::string("");
  pid_t receiverPid = args[0]->IsUint32() ? args[0]->Uint32Value() : 0;

  if(receiverName.length() == 0 && receiverPid <= 0) {
    THROW_AND_RET("concurrent.throw: invalid receiver.");
  }

  if(args[1]->IsString()) {
    _ARG_STRING(1, exception);
    pushException(exception, receiverPid, receiverName, process_name);
  } else if (args[1]->IsObject()) {
    std::string exceptionText = JsUtils::toStdString(args[1]->ToString());
    std::string exceptionType = JsUtils::toStdString(args[1]->ToObject()->ObjectProtoToString());

    if(exceptionType.compare("[object Error]") == 0) {
      pushException(exceptionText, receiverPid, receiverName, process_name);
    } else {
      THROW_AND_RET("concurrent.throw: argument wether string nor error.");
    }
  }

  Event *reply;
  waitFor(&reply, Event_EventType_ET_GENERAL_ACK);
  bool success = reply->bool_data();

  // reply contains now the address of an event on the
  // heap, so we must delete it.
  delete reply;

  return v8::Boolean::New(success);
}

// -------------------------------------------------------------------------------------------------
// </Concurrency>
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
// <Core functions>
// -------------------------------------------------------------------------------------------------

Handle<Value> ScriptHandler::staticPrint(const Arguments &args) {
  for(int i = 0; i < args.Length(); i++) {
    std::cout << JsUtils::toStdString(args[i]->ToString());
  } std::cout.flush();
  return v8::Undefined();
}

Handle<Value> ScriptHandler::staticGetVersion(const Arguments &) {
  return v8::String::New(v8::V8::GetVersion());
}

Handle<Value> ScriptHandler::staticHardwareConcurrency(const Arguments &) {
  return JSUINT32(boost::thread::hardware_concurrency());
}

// -------------------------------------------------------------------------------------------------
// </Core functions>
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
// </Helper functions>
// -------------------------------------------------------------------------------------------------

v8::Handle<v8::Value> ScriptHandler::futureCtor(const v8::Arguments& args) {
  if(!args.IsConstructCall()) {
    THROW("future constructor must be called with 'new'.");
  }

  return args.This();
}
