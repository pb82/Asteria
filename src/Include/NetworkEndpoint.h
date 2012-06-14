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

#ifndef NETWORKENDPOINT_H
#define NETWORKENDPOINT_H

#include "EventQueue.h"

/** This is basically just a wrapper around
  * an EventQueue. Anchor is another wrapper,
  * but this one comes without the whole process
  * creation stuff. A NetworkEndpoint is meant to
  * be used to communicate between the nodes of
  * a cluster.
  */

class NetworkEndpoint : public EventQueue
{
  public:
    NetworkEndpoint(int socket);
    ~NetworkEndpoint();

    void sendEvent(Event *ev);
    void nextEvent(Event **ev);
    bool hasEvent();
    void stop();
};

#endif // NETWORKENDPOINT_H
