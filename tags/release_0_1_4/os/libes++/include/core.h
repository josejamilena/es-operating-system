/*
 * Copyright 2008, 2009 Google Inc.
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

#include <es/base/IMonitor.h>
#include <es/base/IThread.h>

void esInitThread();
int esInit(es::Interface** nameSpace);
es::Thread* esCreateThread(void* (*start)(void* param), void* param);
es::Monitor* esCreateMonitor();

#endif

#endif // NINTENDO_ES_LIBES_CORE_H_INCLUDED