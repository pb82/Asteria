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

#ifndef BYTEBUFFER_H
#define BYTEBUFFER_H

#include <string.h>
#include <string>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cstdio>
#include <assert.h>

#define CHUNK_SIZE 32768

// Native implementation of a JavaScript 'byteArray'.
// This one is implemented on top of std::vector
class ByteBuffer
{
public:
  ByteBuffer();
  ByteBuffer(unsigned int size);
  ByteBuffer(ByteBuffer *buffer);
  virtual ~ByteBuffer();

  unsigned char pop();
  void push(unsigned char byte);
  void push(ByteBuffer *buffer);
  void push(unsigned char *byte, int size);
  void setByte(int pos, unsigned char val);
  int write(int index, unsigned char *byte, int size);
  int write(int index, ByteBuffer *buffer);
  int writeToFile(FILE* file);
  void readFromFile(int size, FILE* file);

  unsigned char getByte(int pos);
  unsigned int getLength();
  unsigned char *getData();
  void reverse();

  std::string toAsciiString();

  int16_t readInt16(int at);

  std::vector<unsigned char> &getBackingVector();


private:
  std::vector<unsigned char> data;
};

#endif // BYTEBUFFER_H
