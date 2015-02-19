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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
} segments;            /* Segments */

typedef struct
{
  char *filename;      /* filename */
  FILE *fp;            /* open file pointer */
  uint read_position;  /* position of fd */
  uint size;           /* file size */
} fileDesc;

int markers[7] = {0xd8,     // start of image
                  0xe0,     // app0
                  0xdb,     // quantization table
                  0xc0,     // start of frame
                  0xc4,     // huffman table
                  0xda,     // start of scan
                  0xd9};    // end of image
segments seg;

void init_seg () {
  seg.soi = 0;
  seg.app = 0;
  seg.dqt = calloc (81, sizeof (int));
  seg.sof = calloc (32768, sizeof(int));
  seg.ht =  calloc (16, sizeof (int));
  seg.sos = calloc (8, sizeof (int));
  seg.eoi = 0;
}

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

int check_is_jpeg (fileDesc *f)
{
  uint8_t soi[2] = {0xff, 0xd8};    /* Start Of Image marker */
  uint8_t eoi[2] = {0xd9, 0xff};    /* End Of Image marker */
  uint8_t r;
  int i;

  for (i = 0; i < 2; i++) {
    r = read_byte (f, i);
    if (r != soi[i])
      return 0;
  }

  for (i = 1; i < 3; i++) {
    fseek (f->fp, -1 * i, SEEK_END);
    fread (&r, 1, 1, f->fp);
    if (r != eoi[i - 1])
      return 0;
  }

  return 1;
}

/* Get the Start of Frame */
sof * get_sof (fileDesc *f)
{
  sof *ret = (sof *) malloc (sizeof (sof));
  uint16_t sof[2] = {0xff, 0xc0};
  uint16_t r0 = 0, r1 = 0;
  int pos = seg.sof[0];

  pos += 5;    /* in sof, skip segment length and precision */
  r0 = read_byte (f, pos);    /* read height of image */
  pos++;
  r1 = read_byte (f, pos);
  ret->height = (r0 * 256) + r1;

  pos++;
  r0 = read_byte (f, pos);    /* read width of image */
  pos++;
  r1 = read_byte (f, pos);
  ret->width = (r0 * 256) + r1;

  pos++;
  ret->num_components = read_byte (f, pos);

  return ret;
}

void find_markers (fileDesc *f)
{
  uint16_t marker_start = 0xff;
  uint16_t dqt[2] = {0xff, 0xdb};
  uint16_t r0 = 0, r1 = 0;
  int n, m, j, found;

  for (n = 0; n < f->size; n++) {
    r0 = read_byte (f, n);
    if (r0 == marker_start) {    // 0xff starts a segment
      r1 = read_byte (f, n + 1);

      if (r1 == 0x00)    // escape marker. not a segment.
        continue;

      found = 0;
      for (m = 0; m < 8; m++) {  // compare with markers
        if (r1 == markers[m]) {
          found = 1;
          break;
        }
      }
      if (!found)
        continue;

      switch (m) {    // found a segment, add it to seg
        case 0:
          seg.soi = n;
          break;
        case 1:
          seg.app = n;
          break;
        case 2:
          for (j = 0; seg.dqt[j] != 0; j++);
          seg.dqt[j] = n;
          break;
        case 3:
          for (j = 0; seg.sof[j] != 0; j++);
          seg.sof[j] = n;
          break;
        case 4:
          for (j = 0; seg.ht[j] != 0; j++);
          seg.ht[j] = n;
          break;
        case 5:
          for (j = 0; seg.sos[j] != 0; j++);
          seg.sos[j] = n;
          break;
        case 6:
          seg.eoi = n;
          break;
        default:
          break;
      }
    }
  }

  printf ("soi %d\n", seg.soi);
  printf ("app %d\n", seg.app);

  printf ("dqt %d.", seg.dqt[0]);
  for (j = 1; seg.dqt[j] != 0; j++)
    printf ("  %d.", seg.dqt[j]);
  printf ("\n");

  printf ("sof %d.", seg.sof[0]);
  for (j = 1; seg.sof[j] != 0; j++)
    printf (" %d.", seg.sof[j]);
  printf ("\n");

  printf ("ht  %d.", seg.ht[0]);
  for (j = 1; seg.ht[j] != 0; j++)
    printf ("  %d.", seg.ht[j]);
  printf ("\n");

  printf ("sos %d.", seg.sos[0]);
  for (j = 1; seg.sos[j] != 0; j++)
    printf ("  %d.", seg.sos[j]);
  printf ("\n");

  printf ("eoi %d\n", seg.eoi);
}

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

  file_close (&f);
  return 0;
}
