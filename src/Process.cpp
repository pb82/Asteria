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

#include "Include/Process.h"

Process *Process::This = 0;

Process::Process(int socket, int argc, char *argv[]) : EventQueue(socket), argc(argc), argv(argv) {
  connectSignals();
}

Process::Process(int socket) : EventQueue(socket) {
  connectSignals();
}

void Process::run(bool asRelay) {
  EventQueue::start(asRelay);
}

void Process::stop() {
  EventQueue::stop();
}

// Tell the dispatcher that we were killed.
void Process::cleanup() {
  Event *ev = new Event();
  ev->set_type(Event_EventType_ET_KILLED);
  ev->set_sender_name(process_name);
  ev->set_sender_pid(getpid());
  EventQueue::sendDirect(ev);
}

void Process::onSignal(int sig) {  
  //This->stop();
  This->cleanup();
  exit(sig); 
}

// Get rid of zombies
void Process::onSigChld(int) {
  int stat;
  while(waitpid(-1, &stat, WNOHANG) > 0);
}

void Process::connectSignals() {
  signal(SIGABRT, &Process::onSignal);
  signal(SIGTERM, &Process::onSignal);
  signal(SIGINT,  &Process::onSignal);
  signal(SIGSEGV, &Process::onSignal);  
  signal(SIGCHLD, &Process::onSigChld);
  Process::This = this;
}
