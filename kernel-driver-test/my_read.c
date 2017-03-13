#include <stdio.h>
#include <fcntl.h>

// my_write.c
int main(void)
{
 int fd, num;
 fd = open("/dev/my_driver", O_RDWR);

 if(fd != -1) {
  while(1) {
   read(fd,&num,sizeof(int));
   printf("the my_driver is %d\n",num);
   if(0 == num) {
    close(fd);
    break;
   }
   sleep(1);
  }
 } else{
   printf("device open failure\n"); 
 }

 return 0;
}

//gcc -g -I./ my_read.c -o my_read -L./ -lpthread -pthread -lrt