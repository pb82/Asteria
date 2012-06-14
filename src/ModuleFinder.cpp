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

#include "Include/ModuleFinder.h"

/** The ModuleFinder is part of the 'require' implementation.
  * It finds modules by a given key and returns path and type
  * information. For example, if you execute 'require("fs")'
  * the ModuleFinder will return a struct containing the following
  * information:
  *
  * path: /opt/asteria/modules/libfs.so
  * type: As_T_SharedLib
  * key:  fs
  */

ModuleFinder::ModuleFinder() { }

/** lookup: if the key string starts with ./ or ../ then
  * the module is looked up at the specified path.
  * Otherwise the module default path is used.
  */

as_module_t ModuleFinder::lookup(std::string &key) {
  if(key.substr(0, 1).compare(".") == 0) {
    // Load the module from a given path.
    std::string real_path = getRealPath(key.c_str());
    as_module_t module = getModuleFromPath(real_path);
    if(module.type == As_T_Invalid) {
      // Maybe the user omitted the .js or .so ending
      std::string jsPath = key.append(".js");
      module = getModuleFromPath(jsPath);

      if(module.type == As_T_Invalid) {
        std::string soPath = key.append(".so");
        module = getModuleFromPath(soPath);
      }
    }

    module.key = key;
    return module;
  } else {
    // Load the module from the default module directory.
    std::string env_path = std::string(getenv("JS_MODULE_SEARCH_PATH"));
    env_path.append("/");
    env_path.append(key);

    as_module_t module = getModuleFromPath(env_path);
    module.key = key;
    return module;
  }
}

as_module_t ModuleFinder::getModuleFromPath(std::string &path) {
  as_module_t module;
  module.type = As_T_Invalid;

  if(path.length() == 0) return module;

  switch (getModuleType(path)) {
    case As_T_JavaScript:
      if(getModuleAtLocation(path, &module)) module.type = As_T_JavaScript;
      return module;

    case As_T_SharedLib:
      if(getModuleAtLocation(path, &module)) module.type = As_T_SharedLib;
      return module;

    case As_T_Invalid:
      return probeModuleAtPath(path);

    default:
      return module;
  }
}

// Try to fetch a file at a given path.
bool ModuleFinder::getModuleAtLocation(std::string &path, as_module_t *module) {
  if(fileExists(path)) {
    module->path = path;
    return true;
  }

  /** The given filename is invalid. We'll try a prefixed
    * version of the filename now. This is unlikely for
    * JavaScript modules (libxxx.js) but quite likely for
    * shared libraries (libxxx.so).
    */

  std::string prefixed_path = insertPrefix(path);
  if(fileExists(prefixed_path)) {
    module->path = prefixed_path;
    return true;
  }

  module->path = "";
  return false;
}

/** probeModuleAtPath: The key lacks a '.js' or '.so' extension so we'll try both.
  * This is the most common case as most people will require their modules
  * by 'fs' instead of 'fs.so'.
  *
  * TODO: This solution does not work for so files with names like
  * libfs.so.1.0.0. Find a way to support such files also.
  */

as_module_t ModuleFinder::probeModuleAtPath(std::string &path) {
  as_module_t module;

  std::string so_path = path;
  std::string js_path = path;

  // Probing for JavaScript.
  js_path.append(".js");
  if(getModuleAtLocation(js_path, &module)) {
    module.type = As_T_JavaScript;
    return module;
  }

  // Probing for shared lib.
  so_path.append(".so");
  if(getModuleAtLocation(so_path, &module)) {
    module.type = As_T_SharedLib;
    return module;
  }

  // Fail.
  module.type = As_T_Invalid;
  module.path = "";
  return module;
}

// Get the '/opt/asteria/modules/' part or
// '/opt/asteria/modules/libfs.so'
std::string ModuleFinder::getPathComponent(std::string &path) {
  size_t found = path.find_last_of("/\\");
  return path.substr(0,found);
}

// Get the 'libfs.so' part or
// '/opt/asteria/modules/libfs.so'
std::string ModuleFinder::getFileComponent(std::string &path) {
  size_t found = path.find_last_of("/\\");
  return path.substr(found+1);
}

as_module_type ModuleFinder::getModuleType(std::string &path) {
  size_t pos_so = path.find(".so");
  size_t pos_js = path.find(".js");

  pos_so = pos_so > path.length() ? 0 : pos_so;
  pos_js = pos_js > path.length() ? 0 : pos_js;
  bool invalid = (pos_so == 0)&&(pos_js == 0);

  return invalid
      ? As_T_Invalid
      : pos_js > pos_so
        ? As_T_JavaScript
        : As_T_SharedLib;
}

// Resolve shortcuts like ., .. or ~
std::string ModuleFinder::getRealPath(const char *path) {
  char p[PATH_MAX];
  return realpath(path, p)
      ? std::string(p)
      : "";
}

// Turn a './fs.so' into a './libfs.so'.
// Important: don't change path itself.
std::string ModuleFinder::insertPrefix(std::string &path) {
  std::string path_c = getPathComponent(path);
  std::string file_c = getFileComponent(path);
  std::string lib_str("lib");

  lib_str.append(file_c);
  path_c.append("/");
  path_c.append(lib_str);
  return path_c;
}

// TODO: find a more portable solution.
bool ModuleFinder::fileExists(std::string &path) {
  return access(path.c_str(), F_OK) == 0;
}
