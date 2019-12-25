//  Sample application demonstrating multitasking of multiple IoT sensors and network transmission on Arduino Uno and STM32 Blue Pill with cocoOS.
//  Based on https://github.com/lupyuen/cocoOSExample-arduino
//  Note: Never use the "new" operator like:
//    serialPort = new SoftwareSerial(rx, tx);
//  On STM32 this pulls in the C++ Demangler (cp-demangle.o) which adds 27 KB of useless code.
//  https://docs.google.com/spreadsheets/d/1g_6hTMSofzywcjiIgl0YKy3Qodn9TXdNRaKW-Qyn5s0/edit#gid=517105377
#include "platform.h"
#include <stdlib.h>
#include <string.h>
#include <cocoos.h>
//  These are the functions that we will implement in this file.
static void system_setup(void);                       //  Initialise the system.
static void sensor_setup(uint8_t display_task_id);    //  Start the sensor tasks for each sensor to read and process sensor data.

int main(void) {
  //  The application starts here. We create the tasks to read and send sensor data 
  //  and start the task scheduler. Note: Arduino's setup() and loop() will not be called since main() is defined.

  //enable_debug();       //  Uncomment to allow display of debug messages in development devices. NOTE: This will hang if no debugger is attached.
  //  disable_debug();  //  Uncomment to disable display of debug messages.  For use in production devices.

  //  Init the platform, cocoOS and create any system objects.
  platform_setup();  //  Arduino or STM32 platform setup.
  os_init();         //  Init cocoOS before creating any multitasking objects.
  system_setup();    //  Setup the system.

  //  Erase the aggregated sensor data.
  //setup_aggregate();    //  We will aggregate the sensor data in Network Task before transmitting to network.
  uint8_t task_id = 0;  //  Task ID (for Network Task or Display Task) that will receive sensor data messages.

 
  //  Start the Sensor Task for each sensor to read sensor data and send to the Network Task or Display Task.
  sensor_setup(task_id);

  //  Start the Arduino or STM32 timer to generate ticks for cocoOS to switch tasks.
  platform_start_timer(os_tick);

  //  Start cocoOS task scheduler, which runs the Sensor Tasks, Network Task and UART Task.
  os_start();  //  Never returns.  
  return EXIT_SUCCESS;
}

static void sensor_setup(uint8_t task_id) {
  //  Start the sensor tasks for each sensor to read and process sensor data.
  //  Sensor data will be sent to the message queue at the given task ID,
  //  which is the Network Task or Display Task.  Edit this function to add your own sensors.
  const int pollInterval = 10000;  //  Poll the sensor every 10 seconds.

  //  Set up the sensors and get their sensor contexts.  For each sensor, we will create sensor tasks 
  //  using the same task function sensor_task(), but with unique sensor context.
  //  All task priorities must be unique, from 1 (highest priority) to 254 (lowest).
  //  task_create(..., 0, 0, 0) means that the task will not receive any message queue data.  //  debug(F("task_create"));

  //  Use the new Event-based Sensor that's better for multitasking.
  SensorContext *tempEventContext = setup_temp_event_sensor(pollInterval, task_id);
  task_create(sensor_task, tempEventContext, 100, 0, 0, 0);  //  Priority 100 = lower priority than network task    
}

//  Disable exceptions for abstract classes. From https://arobenko.gitbooks.io/bare_metal_cpp/content/compiler_output/abstract_classes.html
extern "C" void __cxa_pure_virtual() { while (true) {} }
void operator delete(void *) { }

//  Arduino-specific functions are in send_altitude_cocoos.ino.
//  STM32-specific functions are in stm32/bluepill library.
static void system_setup(void) {
}
