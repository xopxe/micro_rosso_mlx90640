# micro rosso mlx90640

This a module for the [micro_rosso](https://github.com/xopxe/micro_rosso_platformio) system.

It provides support for the MLX90640 thermal camera by publishing ROS2 topics.

## Loading and starting

First, import the module into your project's `platformio.ini`:

```ini
lib_deps =
    ...
        "xopxe/micro_rosso_mlx90640": "^0.1.0"
```

Then, in your `main.cpp`:

```cpp
...
#include "micro_rosso_mlx90640.h"
CamMLX90640 cam;
...
void setup() {
  Wire.begin(I2C_SDA, I2C_SCL); // initialize I2C as needed
  ...
  cam.setup(Wire);
  ...
}
```

The setup method allows passing optional topic names and a different micro_rosso timer to change the publication rate (by default, it uses the 5Hz timer). It is declared as follows:

```h
  static bool setup(TwoWire &wire = Wire,
                    const char *topic_image = "/cam/image",
                    const char *topic_temp = "/cam/temperature",
                    timer_descriptor &timer = micro_rosso::timer_report);
```

In particular it may be usefull to pass a slower timer, as the rate of acquisition of the module is slower than the default timer_report.

## Using the module

The module emits the following topics:

* image: [sensor_msgs/msg/image]([https://docs.ros2.org/latest/api/sensor_msgs/msg/Image.html)). The image uses mono16 encoding, where each pixel is a 16 bit number indicating a temperature in hundreths of degrees Celsius (so 2950 means 29.5ºC).
* temperature: [sensor_msgs/msg/temperature](https://docs.ros2.org/foxy/api/sensor_msgs/msg/Temperature.html). The temperature as detected by the module and used for image calibration.

## Authors and acknowledgment

jvisca@fing.edu.uy - [Grupo MINA](https://www.fing.edu.uy/inco/grupos/mina/), Facultad de Ingeniería - Udelar, 2024

## License

MIT
