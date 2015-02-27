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
#include "jpeg.h"
#include "limp.h"


int main (int argc, char *argv[])
{
  fileDesc *f;
  char *location = argv[1];

  printf ("Let's play with jpeg image files!\n");

  file_start (&f, location);
  if (!check_is_jpeg (f)) {
    printf ("File is not jpeg\nClosing\n");
    return 0;
  }

  init_seg ();
  find_markers (f);

  sof *s = get_sof (f);
  printf ("height is %d\n", s->height);
  printf ("width is %d\n", s->width);
  if (s->num_components == 3)
    printf ("color image\n");
  else
    printf ("grayscale image\n");

  mess_with_sos (f, 5);

  file_close (&f);
  return 0;
}
