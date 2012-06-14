#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <cerrno>
#include <string.h>
#include <time.h>
#include <sys/param.h>

#include "../../src/Include/Plugin.h"

namespace system_module {

#define READ 0
#define WRITE 1
#define DEFAULT_BUFLEN 128

/**
  * popen2() implementation: write and read from processes using pipes.
  * Thanks to http://snippets.dzone.com/posts/show/1134 where this
  * implementation was posted.
  */
pid_t popen2(const char *command, int *infp, int *outfp)
{
  int p_stdin[2], p_stdout[2];
  pid_t pid;

  if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0)
    return -1;

  pid = fork();

  if (pid < 0)
    return pid;

  else if (pid == 0) {
    close(p_stdin[WRITE]);
    dup2(p_stdin[READ], READ);
    close(p_stdout[READ]);
    dup2(p_stdout[WRITE], WRITE);

    execl("/bin/sh", "sh", "-c", command, NULL);
    perror("execl");
    exit(1);
  }

  if (infp == NULL)
    close(p_stdin[WRITE]);

  else
    *infp = p_stdin[WRITE];

  // The way it was p_stdin[read] in this program is still open
  if (outfp == NULL)
    close(p_stdout[READ]);
  else
    *outfp = p_stdout[READ];

  // as well as p_stdout[write], they're closed in the fork, but not in the original program
  //fix is ez:
  close(p_stdin[READ]); // We only write to the forks input anyway
  close(p_stdout[WRITE]); // and we only read from its output
  return pid;
}

JSFUNCTION_NOARGS(getPid) {
  return JSINTEGER(getpid());
}

JSFUNCTION_NOARGS(getParentPid) {
  return JSINTEGER(getppid());
}

JSFUNCTION_NOARGS(getCwd) {
  char buffer[MAXPATHLEN];
  if(getcwd(buffer, MAXPATHLEN) == buffer) {
    return JSSTRING(buffer);
  }

  THROW("getCwd: Error. Check getErrNo() or getErrorStr().");
  return v8::Undefined();
}

JSFUNCTION_NOARGS(getErrNo) {
  return JSINTEGER(errno);
}

JSFUNCTION_NOARGS(getErrorStr) {
  return JSSTRING(strerror(errno));
}

JSFUNCTION(getEnv) {
  JS_ASSERT(args[0]->IsString(), "getEnv: Argument must be a string.", v8::Undefined());
  ARG_STRING(0, variable);

  char *value = getenv(variable.c_str());
  if(value) {
    return JSSTRING(value);
  } else {
    return v8::Undefined();
  }
}

JSFUNCTION(setEnv) {
  JS_ASSERT(args.Length() >= 2, "setEnv: need at least 2 arguments (name|value|[overwrite]).", v8::Undefined());
  JS_ASSERT(args[0]->IsString(), "setEnv: first argument must be a string.", v8::Undefined());
  JS_ASSERT(args[1]->IsString(), "setEnv: second argument must be a string.", v8::Undefined());

  ARG_STRING(0, name);
  ARG_STRING(1, value);

  bool replace = args[2]->IsBoolean() ?  args[2]->ToBoolean()->Value() : false;
  return JSINTEGER(setenv(name.c_str(), value.c_str(), replace ? 1 : 0));
}

JSFUNCTION_NOARGS(getHostName) {
  char buffer[256];
  if(gethostname(buffer, 256) == 0) {
    return JSSTRING(buffer);
  } else {
    THROW("getCwd: Error. Check getErrNo() or getErrorStr().");
    return v8::Undefined();
  }
}

JSFUNCTION(setHostName) {
  JS_ASSERT(args.Length() == 1, "setHostName: need exactly one argument (command).", v8::Undefined());
  JS_ASSERT(args[0]->IsString(), "setHostName: argument must be a string.", v8::Undefined());

  ARG_STRING(0, hostname);
  JS_ASSERT(hostname.length() > 0, "setHostName: invalid hostname.", v8::Undefined());

  return JSINTEGER(sethostname(hostname.c_str(), hostname.length()));
}

JSFUNCTION(setCwd) {
  JS_ASSERT(args[0]->IsString(), "setCwd: argument must be a string.", v8::Undefined());
  ARG_STRING(0, path);
  return JSINTEGER(chdir(path.c_str()));
}

JSFUNCTION_NOARGS(getUid) {
  return JSINTEGER(getuid());
}

JSFUNCTION_NOARGS(getEUid) {
  return JSINTEGER(geteuid());
}

JSFUNCTION_NOARGS(getGid) {
  return JSINTEGER(getgid());
}

JSFUNCTION_NOARGS(getEGid) {
  return JSINTEGER(getegid());
}
/*
 * http://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c
 */
JSFUNCTION(execute) {
  std::string command = "";
  if(args.Length() == 0) {
    return v8::Undefined();
  } else if (args[0]->IsArray()) {
    // Function argument is an array -> shell.execute(['ls', '-la']);
    v8::Handle<v8::Array> array = v8::Handle<v8::Array>::Cast(args[0]);
    if(array->Length() == 0) {
      return v8::Undefined();
    }

    for(unsigned int index = 0; index < array->Length(); index++) {
      v8::Handle<v8::String> fraction = array->Get(index)->ToString();
      command.append(JsUtils::toStdString(fraction));
      command.append(" ");
    }
  } else {
    // Multiple function arguments: shell.execute('ls', '-la');
    for(int index  = 0; index < args.Length(); index++) {
      command.append(JsUtils::toStdString(args[index]->ToString()));
      command.append(" ");
    }
  }

  FILE* pipe = 0;
  pipe = popen(command.c_str(), "r");
  if(!pipe) {
    THROW("execute: Error. Check getErrNo() or getErrorStr().");
    return v8::Undefined();
  }

  char buffer[DEFAULT_BUFLEN];
  std::string result = "";
  while(!feof(pipe)) {
    if(fgets(buffer, DEFAULT_BUFLEN, pipe) != NULL)
    result += buffer;
  }
  pclose(pipe);

  return JSSTRING(result.c_str());
}

JSFUNCTION(sysexec) {
  JS_ASSERT(args.Length() == 1, "system: exactly one argument required.", v8::Undefined());
  JS_ASSERT(args[0]->IsString(), "system: argument must be a string.", v8::Undefined());

  ARG_STRING(0, command);
  return JSINTEGER(system(command.c_str()));
}

JSFUNCTION(jspopen2) {
  JS_ASSERT(args.Length() == 2, "popen2: need exactly two arguments.", v8::Undefined());
  JS_ASSERT((args[0]->IsString() || args[0]->IsArray()) && args[1]->IsString(), "popen2: both arguments must be strings.", v8::Undefined());

  std::string command;
  if(args[0]->IsArray()) {
    v8::Handle<v8::Array> array = v8::Handle<v8::Array>::Cast(args[0]);
    if(array->Length() == 0) {
      return v8::Undefined();
    }

    for(unsigned int index = 0; index < array->Length(); index++) {
      v8::Handle<v8::String> fraction = array->Get(index)->ToString();
      command.append(JsUtils::toStdString(fraction));
      command.append(" ");
    }
  } else {
    command = JsUtils::toStdString(args[0]->ToString());
  }

  ARG_STRING(1, parameter);

  int infp, outfp;
  if (popen2(command.c_str(), &infp, &outfp) <= 0) {
    THROW("popen2: Error executing popen2. Check getErrNo() or getErrorStr().");
    return v8::Undefined();
  }

  unsigned int bytesWritten = write(infp, parameter.c_str(), parameter.length());
  close(infp);

  if(bytesWritten != parameter.length()) {
    THROW("popen2: Error writing to pipe. Check getErrNo() or getErrorStr().");
    return v8::Undefined();
  }

  std::string result = "";
  char buffer[DEFAULT_BUFLEN];
  while(1) {
    memset(buffer, 0, DEFAULT_BUFLEN);
    unsigned int bytesRead = read(outfp, buffer, DEFAULT_BUFLEN);
    if(bytesRead <= 0) {
      break;
    }

    result += buffer;
  }

  close(outfp);
  return JSSTRING(result.c_str());
}

JSFUNCTION(getPassword) {
  JS_ASSERT(args[0]->IsString() || args[0]->IsUndefined(), "getPassword: argument must be a string or empty.", v8::Undefined());

  char *prompt = 0;
  if(args[0]->IsString()) {
    ARG_STRING(0, tmp);
    prompt = const_cast<char *>(tmp.c_str());
  }

  char *result = getpass(prompt);
  v8::Handle<v8::String> resultString = v8::String::New(result, strlen(result));
  memset(result, 0, strlen(result));

  return resultString;
}

/* long sleep, granularity: seconds */
JSFUNCTION(lsleep) {
  JS_ASSERT(args[0]->IsUint32(), "sleep: argument must be an unsigned integer.", v8::Undefined());
  ARG_UINT32(0, seconds);
  return JSINTEGER(sleep(seconds));
}

/* millisecond sleep */
/* Thanks, http://cc.byexamples.com/2007/05/25/nanosleep-is-better-than-sleep-and-usleep/ */
JSFUNCTION(msleep) {
  JS_ASSERT(args[0]->IsUint32(), "msleep: argument must be an unsigned integer.", v8::Undefined());
  ARG_UINT32(0, milliseconds);
  struct timespec req={0, 0};
  time_t sec=(int)(milliseconds/1000);
  milliseconds=milliseconds-(sec*1000);
  req.tv_sec=sec;
  req.tv_nsec=milliseconds*1000000L;

  while(nanosleep(&req,&req)==-1)
    continue;

  return v8::Undefined();
}

JSFUNCTION(stdinRead) {
  bool nl = false;
  std::string line;
  std::string total;

  while(std::cin) {
    getline(std::cin, line);
    if(nl) total.append("\n");
    total.append(line);
    nl = true;
  }

  int length = args[0]->IsUint32() ? args[0]->Uint32Value() : total.size();
  return v8::String::New(total.c_str(), length);
}

JSFUNCTION(stdoutWrite) {
  JS_ASSERT(args.Length() == 1, "stdout.write: need exactly one argument.", v8::Undefined());
  std::cout << JsUtils::toStdString(args[0]->ToString());
  return v8::Undefined();
}

JSFUNCTION(stdoutFlush) {
  JS_ASSERT(args.Length() == 0, "stdout.flush: doesen't take any arguments.", v8::Undefined());
  std::cout.flush();
  return v8::Undefined();
}

JSFUNCTION(stdinReadline)  {
  if(args.Length() > 0) {
    JS_ASSERT(args[0]->IsString(), "stdin.readline: Argument (prompt) must be a string.", v8::Undefined());
    std::cout << JsUtils::toStdString(args[0]->ToString());
  }

  std::string line;
  std::getline(std::cin,  line);
  return JSSTRING(line.c_str());
}

JSFUNCTION_NOARGS(stdinEof) {
  return v8::Boolean::New(std::cin.eof());
}


ENTRY_POINT_NOHOST(module) {
  // Get functions
  module->Set(v8::String::New("getPid"), v8::FunctionTemplate::New(getPid)->GetFunction());
  module->Set(v8::String::New("getParentPid"), v8::FunctionTemplate::New(getParentPid)->GetFunction());
  module->Set(v8::String::New("getCwd"), v8::FunctionTemplate::New(getCwd)->GetFunction());
  module->Set(v8::String::New("getErrno"), v8::FunctionTemplate::New(getErrNo)->GetFunction());
  module->Set(v8::String::New("getErrorStr"), v8::FunctionTemplate::New(getErrorStr)->GetFunction());
  module->Set(v8::String::New("getEnv"), v8::FunctionTemplate::New(getEnv)->GetFunction());
  module->Set(v8::String::New("getHostName"), v8::FunctionTemplate::New(getHostName)->GetFunction());
  module->Set(v8::String::New("getUid"), v8::FunctionTemplate::New(getUid)->GetFunction());
  module->Set(v8::String::New("getEUid"), v8::FunctionTemplate::New(getEUid)->GetFunction());
  module->Set(v8::String::New("getGid"), v8::FunctionTemplate::New(getGid)->GetFunction());
  module->Set(v8::String::New("getEGid"), v8::FunctionTemplate::New(getEGid)->GetFunction());

  // Set functions
  module->Set(v8::String::New("setEnv"), v8::FunctionTemplate::New(setEnv)->GetFunction());
  module->Set(v8::String::New("setHostName"), v8::FunctionTemplate::New(setHostName)->GetFunction());
  module->Set(v8::String::New("setCwd"), v8::FunctionTemplate::New(setCwd)->GetFunction());

  // Aux
  module->Set(v8::String::New("execute"), v8::FunctionTemplate::New(execute)->GetFunction());
  module->Set(v8::String::New("system"), v8::FunctionTemplate::New(sysexec)->GetFunction());
  module->Set(v8::String::New("popen2"), v8::FunctionTemplate::New(jspopen2)->GetFunction());
  module->Set(v8::String::New("getPassword"), v8::FunctionTemplate::New(getPassword)->GetFunction());
  module->Set(v8::String::New("sleep"), v8::FunctionTemplate::New(lsleep)->GetFunction());
  module->Set(v8::String::New("msleep"), v8::FunctionTemplate::New(msleep)->GetFunction());

  // Stdin and Stdout
  v8::Handle<v8::Object> stdin = v8::Object::New();
  v8::Handle<v8::Object> stdout = v8::Object::New();

  stdin->Set(JSSTRING("read"), v8::FunctionTemplate::New(stdinRead)->GetFunction());
  stdin->Set(JSSTRING("readLine"), v8::FunctionTemplate::New(stdinReadline)->GetFunction());
  stdin->Set(JSSTRING("eof"), v8::FunctionTemplate::New(stdinEof)->GetFunction());

  stdout->Set(JSSTRING("write"), v8::FunctionTemplate::New(stdoutWrite)->GetFunction());
  stdout->Set(JSSTRING("flush"), v8::FunctionTemplate::New(stdoutFlush)->GetFunction());

  module->Set(v8::String::New("stdin"), stdin);
  module->Set(v8::String::New("stdout"), stdout);
}

// End of namespace
}
