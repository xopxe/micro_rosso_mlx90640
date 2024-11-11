
#include <Arduino.h>
#include "micro_rosso.h"

#include <Wire.h>

#define I2C_SCL 22
#define I2C_SDA 21

#include "micro_rosso_mlx90640.h"
CamMLX90640 cam;

#include "ticker.h"
Ticker ticker;

#include "ros_status.h"
RosStatus ros_status;

void setup()
{
  D_println("Booting...");

  Wire.begin(I2C_SDA, I2C_SCL);

  D_print("Setting up transport... ");
#if defined(MICRO_ROS_TRANSPORT_ARDUINO_WIFI)
  // if Wifi.config not called, will use DHCP
  // WiFi.config(MICRO_ROS_TRANSPORT_WIFI_STATIC_IP,MICRO_ROS_TRANSPORT_WIFI_STATIC_GATEWAY, MICRO_ROS_TRANSPORT_WIFI_STATIC_SUBNET);
  set_microros_wifi_transports(ssid, pass, agent_ip, agent_port);
#elif defined(MICRO_ROS_TRANSPORT_ARDUINO_SERIAL)
  Serial.begin(115200);
  set_microros_serial_transports(Serial);
#endif
  D_println("Done.");

  if (!micro_rosso::setup("camdemo_rclc"))
  {
    D_println("FAIL micro_rosso.setup()");
  }

  if (!ticker.setup())
  {
    D_println("FAIL ticker.setup()");
  };

  if (!ros_status.setup())
  {
    D_println("FAIL ros_status.setup()");
  };

  if (!cam.setup(Wire, "/cam/image", "/cam/temperature", ticker.timer_tick))
  //if (!cam.setup_32f1c(Wire, "/cam/image", "/cam/temperature", ticker.timer_tick))
  {
    D_println("FAIL cam.setup()");
  };

  D_println("Boot completed.");
}

void loop()
{
  micro_rosso::loop();
}