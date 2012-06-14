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

#ifndef MAINLOOP_H
#define MAINLOOP_H

#include <unistd.h>

#include "RequestMapper.h"
#include "../../src/Include/JsUtils.h"
#include "../../src/Include/Anchor.h"
#include "../../src/Include/ConsoleLogger.h"

struct RuntimeState {
private:
  bool terminate;
  bool refreshProcesses;

public:
  RuntimeState() :
    terminate(false),
    refreshProcesses(false) { }

  bool getTerminate() {
    return this->terminate;
  }

  void setTerminate(bool state) {
    this->terminate = state;
  }

  bool getRefreshProcesses() {
    return this->refreshProcesses;
  }

  void setRefreshProcesses(bool state) {
    this->refreshProcesses = state;
  }
};

class MainLoop
{
public:
  MainLoop(int argc, char *argv[]);
  ~MainLoop();

private:
  // The main process of execution
  Anchor *master;

  // Track processes by pid
  std::map<pid_t, Anchor*> processByPid;

  // Track processes by name
  std::map<std::string, Anchor*> processByName;

  // The requestMapper helps to identify which
  // response messages belong to which request
  // messages
  RequestMapper requestMapper;

  // Keep track of the state of the runtime.
  // (Need to exit? Need to refresh the process list?)
  RuntimeState runtimeState;

  // Main loop function
  void loop();
  void handleFinished(Anchor *process, Event *message);
  void handleForkRequest(Anchor *parent, Event *message);
  void handleAliveRequest(Anchor *process, Event *message);
  void handleKilled(Anchor *process, Event *message);
  void handleKillRequest(Event *message);
  void handleData(Anchor *process, Event *message);

  void deleteProcessByName(const std::string &processName);

  Anchor *createProcess(int *socket, int *pid, int argc, char *argv[], const char *name);
};

#endif // MAINLOOP_H
