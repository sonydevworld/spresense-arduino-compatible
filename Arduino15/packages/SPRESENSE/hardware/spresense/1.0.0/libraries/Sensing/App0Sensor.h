#ifndef __APP0SENSOR__
#define __APP0SENSOR__


#include <sensing/logical_sensor/step_counter.h>


class App0Sensor : public ApplicationSensor
{
public:
  App0Sensor(int                       id,
             unsigned int              subscriptions,
             sensor_data_mh_callback_t callback) :
    ApplicationSensor(id, subscriptions, callback)
  {
    puts("Start sensing...");
    puts("-------------------------------------------------------------------------------------");
    puts("      tempo,     stride,      speed,   distance,    t-stamp,       step,  move-type");
  }

};

#endif /* __APP0SENSOR__ */
