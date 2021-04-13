/*
 *  multi_print.h - Spresense MultiCore printlog functions
 *  Copyright 2019,2021 Sony Semiconductor Solutions Corporation
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

#ifndef __MULTI_PRINT_H__
#define __MULTI_PRINT_H__

#include <sdk/config.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void init_multi_print(void);
irqstate_t printlock(void);
void printunlock(irqstate_t flags);
ssize_t uart_syncwrite(const char *buffer, size_t buflen);
int sync_printf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif  /* __MULTI_PRINT_H__ */
