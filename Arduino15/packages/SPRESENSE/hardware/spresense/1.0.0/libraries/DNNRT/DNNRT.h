/*
 *  DNNRT.h - Spresense Arduino DNN runtime library 
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

#ifndef Dnnrt_h
#define Dnnrt_h

#ifdef SUBCORE
#error "DNNRT library is NOT supported by SubCore."
#endif

/**
 * @defgroup dnnrt DNN Library API
 * @brief API for using Deep Neural Network Library
 * @{
 */

#include <Arduino.h>

#include <dnnrt/runtime.h>

class DNNVariable; // forward reference
class File;

/**
 * @file DNNRT.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief Spresense Arduino DNN runtime library 
 * 
 * @details 
 *
 * 
 */

class DNNRT {

public:
  DNNRT() {};
  ~DNNRT() {};

  /**
   * Initialize runtime object from .nnb file
   *
   * User must be generate a network model data file (.nnb) by Neural Network
   * Console (NNC) before use this library.
   *
   * About Neural Network Console:
   *  https://dl.sony.com/
   *
   * @param nnbfile nnb network model binary file
   * @param cpu_num the number of CPUs to be used by DNN runtime (default 1)
   * @return 0 on success, otherwise error.
   * @retval -22(-EINVAL) invalid argument of cpu_num
   * @retval -16(-EBUSY) dnnrt-mp included in bootloader isn't installed,
   *                     or no memory space to load it.
   * @retval -1 no memory space to load nnbfile
   * @retval -2 communication error with dnnrt-mp
   * @retval -3 illegal input/output data in network model
   */  
  int begin(File &nnbfile, unsigned char cpu_num = 1);

  /**
   * Finalize runtime object
   *
   * @return 0 on success, otherwise error.
   */
  int end();

  /**
   * Set input data at index
   *
   * @param [in] var   Input data to the network
   * @param [in] index Index of input data
   * @return 0 on success, otherwise error.
   * @note Number of input data is depends on the network model.
   */
  int inputVariable(DNNVariable &var, unsigned int index);

  /**
   * Get output data at index
   *
   * @param [in] index Index of output data
   * @return Output variable data. the shape of output data is depends on the
   *         network model.
   */
  DNNVariable& outputVariable(unsigned int index);

  /**
   * Execute forward propagation
   *
   * @return 0 on success, otherwise error.
   */
  int forward();

  /**
   * Get number of network inputs
   *
   * @return Number of input data
   */
  int numOfInput();

  /**
   * Size of input data at index
   *
   * @param [in] index Index of input data
   * @return Number of input data elements
   */
  int inputSize(unsigned int index);

  /**
   * Get dimension of input data at index
   *
   * @param [in] index Index of input data
   * @return Number of input data dimension
   */
  int inputDimension(unsigned int index);

  /**
   * Shape size at shape index
   *
   * @param [in] index Index of input data
   * @param [in] shapeindex Index of shape
   * @return Shape size
   */
  int inputShapeSize(unsigned int index, unsigned int shapeindex);

  /**
   * Get number of network outputs
   *
   * @return Number of output data
   */
  int numOfOutput();

  /**
   * Size of output data at index
   *
   * @param [in] index Index of output data
   * @return Number of output data elements
   */
  int outputSize(unsigned int index);

  /**
   * Get dimension of output data at index
   *
   * @param [in] index Index of output data
   * @return Number of dimension
   */
  int outputDimension(unsigned int index);

  /**
   * Shape size at output shape index
   *
   * @param [in] index Index of output data
   * @param [in] shapeindex Index of output data
   * @return Shape size
   */
  int outputShapeSize(unsigned int index, unsigned int shapeindex);

private:
  dnn_runtime_t *_rt;            // DNN runtime context
  nn_network_t  *_network;       // Network data from .nnb file

  void         **_input;         // Input data array
  int            _nr_inputs;     // Number of input data
  DNNVariable   *_output;        // Output data array
  int            _nr_outputs;    // Number of output data
};

class DNNVariable {
  friend class DNNRT;

public:
  DNNVariable(unsigned int size);
  ~DNNVariable();

  /**
   * Return size of data array in elements
   *
   * @return Data array size
   */
  unsigned int size() {
    return _size;
  }

  const float& operator[](unsigned int index) const {
    return _data[index];
  }

  /**
   * Return data array pointer
   *
   * This function uses create input data from any target data
   * (image, sensing data, etc...)
   *
   * @return Pointer to data array in float
   */
  float *data() {
    return _data;
  }

  /**
   * Return array index to the elements in maximum value
   *
   * @return Array index
   */
  int maxIndex();

private:
  DNNVariable();
  float *_data;
  unsigned int _size;
  bool _allocated;
};

/** @} dnnrt */

#endif
