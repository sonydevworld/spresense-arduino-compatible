#ifndef __APPLICATIONSENSOR__
#define __APPLICATIONSENSOR__

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nuttx/init.h>
#include <nuttx/arch.h>
#include <asmp/mpshm.h>

#include <pins_arduino.h>

#include <sensing/sensor_api.h>
#include <sensing/sensor_id.h>
#include <sensing/sensor_ecode.h>

#include <MemoryUtil.h>

#include "memutil/msgq_id.h"
#include "memutil/mem_layout.h"


class ApplicationSensor
{
public:
  ApplicationSensor(int                       id,
                    unsigned int              subscriptions = 0,
                    sensor_data_mh_callback_t callback      = NULL);

  virtual int write_data(void *param)
  {
    return 0;
  }

  virtual int read_data(sensor_command_data_mh_t& data)
  {
    return 0;
  }

protected:
  int write_data_mh(FAR void* data,
                    uint32_t  sample_size,
                    uint32_t  sample_freq,
                    uint32_t  sample_num,
                    uint32_t  timestamp);

private:
  int m_id;

};

#endif /* __APPLICATIONSENSOR__ */
