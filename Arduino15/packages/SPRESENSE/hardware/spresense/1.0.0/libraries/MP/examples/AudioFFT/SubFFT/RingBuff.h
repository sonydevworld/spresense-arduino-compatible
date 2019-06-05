/*
 *  RingBuff.h - MP Example for Audio FFT
 *  Copyright 2019 Sony Semiconductor Solutions Corporation
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

#ifndef _RINGBUFF_H_
#define _RINGBUFF_H_

class RingBuff
{
public:
  RingBuff(int sample) {
    int size = sample * sizeof(q15_t);
    _top = (q15_t*)malloc(size);
    _bottom = _top + sample;
    _wptr = _rptr = _top;
  };
  ~RingBuff() {
    free(_top);
  };

  int put(q15_t *buf, int sample) {
    if (sample < (_bottom - _wptr)) {
      arm_copy_q15(buf, _wptr, sample);
      _wptr += sample;
    } else {
      int part = _bottom - _wptr;
      arm_copy_q15(buf, _wptr, part);
      arm_copy_q15(&buf[part], _top, sample - part);
      _wptr = _top + sample - part;
    }
    //printf("[%4d+ %4d] (w:0x%08x+ r:0x%08x)\n", stored(), remain(), (uint32_t)_wptr, (uint32_t)_rptr);
    return sample;
  };

  int put(q15_t *buf, int sample, int chnum, int ch) {
    int i;
    if (sample < (_bottom - _wptr)) {
      for (i = 0; i < sample; i++) {
        _wptr[i] = buf[chnum * i + ch];
      }
      _wptr += sample;
    } else {
      int part = _bottom - _wptr;
      for (i = 0; i < part; i++) {
        _wptr[i] = buf[chnum * i + ch];
      }
      for (i = part; i < sample; i++) {
        _top[i - part] = buf[chnum * i + ch];
      }
      _wptr = _top + sample - part;
    }
    //printf("[%4d+ %4d] (w:0x%08x+ r:0x%08x)\n", stored(), remain(), (uint32_t)_wptr, (uint32_t)_rptr);
    return sample;
  };

  int get(float *buf, int sample) {
    if ((_rptr + sample) < _bottom) {
      arm_q15_to_float(_rptr, buf, sample);
      _rptr += sample;
    } else {
      int part = _bottom - _rptr;
      arm_q15_to_float(_rptr, buf, part);
      arm_q15_to_float(_top, &buf[part], sample - part);
      _rptr = _top + sample - part;
    }
    //printf("[%4d +%4d] (w:0x%08x +r:0x%08x)\n", stored(), remain(), (uint32_t)_wptr, (uint32_t)_rptr);
    return sample;
  };

  int remain() {
    return (_bottom - _top) - stored();
  };
  int stored() {
    return (_rptr <= _wptr) ?
      _wptr - _rptr : (_bottom - _rptr) + (_wptr - _top);
  };

private:
  q15_t *_top;
  q15_t *_bottom;
  q15_t *_wptr;
  q15_t *_rptr;
};

#endif
