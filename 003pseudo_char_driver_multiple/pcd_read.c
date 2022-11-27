#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<errno.h>
#include<stdio.h>

char buffer[2048];

#define TRY_READ 100

int main()
{
	int fd;

	/*this variable holds remaining data bytes to be read*/
	int remaining = TRY_READ;

	/*holds count of total data bytes read so far*/
	int total_read = 0;

	int n=0,ret=0;

	fd = open("/dev/pcdev-3",O_RDONLY);
	if(fd < 0)
	{
		/*perror decodes user space errno variable and cause of failure*/
		perror("open");
		goto out;
	}

	printf("Open was successful\n");

	while(n != 2 && remaining)
	{
		/*read data from fd*/
		ret = read(fd,&buffer[total_read],remaining);

		if(!ret){
			/*There is nothing to read*/
			printf("end of file \n");
			break;
		}
		else if(ret <= remaining){
			printf("read %d bytes of data\n",ret);
			/*'ret' contains count of data bytes successfully read, so add it to 'total_read'*/
			total_read += ret;

			/* We read some data, so decrement 'remaining'*/
			remaining -= ret;
		}
		else if(ret < 0){
			printf("Something went wrong\n");
			break;
		}
		else
			break;
		n++;
	}
	printf("total_read = %d\n",total_read);

	//dump buffer
	for(int i=0;i < total_read; i++)
		printf("%c",buffer[i]); 
	printf("\n");

out:
	close(fd);
	return 0;
}


