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

#ifndef EVENTQUEUE_H
#define EVENTQUEUE_H

#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <assert.h>
#include <queue>
#include <deque>
#include <pthread.h>

#include "Ipc.h"

class EventQueue : Ipc
{
public:
  EventQueue(int socket);
  ~EventQueue();

protected:
  void start(bool asRelay);
  void stop();
  bool running();
  bool hasEvent();
  bool popEvent(Event **ev);
  void pushEvent(Event * ev);
  bool waitFor(Event_EventType type);
  bool waitFor(Event** ev, Event_EventType type);
  void waitForData(Event **data);
  void waitForDataFrom(Event **data, pid_t pid, std::string &name, uint32_t requestId);

  void pushSpawnMessage(std::string &host, std::string &name, std::string &function, std::string &thisName);
  void pushResult(std::string &result, std::string &thisName);

  void pushStringData(std::string &val,             pid_t receiver, std::string &receiverName, std::string &senderName);
  void pushInt32Data(int32_t val,                   pid_t receiver, std::string &receiverName, std::string &senderName);
  void pushUint32Data(uint32_t val,                 pid_t receiver, std::string &receiverName, std::string &senderName);
  void pushBoolData(bool val,                       pid_t receiver, std::string &receiverName, std::string &senderName);
  void pushBinaryData(const char *val, size_t size, pid_t receiver, std::string &receiverName, std::string &senderName);
  void pushDoubleData(double val,                   pid_t receiver, std::string &receiverName, std::string &senderName);
  void pushFunctionData(std::string &function,      pid_t receiver, std::string &receiverName, std::string &senderName);
  void pushArrayData(Event *message,                pid_t receiver, std::string &receiverName, std::string &senderName);
  void pushException(std::string &exception,        pid_t receiver, std::string &receiverName, std::string &senderName);

  void sendDirect(Event *ev);

private:
  void loop();
  bool data();
  bool outgoing();
  bool hasData();
  bool hasDataFrom(pid_t pid, std::string &name, int *pos, uint32_t requestId);
  Event *newEvent(Event_EventType type);
  Event *newEvent(Event_EventType type, Event_DataType dataType);

  void pushData(Event *ev, pid_t receiver, std::string &receiverName, std::string &senderName);
  void pushEvent(Event *ev, std::queue<Event *> *ref);
  bool popEvent(Event **ev, std::queue<Event *> *ref);
  void storeData(Event *ev);

  std::deque<Event *> dataIn;
  std::queue<Event *> eventsIn;
  std::queue<Event *> eventsOut;

  boost::condition_variable notifyIncoming;

  boost::mutex mutex;
  boost::thread t;

  int socket;
  bool terminate;
  bool isRelay;
};

#endif // EVENTQUEUE_H
