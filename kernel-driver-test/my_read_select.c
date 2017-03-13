#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>

// my_write.c
int main(void)
{
    int fd, num;
    fd = open("/dev/my_driver", O_RDWR);

    if(fd != -1) {
        while(1) {
            struct timeval tm;
            fd_set fdset;
            int highestfd = -1;

            tm.tv_sec = 1;
            tm.tv_usec = 0;

            highestfd = fd;

            FD_ZERO(&fdset);
            int ret = select((int)highestfd+1, &fdset, NULL, NULL, &tm);

            /* Check if this is a timeout (0) or error (-1) */
            if (ret < 1) {
                printf("timeout (0) or error (-1)\n");
                continue;
            }

            if (fd >= 0 && FD_ISSET(fd, &fdset)) {
                printf("[%d] socket data received\n", fd);

                read(fd,&num,sizeof(int));
                printf("the my_driver is %d\n",num);
                if(-1 == num) {
                    close(fd);
                    break;
                }
            }
            sleep(1);
        }
    } else{
        printf("device open failure\n");
    }

    return 0;
}

//gcc -g -I./ my_read_select.c -o my_read_select -L./ -lpthread -pthread -lrt