
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	//3FAD5C00 ===>ë„ï\1.354370V
	//char array[4] = {0x00,0x5c,0xAD,0x3F}; // JA039 CPU è¨í[äiéÆ

	//4136E610
	char array[4] = {0x10,0xE6,0x36,0x41}; // JA039 CPU è¨í[äiéÆ
	float f = *(float *)array;
	printf("f == %f \r\n",f);

	//4137FE83
	char array2[4] = {0x83,0xFE,0x37,0x41}; // JA039 CPU è¨í[äiéÆ
	float f2 = *(float *)array2;

	printf("f == %f \r\n",f2);
}

//f==1.354370
