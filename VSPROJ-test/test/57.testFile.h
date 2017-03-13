#include <stdio.h>

int main()
{
	FILE* fp;

	while (1)
	{
		fp = fopen("/mnt/sdcard/aaabbbccc.txt","a+");
		if (fp == NULL) {
			printf("fopen fp:NULL")
		}
		char buff[20];
		memset(buff, 1, sizeof(buff));

		int wrSize = fwrite(buff, sizeof(buff), 1, fp);
		printf("fwrite size:%d", wrSize);

		int wrClose = fclose(fp);
		printf("fclose ret:%d", wrClose);

		sleep(200);
	}




	return 0;
}