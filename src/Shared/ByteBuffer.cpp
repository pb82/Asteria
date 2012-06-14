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

#include "../Include/Shared/ByteBuffer.h"

ByteBuffer::~ByteBuffer() { }

// Create an empty buffer
ByteBuffer::ByteBuffer() {
  data.clear();
}

/** Create a buffer from an existing buffer.
  * The existing buffer is copied.
  */

ByteBuffer::ByteBuffer(ByteBuffer *buffer) {
  assert(buffer);
  push(buffer);
}

// Create a buffer with a given capacity.
ByteBuffer::ByteBuffer(unsigned int size) {
  data.resize(size);
  std::fill(data.begin(), data.end(), 0);
}

// Return the std::vector that's used as a storage for the buffer
std::vector<unsigned char> &ByteBuffer::getBackingVector() {
  return data;
}

// Push a single byte at the end of the buffer
void ByteBuffer::push(unsigned char byte) {
  data.push_back(byte);
}

/** Push the contents of a foreign buffer at the end of this buffer.
  * The foreign buffer is copied.
  */

void ByteBuffer::push(ByteBuffer *buffer) {
  assert(buffer);
  data.insert(data.end(), buffer->getBackingVector().begin(), buffer->getBackingVector().end());
}

// Push a chunk of bytes at the end of this buffer.
// A buffer resize is required to do this.
void ByteBuffer::push(unsigned char *byte, int size) {
  assert(byte);
  assert(size > 0);
  int current_size = data.size();
  int index = current_size;
  data.resize(current_size + size);
  memcpy((void *)(&data[index]), byte, size);
}

// Return the last byte in the buffer.
unsigned char ByteBuffer::pop() {
  unsigned char last = data.back();
  data.pop_back();
  return last;
}

void ByteBuffer::reverse() {
  std::reverse(data.begin(), data.end());
}

void ByteBuffer::setByte(int pos, unsigned char val) {
  data[pos] = val;
}

unsigned char ByteBuffer::getByte(int pos) {
  return data[pos];
}

unsigned int ByteBuffer::getLength() {
  return data.size();
}

// Get a raw pointer to the data in this buffer.
unsigned char *ByteBuffer::getData() {
  return &data[0];
}

// Create and return a std::string from the data in this buffer.
std::string ByteBuffer::toAsciiString() {
  char buffer[data.size()];
  memcpy(buffer, &data[0], data.size());
  buffer[data.size()] = '\0';
  return std::string(buffer);
}

int16_t ByteBuffer::readInt16(int at) {
  int16_t i;  
  memcpy(&i, &data[at], sizeof(int16_t));
  return i;
}

int ByteBuffer::write(int index, ByteBuffer *buffer) {
  return write(index, buffer->getData(), buffer->getLength());
}

/** write
  * write a range of bytes at a given position
  * into the array.
  */

int ByteBuffer::write(int index, unsigned char *byte, int size) {
  assert(byte);
  int required_bytes = index + size;
  int current_size = data.size();

  if(current_size < required_bytes) {
    data.resize(required_bytes);
    memcpy((void *)(&data[index]), byte, size);
  } else {
    memcpy((void *)(&data[index]), byte, size);
  }
  return size;
}

// Write the contents of this buffer to a file
int ByteBuffer::writeToFile(FILE *file) {
  int already_written = 0;
  int written_this_time = 0;
  int size = this->getLength();
  void *ptr = (void *)this->getData();

  while(already_written < size) {
    written_this_time = fwrite(ptr, 1, size, file);
    if(written_this_time == 0) break;
    already_written += written_this_time;
  }
  return already_written;
}

// Read from a file and push the contents on this buffer
void ByteBuffer::readFromFile(int size, FILE *file) {
  if(size <= 0)
    return;

  int already_read = 0;
  int read_this_time = 0;
  unsigned char buffer[CHUNK_SIZE];
  while(already_read < size) {
    read_this_time = fread(buffer, 1, CHUNK_SIZE, file);
    if(read_this_time == 0) break;
    this->push(buffer, read_this_time);
    already_read += read_this_time;
  }
}
