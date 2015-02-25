/*
 * Limp
 *
 * Copyright (C) 2015 Luis de Bethencourt <luis@debethencourt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this source code; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "file_io.h"

void file_start (fileDesc **f, char *location)
{
  *f = (fileDesc *) malloc (sizeof (fileDesc));
  (*f)->filename = location;
  (*f)->fp = NULL;
  (*f)->read_position = 0;

  printf ("opening file: %s\n", location);

  (*f)->fp = fopen ((*f)->filename, "rb");
  if (!(*f)->fp) {
    perror ("Unable to open file :(\n");
    return;
  }

  fseek((*f)->fp, 0, SEEK_END);
  (*f)->size = ftell((*f)->fp);
  printf ("file size: %d\n", (*f)->size);
}

void file_close (fileDesc **f)
{
  fclose ((*f)->fp);
  free (*f);
}

uint8_t read_byte (fileDesc *f, int position)
{
  uint8_t ret;

  fseek (f->fp, position, SEEK_SET);
  fread (&ret, 1, 1, f->fp);

  return ret;
}
