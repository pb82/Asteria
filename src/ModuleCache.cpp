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

#include "Include/ModuleCache.h"

/** A simple Name -> Object mapping for modules that were loaded
  * by 'require'. The object is stored and if a module is required
  * a second time, then the already stored object is returned.
  */

ModuleCache::ModuleCache() { }

bool ModuleCache::moduleInCache(std::string &module) {  
  return dict.size() == 0 ? false : dict.find(module) != dict.end();
}

v8::Handle<v8::Object> ModuleCache::getModule(std::string &module) {
  std::map<std::string, v8::Persistent<v8::Object> >::iterator iter = dict.find(module);
  if(iter == dict.end()) {

    // We can't return undefined here, because it does not inherit
    // from v8::Object.
    return v8::Object::New();
  } else {
    return iter->second;
  }
}

void ModuleCache::putModule(std::string name, v8::Handle<v8::Object> module) {
  dict[name] = v8::Persistent<v8::Object>::New(module);
}
