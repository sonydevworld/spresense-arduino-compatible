#include <BMI160Gen.h>
#include <ApplicationSensor.h>
#include <AccelSensor.h>


const int i2c_addr     = 0x68;
const int baudrate     = 115200;
/**/
const int accel_range  = 2;  /* 2G */
const int accel_rate   = 50; /* 50 Hz */
const int accel_sample = 50; /* 50 sample */

const int step_counter_rate   = 32; /* 32 Hz */
const int step_counter_sample = 32; /* 32sample/1process */

const int read_duration = 1000 / accel_rate;


AccelSensor::AccelSensor(int                       id,
                         unsigned int              subscriptions,
                         sensor_data_mh_callback_t callback) :
  ApplicationSensor(id, subscriptions, callback)
{
  /* Initialize Serial communication. */

  Serial.begin(baudrate);

  /* Wait for the serial port to open. */

  while (!Serial);

  /* Initialize device. */

  BMI160.begin(BMI160GenClass::I2C_MODE, i2c_addr);

  /* Set device setting */

  BMI160.setAccelerometerRange(accel_range);
  BMI160.setAccelerometerRate(accel_rate);
}


int AccelSensor::write_data(void *param)
{
  uint32_t timestamp = (uint32_t)param;
 
  /* Static variable for sensing process */

  static int cnt = 0;
  static struct accel_float_t
  {
    float x;  /* X axis standard gravity acceleration.[G] */
    float y;  /* Y axis standard gravity acceleration.[G] */
    float z;  /* Z axis standard gravity acceleration.[G] */
  }
  data[step_counter_rate];

  static float x[accel_sample];
  static float y[accel_sample];
  static float z[accel_sample];
  static unsigned long previous_time = timestamp;
  /* Convert table 50Hz to 32Hz */
  const char freq_convert_table[] =
  {
    0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16, 18, 19, 21, 23, 25,
    26, 28, 29, 31, 32, 34, 35, 37, 38, 40, 41, 43, 44, 46, 48
  };
  static unsigned long adjust_time = 0;

  /* Check reading cycle */

  unsigned long now = timestamp;
  if ((now - previous_time) <= (read_duration - adjust_time))
    {
      return 0;
    }

  adjust_time = now - previous_time - read_duration;
  if (adjust_time > read_duration)
    {
      adjust_time = 0;
    }

  /* Read raw accelerometer measurements from BMI160 */

  BMI160.readAccelerometerScaled(x[cnt], y[cnt], z[cnt]);

  cnt += 1;

  /* Check if the sample for one process has accumulated */

  if (cnt == accel_sample)
    {
      cnt = 0;
      struct accel_float_t *p_data = data;

      /* Convert frequency */

      for(int i = 0; i < step_counter_sample; i++, p_data++)
        {
          p_data->x = x[freq_convert_table[i]];
          p_data->y = y[freq_convert_table[i]];
          p_data->z = z[freq_convert_table[i]];
        }

      /* Write Accelarometer data to SensorClass */

      write_data_mh(data,
                    sizeof(struct accel_float_t),
                    step_counter_rate,
                    step_counter_sample,
                    now);
    }

  /* Update previous time */

  previous_time = now;

  return 0;
}
