/*
 * Limp
 *
 * Copyright (C) 2015 Luis de Bethencourt <luis@debethencourt.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef __JPEG_H__
#define __JPEG_H__

#include <stdlib.h>
#include "file_io.h"

typedef struct
{
  int width;
  int height;
  int num_components;
} sof;                 /* Start of Frame */

typedef struct
{
  int soi;
  int app;
  int* dqt;
  int* sof;
  int* ht;
  int* sos;
  int eoi;
  int progressive;
} segments;            /* Segments */


void init_seg ();
void file_start (fileDesc **f, char *location);
int check_is_jpeg (fileDesc *f);
sof * get_sof (fileDesc *f);
void find_markers (fileDesc *f);

#endif
