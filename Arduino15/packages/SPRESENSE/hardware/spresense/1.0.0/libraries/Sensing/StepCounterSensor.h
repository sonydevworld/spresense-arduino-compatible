#ifndef __STEPCOUNTERSENSOR__
#define __STEPCOUNTERSENSOR__

#include <sensing/logical_sensor/step_counter.h>


class StepCounterSensor : public ApplicationSensor, StepCounterClass
{
public:
  StepCounterSensor(int                       id,
                    unsigned int              subscriptions = 0,
                    sensor_data_mh_callback_t callback      = NULL) :
    ApplicationSensor(id, subscriptions, callback),
    StepCounterClass(SENSOR_DSP_CMD_BUF_POOL)
  {
    open();

    StepCounterSetting setting;

    setting.walking.step_length = walking_stride;
    setting.walking.step_mode   = STEP_COUNTER_MODE_FIXED_LENGTH;
    setting.running.step_length = running_stride;
    setting.running.step_mode   = STEP_COUNTER_MODE_FIXED_LENGTH;

    set(&setting);
  }

  virtual int read_data(sensor_command_data_mh_t& data)
  {
    write(&data);
    return 0;
  }

private:
  const int walking_stride = 60; /* 60cm */
  const int running_stride = 80; /* 80cm */


};

#endif /* __STEPCOUNTERSENSOR__ */
