/*
 *  DNNRT.cpp - Spresense Arduino DNN runtime library 
 *  Copyright 2018 Sony Semiconductor Solutions Corporation
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file DNNRT.cpp
 * @author Sony Semiconductor Solutions Corporation
 * @brief Spresense Arduino DNN runtime library 
 * 
 * @details It is a library for use Deep Neural Network (DNN).
 */

#include <Arduino.h>
#include <sdk/config.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <dnnrt/runtime.h>

#include <Arduino.h>
#include <DNNRT.h>
#include <File.h>

int
DNNRT::begin(File& nnbfile, unsigned char cpu_num)
{
  int ret;
  size_t size;
  dnn_config_t config;

  // Specify the number of CPUs to be used by DNN runtime

  if ((cpu_num < 1) || (cpu_num > 5))
    {
      return -EINVAL;
    }

  config.cpu_num = cpu_num;

  ret = dnn_initialize(&config);
  if (ret < 0)
    {
      return ret;
    }

  // Read whole data from network file

  size = nnbfile.size();
  _network = (nn_network_t *)malloc(size);
  if (!_network)
    {
      return -1;
    }

  ret = nnbfile.read(_network, size);
  if (ret < 0)
    {
      free(_network);
      _network = NULL;
      return -1;
    }

  _rt = (dnn_runtime_t *)malloc(sizeof(dnn_runtime_t));
  if (!_rt)
    {
      free(_network);
      _network = NULL;
      return -1;
    }

  ret = dnn_runtime_initialize(_rt, _network);
  if (ret < 0)
    {
      free(_network);
      free(_rt);

      _network = NULL;
      _rt = NULL;

      return -2;
    }

  // Get number of input/output data defined by network model.

  _nr_inputs = dnn_runtime_input_num(_rt);
  _nr_outputs = dnn_runtime_output_num(_rt);

  if (_nr_inputs <= 0 || _nr_outputs <= 0)
    {
      free(_network);
      free(_rt);

      _network = NULL;
      _rt = NULL;

      return -3;
    }

  // Allocate input and output data array from network model

  _input = (void **)malloc(sizeof(void *) * _nr_inputs);
  _output = new DNNVariable[_nr_outputs];

  return 0;
}

int
DNNRT::end()
{
  dnn_runtime_finalize(_rt);
  dnn_finalize();

  if (_network)
    {
      free(_network);
    }
  if (_rt)
    {
      free(_rt);
    }
  if (_input)
    {
      free(_input);
    }
  if (_output)
    {
      delete[] _output;
    }

  return 0;
}

int
DNNRT::inputVariable(DNNVariable &var, unsigned int index)
{
  if (index >= (unsigned int)_nr_inputs)
    {
      return -1;
    }

  _input[index] = var.data();

  return 0;
}

DNNVariable&
DNNRT::outputVariable(unsigned int index)
{
  return _output[index];
}

int
DNNRT::forward(void)
{
  int ret = dnn_runtime_forward(_rt, (const void **)_input, _nr_inputs);
  if (ret < 0)
    {
      return ret;
    }

  for (int i = 0; i < _nr_outputs; i++)
    {
      _output[i]._data = (float *)dnn_runtime_output_buffer(_rt, i);
      _output[i]._size = dnn_runtime_output_size(_rt, i);
    }

  return ret;
}

int
DNNRT::numOfInput(void)
{
  int ret = dnn_runtime_input_num(_rt);
  return ret;
}

int
DNNRT::inputSize(unsigned int index)
{
  int ret = dnn_runtime_input_size(_rt, index);
  return ret;
}

int
DNNRT::inputDimension(unsigned int index)
{
  int ret = dnn_runtime_input_ndim(_rt, index);
  return ret;
}

int
DNNRT::inputShapeSize(unsigned int index, unsigned int dindex)
{
  int ret = dnn_runtime_input_shape(_rt, index, dindex);
  return ret;
}

int
DNNRT::numOfOutput()
{
  int ret = dnn_runtime_output_num(_rt);
  return ret;
}

int
DNNRT::outputSize(unsigned int index)
{
  int ret = dnn_runtime_output_size(_rt, index);
  return ret;
}

int
DNNRT::outputDimension(unsigned int index)
{
  int ret = dnn_runtime_output_ndim(_rt, index);
  return ret;
}

int
DNNRT::outputShapeSize(unsigned int index, unsigned int dindex)
{
  int ret = dnn_runtime_output_shape(_rt, index, dindex);
  return ret;
}

////////////////////////////////////////////////////////////////////////////
// DNNVariable
////////////////////////////////////////////////////////////////////////////

DNNVariable::DNNVariable() :
  _data(0),
  _size(0),
  _allocated(false)
{
}

DNNVariable::DNNVariable(unsigned int size)
{
  _data = (float *)malloc(size * sizeof(float));
  _size = size;
  _allocated = true;
}

DNNVariable::~DNNVariable()
{
  /*
   * Free memory for allocated by constructor.
   * When use this object as output, DNNRT class set _data member
   * directory, so not free them.
   */

  if (_allocated)
    {
      free(_data);
    }
}

int
DNNVariable::maxIndex()
{
  float max = __FLT_MIN__;
  int index;

  if (_data == NULL)
    {
      return -1;
    }

  index = -1;
  for (int i = 0; i < (int)_size; i++)
    {
      if (max < _data[i])
        {
          max = _data[i];
          index = i;
        }
    }
  return index;
}
