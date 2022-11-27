#include<stdio.h>
#include<unistd.h>

int main()
{
	FILE *ptr;
	char ch;
	int choice;
	scanf("%d",&choice);
	switch(choice)
	{
		case 1:		

			ptr = fopen("/dev/pcdev-3","r");
			if(NULL == ptr)
			{
				printf("File can't be opened\n");
			}	

			printf("content of this file are \n",ptr);

		fclose(ptr);
		printf("\n");
		break;

                case 2:

                        ptr = fopen("/dev/pcdev-3","w");
                        if(NULL == ptr)
                        {
                                printf("File can't be opened\n");
                        }

                        printf("content of this file are \n");
                while(ch!=EOF)
                {
                        ch = fgetc(ptr);
                        printf("%c",ch);
                }
                fclose(ptr);
                printf("\n");
                break;

		case 3:
		ptr = fopen("/dev/pcdev-3","r");
		if(NULL == ptr)
		{
			printf("File can't be opened \n");
		}
		int i = fseek(ptr,-10,SEEK_SET);
		if(i < 0){
			perror("lseek");
			fclose(ptr);
		}
	}
}
