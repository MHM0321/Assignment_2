#include<stdlib.h>
#include<pthread.h>
#include<stdio.h>
#include<stdbool.h>
#include<unistd.h>
#include<sys/syscall.h>
#include<sys/types.h>
#include<sys/wait.h>


typedef struct params
{
    int currSize;
    int*p;
    int k;

}params;


int main()
{
    pthread_t TID;
    int size;
    printf("Give array size :");
    scanf("%d", &size);

    int *arr = (int *)malloc(sizeof(int)*size);

    printf("\nGive array content :\n");
    for(int i = 0;i<size;i++)
    {
        scanf("%d", &arr[i]);
    }

    printf("Give sub-array number :");
    int subCont;
    scanf("%d", &subCont);








    return 0;
}