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

#ifndef PROCESS_H
#define PROCESS_H

#include "EventQueue.h"
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

class Process : public EventQueue
{
friend class Anchor;
public:
  Process(int socket);
  Process(int socket, int argc, char *argv[]);
  virtual ~Process() { }

  virtual void stop();
  virtual void run(bool asRelay);
  void cleanup();

protected:
  int argc;
  char **argv;
  std::string process_name;

private:
  void connectSignals();
  static Process *This;
  static void onSignal(int sig);    
  static void onSigChld(int sig);
};

#endif // PROCESS_H
