#ifndef __ACCELSENSOR__
#define __ACCELSENSOR__


class AccelSensor : public ApplicationSensor
{
public:
  AccelSensor(int                       id,
              unsigned int              subscriptions = 0,
              sensor_data_mh_callback_t callback      = NULL);

  int write_data(void *param);

};

#endif /* __ACCELSENSOR__ */
