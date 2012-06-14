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

#include "Include/EventQueue.h"

using namespace boost;

EventQueue::EventQueue(int socket) : Ipc(socket), socket(socket), terminate(true) { }
EventQueue::~EventQueue() {
  stop();
}

// -------------------------------------------------------------------------------------------------
// <Start / Stop>
// -------------------------------------------------------------------------------------------------

void EventQueue::start(bool asRelay) {
  if(!running()) {
    isRelay = asRelay;
    terminate = false;
    t = thread(bind(&EventQueue::loop, this));
  }
}

/** Stopping the event queue:
  * set the terminate flag and wait
  * until the thread terminated properly
  */

void EventQueue::stop() {
  if(running()) {
    mutex.lock();
    terminate = true;
    mutex.unlock();
    t.join();
  }
}

// -------------------------------------------------------------------------------------------------
// </Start / Stop>
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
// <Push / Pop>
// -------------------------------------------------------------------------------------------------

bool EventQueue::popEvent(Event **ev, std::queue<Event *> *ref) {
  boost::mutex::scoped_lock lock(mutex);
  if(ref->size() == 0) return false;
  *ev = ref->front();
  ref->pop();
  return *ev != 0;
}

void EventQueue::pushEvent(Event *ev, std::queue<Event *> *ref) {
  boost::mutex::scoped_lock lock(mutex);
  ref->push(ev);
}

bool EventQueue::popEvent(Event **ev) {
  return popEvent(ev, &eventsIn);
}

void EventQueue::pushEvent(Event *ev) {
  pushEvent(ev, &eventsOut);
}

void EventQueue::storeData(Event *ev) {
  boost::mutex::scoped_lock lock(mutex);
  dataIn.push_front(ev);
}

// -------------------------------------------------------------------------------------------------
// </Push / Pop>
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
// <Shortcuts for common event signatures>
// -------------------------------------------------------------------------------------------------

Event *EventQueue::newEvent(Event_EventType type) {
  Event *ev = new Event;
  ev->set_type(type);
  return ev;
}

Event *EventQueue::newEvent(Event_EventType type, Event_DataType dataType) {
  Event *ev = newEvent(type);
  ev->set_data_type(dataType);
  return ev;
}

void EventQueue::pushResult(std::string &result, std::string &thisName) {
  Event *ev = newEvent(Event_EventType_ET_FINISHED);
  ev->set_sender_name(thisName);
  ev->set_sender_pid(getpid());
  ev->set_str_data(result);
  pushEvent(ev);
}

void EventQueue::pushSpawnMessage(std::string& host, std::string &name, std::string &function, std::string &thisName) {
  Event *ev = newEvent(Event_EventType_ET_FORK_REQUEST);
  ev->add_str_array(name);
  ev->add_str_array(function);
  ev->add_str_array(host);
  ev->set_sender_pid(getpid());
  ev->set_sender_name(thisName);
  pushEvent(ev);
}

// -------------------------------------------------------------------------------------------------
// </Shortcuts for common event signatures>
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
// <Blocking methods>
// TODO: Replace sleep with condition variable waits.
// -------------------------------------------------------------------------------------------------

bool EventQueue::waitFor(Event_EventType type) {
  Event *dummy;
  bool ret = waitFor(&dummy, type);
  // dummy contains now the address of an event in the
  // heap, so we must delete it.
  delete dummy;
  return ret;
}

bool EventQueue::waitFor(Event **ev, Event_EventType type) {
  while(1) {
    if(hasEvent()) {
      if(popEvent(ev)) {
        return (*ev)->type() == type;
      } else { return false; }
    } else { this_thread::sleep(posix_time::milliseconds(1)); }
  }
}

void EventQueue::waitForData(Event **data) {
  boost::mutex::scoped_lock lock(mutex);

  while(dataIn.size() <= 0) {
    notifyIncoming.wait(lock);
  }

  (*data) = dataIn.back();
  dataIn.pop_back();
  return;
}

void EventQueue::waitForDataFrom(Event **data, pid_t pid, std::string &name, uint32_t requestId) {
  int pos = -1;
  while(1) {
    if(hasDataFrom(pid, name, &pos, requestId)) {
      (*data) = dataIn.at(pos);
      dataIn.erase(dataIn.begin() + pos);
      return;
    } else { this_thread::sleep(posix_time::milliseconds(1)); }
  }
}

// -------------------------------------------------------------------------------------------------
// </Blocking methods>
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
// <Checks>
// -------------------------------------------------------------------------------------------------

bool EventQueue::running() {
  boost::mutex::scoped_lock lock(mutex);
  return !terminate;
}

bool EventQueue::hasEvent() {
  boost::mutex::scoped_lock lock(mutex);
  return eventsIn.size() > 0;
}

bool EventQueue::outgoing() {
  boost::mutex::scoped_lock lock(mutex);
  return eventsOut.size() > 0;
}

bool EventQueue::data() {
  fd_set r;
  FD_ZERO(&r);
  FD_SET(socket, &r);
  timeval t = { 0, 1000 };
  return select(socket + 1, &r, 0, 0, &t) > 0;
}

bool EventQueue::hasData() {
  boost::mutex::scoped_lock lock(mutex);
  return dataIn.size() > 0;
}

bool EventQueue::hasDataFrom(pid_t pid, std::string &name, int *pos, uint32_t requestId) {
  boost::mutex::scoped_lock lock(mutex);
  for(unsigned int i = 0; i < dataIn.size(); i++) {
    if(dataIn[i]->sender_name() == name ||
       dataIn[i]->sender_pid()  == (unsigned int)pid) {
      if(dataIn[i]->request_id() == requestId) {
        *pos = i;
        return true;
      }
    }
  }
  return false;
}

// -------------------------------------------------------------------------------------------------
// </Checks>
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
// <Main loop>
// -------------------------------------------------------------------------------------------------

void EventQueue::loop() {
  while(!terminate) {
    if(data()) {
      Event *in = new Event();
      if(receiveEvent(in)) {
        if(in->type() == Event_EventType_ET_DATA) {
          storeData(in);

          /** A relay is a process that does not receive data
            * but only forwards data to other processes. As such
            * it has to check it's event queue regulary if new data
            * events have arrived. A non-relay process on the other hand
            * must not be be notified on data arrival events. It's the
            * users decision when to check for data.
            */

          if(isRelay) {
            pushEvent(in, &eventsIn);
          } else {
            notifyIncoming.notify_all();
          }
        } else {
          pushEvent(in, &eventsIn);
        }
      } else {
        delete in;
      }
    }

    if(outgoing()) {
      Event *out = 0;
      if(popEvent(&out, &eventsOut)) {
        sendEvent(out);
        delete out;
      }
    }
  }
}

// -------------------------------------------------------------------------------------------------
// </Main loop>
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
// <Data push>
// -------------------------------------------------------------------------------------------------

void EventQueue::pushData(Event *ev, pid_t receiver, std::string &receiverName, std::string &senderName) {
  ev->set_sender_pid(getpid());
  ev->set_sender_name(senderName);
  ev->set_receiver_pid(receiver);
  ev->set_receiver_name(receiverName);
  pushEvent(ev);
}

void EventQueue::pushStringData(std::string &val, pid_t receiver, std::string &receiverName, std::string &senderName) {
  Event *ev = newEvent(Event_EventType_ET_DATA, Event_DataType_DT_STRING);
  ev->set_str_data(val);

  pushData(ev, receiver, receiverName, senderName);
}

void EventQueue::pushInt32Data(int32_t val, pid_t receiver, std::string &receiverName, std::string &senderName) {
  Event *ev = newEvent(Event_EventType_ET_DATA, Event_DataType_DT_INT32);
  ev->set_int32_data(val);

  pushData(ev, receiver, receiverName, senderName);
}

void EventQueue::pushUint32Data(uint32_t val, pid_t receiver, std::string &receiverName, std::string &senderName) {
  Event *ev = newEvent(Event_EventType_ET_DATA, Event_DataType_DT_UINT32);
  ev->set_uint32_data(val);

  pushData(ev, receiver, receiverName, senderName);
}

void EventQueue::pushBoolData(bool val, pid_t receiver, std::string &receiverName, std::string &senderName) {
  Event *ev = newEvent(Event_EventType_ET_DATA, Event_DataType_DT_BOOL);
  ev->set_bool_data(val);

  pushData(ev, receiver, receiverName, senderName);
}

void EventQueue::pushBinaryData(const char *val, size_t size, pid_t receiver, std::string &receiverName, std::string &senderName) {
  Event *ev = newEvent(Event_EventType_ET_DATA, Event_DataType_DT_BINARY);
  ev->set_binary_data(val, size);

  pushData(ev, receiver, receiverName, senderName);
}

void EventQueue::pushDoubleData(double val, pid_t receiver, std::string &receiverName, std::string &senderName) {
  Event *ev = newEvent(Event_EventType_ET_DATA, Event_DataType_DT_NUMBER);
  ev->set_double_data(val);

  pushData(ev, receiver, receiverName, senderName);
}

void EventQueue::pushFunctionData(std::string &val, pid_t receiver, std::string &receiverName, std::string &senderName) {
  Event *ev = newEvent(Event_EventType_ET_DATA, Event_DataType_DT_FUNCTION);
  ev->set_str_data(val);

  pushData(ev, receiver, receiverName, senderName);
}

void EventQueue::pushArrayData(Event *message, pid_t receiver, std::string &receiverName, std::string &senderName) {
  message->set_type(Event_EventType_ET_DATA);
  message->set_data_type(Event_DataType_DT_ARRAY);
  pushData(message, receiver, receiverName, senderName);
}

void EventQueue::pushException(std::string &exception, pid_t receiver, std::string &receiverName, std::string &senderName) {
  Event *ev = newEvent(Event_EventType_ET_DATA, Event_DataType_DT_EXCEPTION);
  ev->set_str_data(exception);

  pushData(ev, receiver, receiverName, senderName);
}

// -------------------------------------------------------------------------------------------------
// </Data push>
// -------------------------------------------------------------------------------------------------

/** This should only be used on process termination, when
  * the event loop is already stopped and the kill
  * notification has to be sent.
  */

void EventQueue::sendDirect(Event *ev) {
  Ipc::sendEvent(ev);
}
