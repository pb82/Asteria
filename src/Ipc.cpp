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

#include "Include/Ipc.h"

Ipc::Ipc(int socket) : socket(socket)  { }
Ipc::~Ipc() { }

bool Ipc::receiveContent(void *buffer, unsigned int len) {
assert(buffer != 0);
assert(len > 0);

  int received = 0;
  int bytes_left = len;
  char *position = (char *)buffer;

  while(bytes_left > 0) {
    received = read(socket, position, bytes_left);
    if(received <= 0) {
      return false;
    }

    bytes_left -= received;
    position += received;
  }

  return true;
}

bool Ipc::sendContent(void *data, unsigned int len) {
assert(data != 0);
assert(len > 0);

  int written = 0;
  int bytes_left = len;
  char *position = (char *)data;

  while(bytes_left > 0) {
    written = write(socket, position, bytes_left);
    if(written <= 0) {
      return false;
    }

    bytes_left -= written;
    position += written;
  }

  return true;
}

bool Ipc::sendString(std::string &str) {
  uint32_t len = str.length();
  if(write(socket, &len, sizeof(uint32_t)) != sizeof(uint32_t)) {
    return false;
  }

  return sendContent((void *)str.data(), len);
}

bool Ipc::receiveEvent(Event *ev) {
  uint32_t size;
  if(!checkSign()) {
    return false;
  }

  if(read(socket, &size, sizeof(uint32_t)) >= 0) {
    char buffer[size];
    receiveContent(&buffer, size);
    return ev->ParseFromArray(buffer, size);
  } else {
    return false;
  }
}

bool Ipc::sendEvent(Event *ev) {
  std::string container;
  if(ev->SerializeToString(&container)) {
    sendSign();
    return sendString(container);
  } else {
    return false;
  }
}

void Ipc::sendSign() {
  uint32_t sign = SIGN;
  write(socket, &sign, sizeof(uint32_t));
}

bool Ipc::checkSign() {
  uint32_t rcv = 0;
  if(read(socket, &rcv, sizeof(uint32_t)) <= 0) {
    /** Ugly hack to prevent the event loop
      * from running hot on unexpected
      * communication loss.
      */
    usleep(1000);    
  }

  return rcv == SIGN;
}
