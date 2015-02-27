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

#include "jpeg.h"
#include "limp.h"
#include <time.h>

#define NUM_MARKERS 9

int markers[NUM_MARKERS] = {0xd8,     // start of image
                            0xe0,     // app0
                            0xdb,     // quantization table
                            0xc0,     // start of frame - baseline
                            0xc2,     // start of frame - progressive
                            0xc4,     // huffman table
                            0xda,     // start of scan
                            0xfe,     // comment
                            0xd9};    // end of image
segments seg;

void init_seg ()
{
  seg.soi = 0;
  seg.app = 0;
  seg.dqt = calloc (81, sizeof (int));
  seg.sof = calloc (32768, sizeof(int));
  seg.ht =  calloc (16, sizeof (int));
  seg.sos = calloc (8, sizeof (int));
  seg.eoi = 0;
  seg.mlist = calloc (327880, sizeof (int));
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

/* Get comment */
char * get_comment (fileDesc *f, segments seg)
{
  char *ret = NULL;
  uint16_t size = 0;
  int pos = seg.comment;

  if (pos) {
    pos += 2;    /* skip comment segment marker */
    size = read_byte (f, pos);
    pos++;
    size = (size * 256) + read_byte (f, pos) -1;
    printf ("size of comment: %d\n", size);

    ret = (char *) calloc (size, sizeof (char));
    read_bytes (f, pos, size, ret);
    ret[size] = '\0';
  }

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

      for (j = 0; seg.mlist[j] != 0; j++);
      seg.mlist[j] = n;

      found = 0;
      for (m = 0; m < NUM_MARKERS; m++) {  // compare with markers
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
        case 4:
          seg.progressive = (m == 4);

          for (j = 0; seg.sof[j] != 0; j++);
          seg.sof[j] = n;
          break;
        case 5:
          for (j = 0; seg.ht[j] != 0; j++);
          seg.ht[j] = n;
          break;
        case 6:
          for (j = 0; seg.sos[j] != 0; j++);
          seg.sos[j] = n;
          break;
        case 7:
          seg.comment = n;
          break;
        case 8:
          seg.eoi = n;
          break;
        default:
          break;
      }
    }
  }

  printf ("soi %#06x\n", seg.soi);
  printf ("app %#06x\n", seg.app);

  printf ("dqt %#06x.", seg.dqt[0]);
  for (j = 1; seg.dqt[j] != 0; j++)
    printf ("  %#06x.", seg.dqt[j]);
  printf ("\n");

  printf ("sof %#06x %#06x.", seg.progressive? "progressive": "baseline", seg.sof[0]);
  for (j = 1; seg.sof[j] != 0; j++)
    printf (" %#06x.", seg.sof[j]);
  printf ("\n");

  printf ("ht  %#06x.", seg.ht[0]);
  for (j = 1; seg.ht[j] != 0; j++)
    printf ("  %#06x.", seg.ht[j]);
  printf ("\n");

  printf ("sos %#06x.", seg.sos[0]);
  for (j = 1; seg.sos[j] != 0; j++)
    printf ("  %#06x.", seg.sos[j]);
  printf ("\n");

  if (!seg.comment)
    printf ("no comment\n", seg.comment);
  else
    printf ("comment %#06x: %s\n", seg.comment, get_comment (f, seg));

  printf ("eoi %#06x\n", seg.eoi);
}

/* return a random number between 0 and limit inclusive. */
int rand_lim(int limit) {
  int divisor = RAND_MAX/(limit+1);
  int ret;

  srand ( time(NULL) );
  do {
    ret = rand() / divisor;
  } while (ret > limit);

  return ret;
}

void mess_with_sos (fileDesc *f, int entropy)
{
  uint16_t size = 0;
  uint8_t v;
  int num_changes, total_num_changes = 0;
  int sos, pos;
  int j, k;

  for (j = 0; seg.sos[j] != 0; j++) {
    sos = seg.sos[j];

    for (k = 0; seg.mlist[k] != sos  && seg.mlist[k] == 0; j++);
    if (seg.mlist[k] == 0)
      return;

    size = seg.mlist[k + 1] - sos - 14;

    num_changes = size * ((float) entropy / 100);
    total_num_changes += num_changes;
    for (k = 0; k < num_changes; k++) {
      pos = sos + rand_lim (size) + 14;
      v = rand_lim (255);
      write_byte (f, pos, &v);
    }
  }

  printf ("num of changes made: %d\n", total_num_changes);

  return;
}
