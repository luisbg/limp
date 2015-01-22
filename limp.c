#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct
{
  int width;
  int height;
} sof;                 /* Start of Frame */

typedef struct
{
  char *filename;      /* filename */
  FILE *fp;            /* open file pointer */
  uint read_position;  /* position of fd */
} fileDesc;

void file_start (fileDesc **f, char *location)
{
  *f = (fileDesc *) malloc (sizeof (fileDesc));
  (*f)->filename = location;
  (*f)->fp = NULL;
  (*f)->read_position = 0;

  (*f)->fp = fopen ((*f)->filename, "rb");
  if (!(*f)->fp) {
    perror ("Unable to open file :(\n");
    return;
  }
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

sof * get_sof (fileDesc *f)
{
  sof *ret = (sof *) malloc (sizeof (sof));
  uint16_t sof[2] = {0xff, 0xc0};
  uint16_t r0 = 0, r1 = 0;
  int i = 0, found = 0;

  while (!found) {
    r0 = read_byte (f, i);
    if (r0 == sof[0]) {
      r1 = read_byte (f, i + 1);
      if (r1 == sof[1])
        break;    /* i = location of sof */
    }

    i++;
  }

  i += 5;    /* in sof, skip segment length and precision */
  r0 = read_byte (f, i);    /* read height of image */
  i++;
  r1 = read_byte (f, i);
  ret->height = (r0 * 256) + r1;

  i++;
  r0 = read_byte (f, i);    /* read width of image */
  i++;
  r1 = read_byte (f, i);
  ret->width = (r0 * 256) + r1;

  return ret;
}

int main(int argc, char *argv[])
{
  fileDesc *f;
  char *location = argv[1];

  printf ("Let's play with jpeg image files!\n");

  file_start (&f, location);
  printf ("file location: %s\n\n", f->filename);

  if (!check_is_jpeg (f)) {
    printf ("File is not jpeg\nClosing\n");
    return 0;
  }

  sof *s = get_sof (f);
  printf ("height is %d\n", s->height);
  printf ("width is %d\n", s->width);

  file_close (&f);
  return 0;
}
