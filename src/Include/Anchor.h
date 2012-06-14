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

#ifndef ANCHOR_H
#define ANCHOR_H

#include <sys/wait.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <cstdio>

#include "../Serialization/Event.pb.h"
#include "ScriptHandler.h"
#include "Ipc.h"

class Anchor : Process
{
public:
  ~Anchor();
  Anchor(int socket, int pid);
  static bool launch(int *socket, int *rPid, int argc, char *argv[], const char *name);

  bool hasEvent();
  void sendEvent(Event *ev);
  void nextEvent(Event **ev);
  void endProcess();

  void sendRunAsSlave(std::string &parent_name, pid_t parent_pid, std::string &function);
  void sendAck(bool ack, pid_t child_pid);
  void sendAck(bool ack, uint32_t req_id);
  void sendAck(bool ack);

  int getSocket();
  void setFunction(std::string &function);
  std::string getFunction();
  const std::string& getName();
  pid_t getPid();

private:
  void sendAck(Event *ev);
  std::string function;
  bool detached;
  int socket;
  pid_t pid;
};

#endif // ANCHOR_H
