#ifndef __imu_mlx90640_h
#define __imu_mlx90640_h

#include <Wire.h>
#include <Adafruit_MLX90640.h>

//if MLX90640_HZ not defined, will auto-probe
//#define MLX90640_HZ MLX90640_4_HZ  

#if !defined(MLX90640_MODE)
#define MLX90640_MODE MLX90640_CHESS
#endif

#if !defined(MLX90640_ADC)
#define MLX90640_ADC MLX90640_ADC_16BIT
#endif

class CamMLX90640
{
public:
  CamMLX90640();
  static bool setup(TwoWire &wire = Wire,
                    const char *topic_image = "/cam/image",
                    const char *topic_temp = "/cam/temperature",
                    timer_descriptor &timer = micro_rosso::timer_report);
};

#endif // __imu_mlx90640_h
