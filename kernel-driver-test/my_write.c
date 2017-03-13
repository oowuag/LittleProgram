#include <stdio.h>
#include <fcntl.h>

// my_write.c
int main(void)
{
 int fd, num;
 fd = open("/dev/my_driver", O_RDWR);

 if(fd != -1) {
  while(1) {
   printf("please input the my_driver!\n");
   scanf("%d",&num);
   write(fd,&num,sizeof(int));
   if(0 == num){
    close(fd);
    break;
   }
  }
 } else {
      printf("open my_driver failure\n");
 }

 return 0;
}

//gcc -g -I./ my_write.c -o my_write -L./ -lpthread -pthread -lrt