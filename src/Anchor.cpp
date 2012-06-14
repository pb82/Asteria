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

#include "Include/Anchor.h"

Anchor::Anchor(int socket, int pid) : Process(socket), socket(socket), pid(pid) {
  Process::run(true);
  detached = false;
}

Anchor::~Anchor() {
  // Important: FIRST end the child process
  // THEN stop the event loop.
  endProcess();
  Process::stop();
}

bool Anchor::launch(int *socket, int *rPid, int argc, char *argv[], const char *name) {
  int sockets[2];
  if(0 > socketpair(AF_UNIX, SOCK_STREAM, 0, sockets)) {
    std::cerr << "socketpair() failure: " << errno << std::endl;
    return false;
  }

  pid_t pid = fork();

  switch(pid) {
  case -1:
    // Error forking.
    // Close sockets and return.
    close(sockets[0]);
    close(sockets[1]);
    return false;
  case 0:
    {
      // Child:
      // Start the process loop
      close(sockets[0]);
      ScriptHandler main(sockets[1], argc, argv, name);
      main.run();
      main.stop();
      close(sockets[1]);
      exit(0);
    }
  default:
    // Parent:
    // Report success and return the socket
    close(sockets[1]);
    *rPid = pid;
    *socket = sockets[0];
    return true;
  }
}

void Anchor::endProcess() {
  if(!detached) {
    // Tell the process to die
    Event *ev = new Event();
    ev->set_type(Event_EventType_ET_SUICIDE);
    pushEvent(ev);

    // Wait for it to die
    waitpid(pid, NULL, 0);

    // Stop the own event queue
    Process::stop();
    detached = true;
  }
}

void Anchor::sendRunAsSlave(std::string &parent_name, pid_t parent_pid, std::string &function) {
  Event *ev = new Event;
  ev->set_type(Event_EventType_ET_RUN_AS_SLAVE);
  ev->set_sender_name(parent_name);
  ev->set_sender_pid(parent_pid);
  ev->set_str_data(function);
  EventQueue::pushEvent(ev);
}

void Anchor::sendAck(bool ack, pid_t child_pid) {
  Event *ev = new Event;
  ev->set_type(Event_EventType_ET_GENERAL_ACK);
  ev->set_bool_data(ack);
  ev->set_uint32_data(child_pid);
  EventQueue::pushEvent(ev);
}

void Anchor::sendAck(bool ack, uint32_t req_id) {
  Event *ev = new Event;
  ev->set_type(Event_EventType_ET_GENERAL_ACK);
  ev->set_request_id(req_id);
  ev->set_bool_data(ack);
  EventQueue::pushEvent(ev);
}

void Anchor::sendAck(bool ack) {
  Event *ev = new Event;
  ev->set_type(Event_EventType_ET_GENERAL_ACK);
  ev->set_bool_data(ack);
  EventQueue::pushEvent(ev);
}

void Anchor::setFunction(std::string &function) {
  this->function = function;
}

std::string Anchor::getFunction() {
  return this->function;
}

void Anchor::nextEvent(Event **ev) {
  popEvent(ev);
}

void Anchor::sendEvent(Event *ev) {
  EventQueue::pushEvent(ev);
}

bool Anchor::hasEvent() {
  return EventQueue::hasEvent();
}

pid_t Anchor::getPid() {
  return pid;
}

int Anchor::getSocket() {
  return socket;
}
