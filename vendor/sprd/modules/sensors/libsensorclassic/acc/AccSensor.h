/*
 * Copyright (C) 2011 The Android Open Source Project
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

#ifndef ACC_ACCSENSOR_H_
#define ACC_ACCSENSOR_H_

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include "sensors.h"
#include "SensorBase.h"
#include "InputEventReader.h"
#include "SensorCoordinate.h"

#ifdef ACC_INSTALL_0
#define INSTALL_DIR 0
#elif defined ACC_INSTALL_1
#define INSTALL_DIR 1
#elif defined ACC_INSTALL_2
#define INSTALL_DIR 2
#elif defined ACC_INSTALL_3
#define INSTALL_DIR 3
#elif defined ACC_INSTALL_4
#define INSTALL_DIR 4
#elif defined ACC_INSTALL_5
#define INSTALL_DIR 5
#elif defined ACC_INSTALL_6
#define INSTALL_DIR 6
#elif defined ACC_INSTALL_7
#define INSTALL_DIR 7
#else
#define INSTALL_DIR 0
#endif
/*****************************************************************************/

struct input_event;

class AccSensor : public SensorBase {
 public:
  AccSensor();
  virtual ~AccSensor();

#ifndef ACC_NULL
  enum {Accelerometer = 0, numSensors};
#else
  static const int numSensors = 0;
#endif

  virtual int readEvents(sensors_event_t* data, int count);
  virtual bool hasPendingEvents() const;
  virtual int setDelay(int32_t handle, int64_t ns);
  virtual int setEnable(int32_t handle, int enabled);
  virtual int64_t getDelay(int32_t handle);
  virtual int getEnable(int32_t handle);
  virtual int populateSensorList(struct sensor_t* list);

 private:
  int mEnabled;
  int64_t mDelay;
  int mLayout;
  InputEventCircularReader mInputReader;
  sensors_event_t mPendingEvent;
  bool mHasPendingEvent;
  char input_sysfs_path[PATH_MAX];
  int input_sysfs_path_len;

  int setInitialState();
  int getInputNum(const char *inputName);
  SensorCoordinate mSensorCoordinate;
};

/*****************************************************************************/

#endif  // ACC_ACCSENSOR_H_
