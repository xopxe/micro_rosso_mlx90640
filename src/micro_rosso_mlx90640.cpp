// FIXME WHAY IS THIS DIFFERENT TO
// https://github.com/micro-ROS/micro-ROS-demos/blob/jazzy/rclc/static_type_handling/main.c

#include "micro_rosso.h"

#include "micro_rosso_mlx90640.h"

#include <micro_ros_utilities/string_utilities.h>
#include <sensor_msgs/msg/image.h>
#include <sensor_msgs/msg/temperature.h>

static const uint32_t HEIGHT = 24;
static const uint32_t WIDTH = 32;

static sensor_msgs__msg__Image msg_image;
static sensor_msgs__msg__Temperature msg_temperature;

static publisher_descriptor pdescriptor_image;
static publisher_descriptor pdescriptor_temperature;

static float frame[WIDTH * HEIGHT]; // buffer for full frame of temperatures

//static uint16_t image_buff[WIDTH * HEIGHT];
static uint16_t *image_buff = (uint16_t *)frame; // save memory by overlaying buffer

Adafruit_MLX90640 mlx;

#define RCCHECK(fn)              \
  {                              \
    rcl_ret_t temp_rc = fn;      \
    if ((temp_rc != RCL_RET_OK)) \
    {                            \
      return false;              \
    }                            \
  }
#define RCNOCHECK(fn)       \
  {                         \
    rcl_ret_t temp_rc = fn; \
    (void)temp_rc;          \
  }

CamMLX90640::CamMLX90640()
{

  msg_image.header.frame_id = micro_ros_string_utilities_set(msg_image.header.frame_id, "cam_link");

  msg_image.height = HEIGHT;
  msg_image.width = WIDTH;
  msg_image.step = WIDTH * sizeof(uint16_t);

  msg_image.data.capacity =  HEIGHT * WIDTH * sizeof(uint16_t); //sizeof(image_buff);
  msg_image.data.size = msg_image.data.capacity;
  msg_image.data.data = (uint8_t *)image_buff;
  // msg_image.encoding = micro_ros_string_utilities_set(msg_image.encoding, "32F1C");
  msg_image.encoding = micro_ros_string_utilities_set(msg_image.encoding, "mono16");

  msg_temperature.temperature = -273.15;
};

static uint16_t swap_uint16(uint16_t val)
{
  return (val << 8) | (val >> 8);
}

static void report_cb(int64_t last_call_time)
{
  if (pdescriptor_image.topic_name != NULL)
  {
    if (mlx.getFrame(frame) != 0)
    {
      D_println("mlx.getFrame Failed");
      return;
    }
    for (int i = 0; i < WIDTH * HEIGHT; i++)
    {
      float t = frame[i];
      image_buff[i] = (uint16_t)(t * 100);
    }

    micro_rosso::set_timestamp(msg_image.header.stamp);
    RCNOCHECK(rcl_publish(
        &pdescriptor_image.publisher,
        &msg_image,
        NULL));
  }
  if (pdescriptor_temperature.topic_name != NULL)
  {
    msg_temperature.temperature = mlx.getTa(false);
    micro_rosso::set_timestamp(msg_temperature.header.stamp);
    RCNOCHECK(rcl_publish(
        &pdescriptor_temperature.publisher,
        &msg_temperature,
        NULL));
  }
}

bool CamMLX90640::setup(TwoWire &wire,
                        const char *topic_image,
                        const char *topic_temp,
                        timer_descriptor &timer_report)
{
  D_print("setup imu_mlx90640... ");
  if (!mlx.begin(MLX90640_I2CADDR_DEFAULT, &wire))
  {
    return false;
  }
  D_println("Found MLX90640");

  D_print(" Serial number: ");
  D_print(mlx.serialNumber[0], HEX);
  D_print(mlx.serialNumber[1], HEX);
  D_println(mlx.serialNumber[2], HEX);

  mlx.setMode(MLX90640_MODE);
  D_print(" Current mode: ");
  if (mlx.getMode() == MLX90640_CHESS)
  {
    D_println("Chess");
  }
  else
  {
    D_println("Interleave");
  }

  mlx.setResolution(MLX90640_ADC);
  D_print(" Current resolution: ");
  mlx90640_resolution_t res = mlx.getResolution();
  switch (res)
  {
  case MLX90640_ADC_16BIT:
    D_println("16 bit");
    break;
  case MLX90640_ADC_17BIT:
    D_println("17 bit");
    break;
  case MLX90640_ADC_18BIT:
    D_println("18 bit");
    break;
  case MLX90640_ADC_19BIT:
    D_println("19 bit");
    break;
  }

//
#if defined(MLX90640_HZ)
  mlx.setRefreshRate(MLX90640_HZ);
#else
  // Auto test rates
  D_print(" Trying rates: ");
  for (int r = MLX90640_64_HZ; r >= MLX90640_0_5_HZ; r--)
  {
    D_print(".");
    mlx90640_refreshrate_t rate = static_cast<mlx90640_refreshrate_t>(r);
    mlx.setRefreshRate(rate);
    if (mlx.getFrame(frame) == 0)
      break;
  }
#endif

  D_print(" Current frame rate: ");
  mlx90640_refreshrate_t rate = mlx.getRefreshRate();
  switch (rate)
  {
  case MLX90640_0_5_HZ:
    D_println("0.5 Hz");
    break;
  case MLX90640_1_HZ:
    D_println("1 Hz");
    break;
  case MLX90640_2_HZ:
    D_println("2 Hz");
    break;
  case MLX90640_4_HZ:
    D_println("4 Hz");
    break;
  case MLX90640_8_HZ:
    D_println("8 Hz");
    break;
  case MLX90640_16_HZ:
    D_println("16 Hz");
    break;
  case MLX90640_32_HZ:
    D_println("32 Hz");
    break;
  case MLX90640_64_HZ:
    D_println("64 Hz");
    break;
  }

  if (topic_image != NULL)
  {
    pdescriptor_image.qos = QOS_DEFAULT;
    pdescriptor_image.type_support =
        (rosidl_message_type_support_t *)ROSIDL_GET_MSG_TYPE_SUPPORT(
            sensor_msgs, msg, Image);
    pdescriptor_image.topic_name = topic_image;
    micro_rosso::publishers.push_back(&pdescriptor_image);
  }

  if (topic_temp != NULL)
  {
    pdescriptor_temperature.qos = QOS_BEST_EFFORT;
    pdescriptor_temperature.type_support = (rosidl_message_type_support_t *)
        ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Temperature);
    pdescriptor_temperature.topic_name = topic_temp;
    micro_rosso::publishers.push_back(&pdescriptor_temperature);
  }
  timer_report.callbacks.push_back(&report_cb);

  D_println("done.");
  return true;
}
