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

#include "./Include/RequestMapper.h"

/** The request mapper is a simple facility to map responses to their
  * corresponding requests. It stores the information when a process
  * sends a message to another process (request) and calls it a response
  * when a message is sent in the opposite direction.
  *
  * Unanswered requests are stored in a queue for each channel separately
  * and are identified by a numeric value (which is just incremented for
  * each new request on the same channel).
  *
  * The whole thing works, because messages sent to a process always have an
  * order: one message after the other. Messages are stored in a queue.
  * A receiver can't take a message out of order unless he previously called
  * sendWait or sendFuture.
  *
  * TODO: Eventually get rid of all the string concatenating and string compare
  * stuff here. Find a more intelligent approach to identify channels.
  */

RequestMapper::RequestMapper() {
  requests.clear();
}

/** isResponse: Decide if a message is a response or a request.
  * A message is considered to be a response, if a
  * previous message has been sent on the same channel
  * in opposite direction (from the current receiver to
  * the current sender).
  */

bool RequestMapper::isResponse(Event *message) {
  std::string descriptor = getResponseDescriptor(message);
  return requests.find(descriptor) != requests.end()
      ? requests[descriptor].size() > 0
      : false;
}

/** getRequestId: If we have a response message, this method returns the
  * corresponding request (id). If no request can be found
  * then it returns zero.
  */

uint32_t RequestMapper::getRequestId(Event *message) {
  std::string descriptor = getResponseDescriptor(message);
  if(requests.find(descriptor) != requests.end()) {
    uint32_t id = requests[descriptor].front();
    requests[descriptor].pop();
    removeId(id);
    return id;
  } else {
    return 0;
  }
}

/** insertNewRequest: If a message is not a response it must be a new request.
  * This method inserts (or actually appends) a new request
  * to the channel.
  */

uint32_t RequestMapper::insertNewRequest(Event *message) {
  std::string descriptor = getRequestDescriptor(message);
  uint32_t id = generateId();
  requests[descriptor].push(id);
  return id;
}

/** getResponseDescriptor: Channels are identified by the sender and the receiver of a
  * message. To get a response descriptor a string in the form of
  * RECEIVERNAME-SENDERNAME is created.
  *
  * This string is used to look up a previously sent request.
  * (The request must have obviously been sent in the opposite
  * direction).
  */

std::string RequestMapper::getResponseDescriptor(Event *message) {
  std::string descriptor;

  std::string sender_name = message->sender_name();
  std::string receiver_name = message->receiver_name();

  descriptor.append(receiver_name.length() > 0
    ? receiver_name
    : intToString(message->receiver_pid()));

  descriptor.append("-");
  descriptor.append(sender_name.length() > 0
    ? sender_name
    : intToString(message->sender_pid()));

  return descriptor;
}

/** getRequestDescriptor: Create a new request descriptor in the form of SENDERNAME-RECEIVERNAME.
  * This string is then used to identify a request.
  */

std::string RequestMapper::getRequestDescriptor(Event *message) {
  std::string descriptor;

  std::string sender_name = message->sender_name();
  std::string receiver_name = message->receiver_name();

  descriptor.append(sender_name.length() > 0
    ? sender_name
    : intToString(message->sender_pid()));

  descriptor.append("-");
  descriptor.append(receiver_name.length() > 0
    ? receiver_name
    : intToString(message->receiver_pid()));

  return descriptor;
}

std::string RequestMapper::intToString(int number) {
  std::stringstream ss; ss << number;
  return ss.str();
}

uint32_t RequestMapper::generateId() {
  while(1) {
    uint32_t id = std::rand();
    if(std::find(ids.begin(), ids.end(), id) != ids.end()) {
      continue;
    }

    ids.push_back(id);
    return id;
  }
}

void RequestMapper::removeId(uint32_t id) {
  std::vector<uint32_t>::iterator iter = std::find(ids.begin(), ids.end(), id);
  if(iter != ids.end()) {
    ids.erase(iter);
  }
}
