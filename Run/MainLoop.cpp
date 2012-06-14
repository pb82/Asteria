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

#include "./Include/MainLoop.h"

MainLoop::MainLoop(int argc, char *argv[]) : master(0) {
  assert(argv != 0);
  assert(argc >= 2);

  std::string source;
  int socket = 0, pid = 0;

  if(JsUtils::loadSource(argv[1], &source, true)) {
    if(source.length() == 0) {
      return;
    }

    master = createProcess(&socket, &pid, argc, argv, "master");
    if(master) {
      // Send the source code to run and the initial
      // event to start to the master process.
      Event *ev = new Event;
      ev->set_type(Event_EventType_ET_RUN_AS_MASTER);
      ev->set_str_data(source);
      master->sendEvent(ev);

      // Keep track of processes by remembering their
      // pid and name.
      processByPid[master->getPid()] = master;
      processByName[std::string("master")] = master;

      // Enter main loop
      loop();
    } else {
      log_error("Error: could not create master process.");
    }
  } else {
    log_error("Error: File not found: %s", argv[1]);
  }
}

MainLoop::~MainLoop() {
  // Cleanup:
  // Kill all remaining processes.
  std::map<pid_t, Anchor*>::iterator iterator = processByPid.begin();
  for(; iterator != processByPid.end(); iterator++) {
    Anchor *proc = static_cast<Anchor *>(iterator->second);
    if(proc) {
      kill(proc->getPid(), SIGABRT);
    }
  }
}

Anchor *MainLoop::createProcess(int *socket, int *pid, int argc, char *argv[], const char *name) {
  if(Anchor::launch(socket, pid, argc, argv, name)) {
    return new Anchor(*socket, *pid);
  } else {
    return 0;
  }
}

/**
  * Runtime main loop:
  * This method implements the runtime that receives all events,
  * sent by it's child processes (in that case the master process
  * is considered a child of the runtime as well) and handles them.
  */

void MainLoop::loop() {
  assert(this->master != 0);
  assert(this->processByPid.size() == 1);
  assert(this->processByName.size() == 1);

  while(!runtimeState.getTerminate()) {
    // Terminate flag not set but no processes left
    // to execute: a runtime error must have occured.
    if(processByPid.size() == 0) {
      log_error("Error: Runtime crashed during execution. Exit.");
      exit(1);
    }

    // Iterate over all of our processes
    std::map<pid_t, Anchor*>::iterator iterator = processByPid.begin();
    for(; iterator != processByPid.end(); iterator++) {
      if(runtimeState.getRefreshProcesses()) {
        // This flag indicates that the iterator object
        // is invalid at this point. This happens when
        // processes terminate or are created.
        runtimeState.setRefreshProcesses(false);
        break;
      }

      Anchor *process = static_cast<Anchor *>(iterator->second);
      assert(process);

      if(process->hasEvent()) {
        Event *message;
        process->nextEvent(&message);

        switch(message->type()) {
          case Event_EventType_ET_FINISHED: handleFinished(process, message);           break;
          case Event_EventType_ET_FORK_REQUEST: handleForkRequest(process, message);    break;
          case Event_EventType_ET_ALIVE_REQUEST: handleAliveRequest(process, message);  break;
          case Event_EventType_ET_KILLED: handleKilled(process, message);               break;
          case Event_EventType_ET_KILL_REQUEST: handleKillRequest(message);             break;
          case Event_EventType_ET_DATA: handleData(process, message);                   break;
          default:
            log_debug("Warning: Unknown message type received: %d", message->type());
            break;
        }
        delete message;
      } else {
      #ifdef LATENCY
        usleep(LATENCY)
      #else
        usleep(1000);
      #endif
      }
    }
  }
}

/**
  * Event_EventType_ET_FINISHED:
  * This message is sent by a process who finished execution or
  * ran into an unhandled Javascript exception.
  */

void MainLoop::handleFinished(Anchor *process, Event *message) {
  assert(process);
  assert(message);

  // Compare process pointers to find out wether
  // the master or some subprocess is affected.
  if(process == this->master) {
    master->endProcess();
    runtimeState.setTerminate(true);
  } else {
    process->endProcess();

    processByPid.erase(process->getPid());
    deleteProcessByName(message->sender_name());

    runtimeState.setRefreshProcesses(true);
    delete process;
  }
}

/**
  * Event_EventType_ET_FORK_REQUEST:
  * This event is sent to the runtime by a process that want's
  * to spawn a child process. It contains the name of the child and
  * the function it should execute.
  */

void MainLoop::handleForkRequest(Anchor *parent, Event *message) {
  assert(parent);
  assert(message);

  int socket, pid;

  std::string parentName = message->sender_name();
  std::string childName = message->str_array(0);
  std::string function = message->str_array(1);

  // Check if a process with the same name already exists
  if(processByName.find(childName) != processByName.end()) {
    parent->sendAck(false);
  } else {
    Anchor *child = createProcess(&socket, &pid, 0, 0, childName.c_str());
    if(child) {
      assert(socket);
      assert(pid);

      processByPid[child->getPid()] = child;

      // Remeber the code that this child executes.
      child->setFunction(function);

      // If a name for the new process was specified, register
      // the process with the name.
      if(childName.length() > 0) processByName[childName] = child;

      // Report success.
      parent->sendAck(true, pid);

      // Tell the new process to execute the given function.
      child->sendRunAsSlave(parentName, parent->getPid(), function);
    } else {
      // Fail.
      // TODO: Report a more specific error message.
      parent->sendAck(false);
    }
  }
}

/**
  * Event_EventType_ET_ALIVE_REQUEST:
  * This message is sent by a process that want's to query the
  * status (alive, not alive) of another process.
  */

void MainLoop::handleAliveRequest(Anchor *process, Event *message) {
  if(message->str_data().length() > 0)   {
    // Query process by name
    process->sendAck(processByName.find(message->str_data()) != processByName.end(), 0);
  } else {
    // Query process by pid
    process->sendAck(processByPid.find(message->uint32_data())  != processByPid.end(), 0);
  }
}

/**
  * Event_EventType_ET_KILLED:
  * This message is sent by a process that received one of the following signals:
  * SIGABRT
  * SIGTERM
  * SIGINT
  * SIGSEGV
  */

void MainLoop::handleKilled(Anchor *process, Event *message) {
  assert(process);
  assert(message);

  processByPid.erase(process->getPid());
  deleteProcessByName(message->sender_name());
  runtimeState.setRefreshProcesses(true);
}

/**
  * Event_EventType_ET_KILL_REQUEST:
  * This message is sent by a process that wants to send
  * the kill signal (and thereby kill) another process.
  */

void MainLoop::handleKillRequest(Event *message) {
  assert(message);

  if(message->str_data().length() > 0) {
    // Kill by name.
    if(processByName.find(message->str_data()) != processByName.end()) {
      pid_t pid = processByName[message->str_data()]->getPid();
      kill(pid, SIGABRT);
      waitpid(pid, NULL, 0);
    }
  } else {
    // Kill by pid.
    kill(message->uint32_data(), SIGABRT);
    waitpid(message->uint32_data(), NULL, 0);
  }
}

/**
  * Event_EventType_ET_DATA:
  * This message is sent by a process that wants to send data to
  * another process.
  */

void MainLoop::handleData(Anchor *process, Event *message) {
  Anchor *receiver = 0;
  if(message->receiver_name().length() > 0) {
    // Get the receiver by name.
    if(processByName.find(message->receiver_name()) != processByName.end()) {
      receiver = processByName[message->receiver_name()];
    }
  } else if (message->has_receiver_pid()) {
    // Get the receiver by pid.
    receiver = processByPid[message->receiver_pid()];
  }

  // Ok, a receiver for the data packet could be identified.
  if(receiver) {
    Event *m = new Event(*message);

    // Check if this is just some data packet or maybe:
    // - A new request to the process.
    // - A Response to a previous request.
    // This is important for the 'sendForProxy' function.
    uint32_t request_id = requestMapper.isResponse(m)
        ? requestMapper.getRequestId(m)
        : requestMapper.insertNewRequest(m);

    m->set_request_id(request_id);
    receiver->sendEvent(m);
    process->sendAck(true, request_id);
  } else {
    process->sendAck(false);
  }
}

void MainLoop::deleteProcessByName(const std::string &processName) {
  if(processName.size() == 0)
    return;

  if(processByName.find(processName) != processByName.end()) {
    processByName.erase(processName);
  }
}
