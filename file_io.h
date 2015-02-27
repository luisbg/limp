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

#ifndef __FILE_IO_H__
#define __FILE_IO_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct
{
  char *filename;      /* filename */
  FILE *fp;            /* open file pointer */
  uint read_position;  /* position of fd */
  uint size;           /* file size */
} fileDesc;


void file_start (fileDesc **f, char *location);
void file_close (fileDesc **f);
uint8_t read_byte (fileDesc *f, int position);
void read_bytes (fileDesc *f, int position, int size, char *output);
void write_byte (fileDesc *f, int position, uint8_t *input);

#endif
