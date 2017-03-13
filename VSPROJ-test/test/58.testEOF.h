// "rb" "r" different
#include <stdio.h>
int main()
{
  int inchar;
  FILE *infile;
  char *inname = "x.txt";
 
  if ((infile = fopen(inname, "rb")) == NULL) {
    printf("Can't open input file %s with \"rb\".\n", inname);
    return 1;
  }
 
  printf("\n\n");
  printf("Here's the result when %s was opened with \"rb\":\n", inname);
  while ((inchar = fgetc(infile)) != EOF) {
    putchar(inchar);
  }
  printf("\n\n");

  printf("%d", fclose(infile));
 
  if ((infile = fopen(inname, "r")) == NULL) {
    printf("Can't open input file %s\n", inname);
    return 1;
  }
  printf("Here's the result when %s was opened with \"r\":\n", inname);
  while ((inchar = fgetc(infile)) != EOF) {
    putchar(inchar);
  }
  printf("\n\n");
  //fclose(infile);
  printf("%d", fclose(infile));
 
 
  return 0;
}