/*
* Copyright (C) 2010 The Android Open Source Project
* Copyright (C) 2012-2015, The Linux Foundation. All rights reserved.
*
* Not a Contribution, Apache license notifications and license are retained
* for attribution purposes only.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef __AUD_COMMON_H__
#define __AUD_COMMON_H__

#define 	LOCAL		static
#define	PUBLIC
#define 	SCI_TRUE	1
#define	SCI_FALSE	0
#define 	SCI_ASSERT(condition)
#define	SCI_NULL	0
#define PNULL  ((void *)0)
#define		CONST		const
#define REG32(addr)		(*(volatile unsigned int *) (addr))

#define udelay  usleep

typedef  unsigned char 		BOOLEAN;

typedef unsigned char  uint8_t;
typedef    short    int16_t;
typedef    int    int32_t;


#endif
