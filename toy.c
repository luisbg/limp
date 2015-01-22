#include <stdio.h>
#include <stdlib.h>

// 0xFFC0 location
// FFH D8H

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

void read_beginning (char *location)
{
  fileDesc *f;
  uint test = 0;
  int i;

  file_start (&f, location);
  printf ("file location: %s\n\n", f->filename);

  for (i = 0; i < 10; i++) {
    fseek (f->fp, i, SEEK_SET);
    fread (&test, 1, 1, f->fp);
    printf ("%02x\n", test);
  }

  file_close (&f);
}

int main(int argc, char *argv[])
{
  printf ("Let's play with jpeg image files!\n");

  char *location = argv[1];
  read_beginning (location);

  return 0;
}
