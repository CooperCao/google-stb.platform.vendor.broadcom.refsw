/***************************************************************************
 * Copyright (c)2016 Broadcom. All rights reserved.
 *
 * Unless you and Broadcom execute a separate written software license agreement
 * governing use of this software, this software is licensed to you under the
 * terms of the GNU General Public License version 2 (GPLv2).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation (the "GPL").
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License version 2 (GPLv2) for more details.
 ***************************************************************************/

#ifndef ASTRA_VERSION_H
#define ASTRA_VERSION_H

/* current version numbers */
#define ASTRA_VERSION_MAJOR 1
#define ASTRA_VERSION_MINOR 1
#define ASTRA_VERSION_BUILD 0

/* utility macros */
#define ASTRA_VERSION_MAJOR_MASK  0xff000000
#define ASTRA_VERSION_MAJOR_SHIFT 24

#define ASTRA_VERSION_MINOR_MASK  0x00ff0000
#define ASTRA_VERSION_MINOR_SHIFT 16

#define ASTRA_VERSION_BUILD_MASK  0x0000ffff
#define ASTRA_VERSION_BUILD_SHIFT 0

#define ASTRA_VERSION_WORD                                  \
    ((ASTRA_VERSION_MAJOR << ASTRA_VERSION_MAJOR_SHIFT) |   \
     (ASTRA_VERSION_MINOR << ASTRA_VERSION_MINOR_SHIFT) |   \
     (ASTRA_VERSION_BUILD << ASTRA_VERSION_BUILD_SHIFT))

#endif /* ASTRA_VERSION_H */
