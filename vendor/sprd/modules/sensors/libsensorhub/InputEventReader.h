/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef INPUTEVENTREADER_H_
#define INPUTEVENTREADER_H_

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <linux/input.h>
#include "sensors.h"
/*****************************************************************************/


class InputEventCircularReader {
    pseudo_event* const mBuffer;
    pseudo_event* const mBufferEnd;
    pseudo_event* mHead;
    pseudo_event* mCurr;
    ssize_t mBufferSize;
    ssize_t mTotalSpace;
    ssize_t mFreeSpace;

 public:
    explicit InputEventCircularReader(size_t numEvents);
    InputEventCircularReader(size_t numEvents, size_t eventSize);
    ~InputEventCircularReader();
    ssize_t fill(int fd);
    ssize_t readEvent(pseudo_event const** events);
    void next();
};

/*****************************************************************************/

#endif  // INPUTEVENTREADER_H_
