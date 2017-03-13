/* fgetc example: money counter */
#include <stdio.h>

int main(int argc, char * argv[])
{
	FILE * pFile;
	int c;
	int n = 0;
	//pFile=fopen (argv[1],"r");

	pFile=fopen ("C:\\SnsProj\\Refactor\\14cyTMAP_Refactor\\Refactor\\nhgps\\Engine\\nutshell.mk","r");


	if (pFile==NULL) perror ("Error opening file");
	else
	{
		do {
			c = fgetc (pFile);
			n++;
			if (c > 0x7F ) 
			{
				//printf("#########################################\n#########Error : %s(%d)\n", argv[1], n);
				break;
			}

			printf("%c",c);
			
		} while (c != EOF);
		fclose (pFile);
		//printf ("The file contains %d dollar sign characters ($).\n",n);
	}
	return 0;
}