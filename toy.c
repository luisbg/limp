#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct
{
  int width;
  int height;
} header;

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

int check_is_jpeg (fileDesc *f)
{
  uint8_t beginning[2] = {0xff, 0xd8};
  uint8_t r;
  int i;

  for (i = 0; i < 2; i++) {
    fseek (f->fp, i, SEEK_SET);
    fread (&r, 1, 1, f->fp);
    if (r != beginning[i])
      return 0;
  }

  f->read_position = i;
  return 1;
}

int main(int argc, char *argv[])
{
  fileDesc *f;
  char *location = argv[1];

  printf ("Let's play with jpeg image files!\n");

  file_start (&f, location);
  printf ("file location: %s\n\n", f->filename);
  printf ("is it jpeg? %s\n", check_is_jpeg (f)? "yes" : "no");

  file_close (&f);
  return 0;
}
