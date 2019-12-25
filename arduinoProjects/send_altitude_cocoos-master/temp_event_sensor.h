//  Instance of Sensor that reads the BME280 temperature sensor connected via SPI port.
//  This is an example of an Event-based Sensor, which signals an event/semaphore when sensor data is available.
//  The other sensors are Polling-based Sensors, which will block the program until the sensor data is available.
//  Event Sensors are better for multitasking.  Caller must use a semaphore to prevent concurrent access to the SPI port.
#ifndef TEMP_EVENT_SENSOR_H_
#define TEMP_EVENT_SENSOR_H_

#include "platform.h"
#include "sensor.h"
#ifdef STM32    //  Event-based Sensor supported only for STM32 Blue Pill
BEGIN_EXTERN_C  //  Allows functions below to be called by C and C++ code.

//  Define the SPI port to be used for the Event-based Sensor: SPI1 or SPI2.
#define BME280_EVENT_SENSOR_PORT SPI2

SensorContext *setup_temp_event_sensor(  //  Set up the sensor parameters.
  uint16_t pollInterval,  //  How often the sensor should be polled, in milliseconds.
  uint8_t taskID          //  Task ID for the Network or Display Task.  Used for sending sensor data for aggregation/transmission or display.
);

END_EXTERN_C  //  End of extern C scope.
#endif  //  STM32
#endif  //  TEMP_EVENT_SENSOR_H_
