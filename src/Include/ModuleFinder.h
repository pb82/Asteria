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

#ifndef MODULEFINDER_H
#define MODULEFINDER_H

#include <unistd.h>
#include <dirent.h>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <cstdio>
#include <string>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>

enum as_module_type {
  As_T_JavaScript,
  As_T_SharedLib,
  As_T_Invalid
};

struct as_module_t {
  std::string path;
  std::string key;
  as_module_type type;
};

class ModuleFinder
{
public:
  ModuleFinder();

  as_module_t lookup(std::string &key);
private:
  as_module_t getModuleFromPath(std::string &path);
  as_module_t probeModuleAtPath(std::string &path);
  as_module_type getModuleType(std::string &path);
  bool getModuleAtLocation(std::string &path, as_module_t *module);

  std::string getRealPath(const char *path);
  std::string getPathComponent(std::string &path);
  std::string getFileComponent(std::string &path);
  std::string insertPrefix(std::string &path);

  bool fileExists(std::string &path);
};

#endif // MODULEFINDER_H
