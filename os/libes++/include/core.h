/*
 * Copyright 2008 Google Inc.
 * Copyright 2006, 2007 Nintendo Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NINTENDO_ES_LIBES_CORE_H_INCLUDED
#define NINTENDO_ES_LIBES_CORE_H_INCLUDED

#include <es.h>

#ifdef __cplusplus

#include <es/base/IThread.h>

void esInitThread();
int esInit(es::IInterface** nameSpace);
es::IThread* esCreateThread(void* (*start)(void* param), void* param);
es::IMonitor* esCreateMonitor();

extern "C" {
#endif

size_t strnlen(const char *s, const size_t n);
int stricmp(const char *s1, const char *s2);
int strnicmp(const char *s1, const char *s2, size_t n);

#ifdef __cplusplus
}
#endif

#endif // NINTENDO_ES_LIBES_CORE_H_INCLUDED
