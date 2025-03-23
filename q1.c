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

void* sort(void* obj)
{
    params *ptr = (params *)obj;

    for (int i = 1; i < ptr->currSize; i++) //insertion sort
    {
        int key = ptr->p[i];
        int j = i - 1;

        while (j >= 0 && ptr->p[j] > key)
        {
            ptr->p[j + 1] = ptr->p[j];
            j--;
        }
        ptr->p[j + 1] = key;
    }



}

void* merge(void* obj)
{
    params *p = (params *)obj;


}


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


    params obj;
    obj.currSize = size;
    obj.p = arr;
    obj.k = subCont;


    for(int i = 0; i < subCont; i++)
    {
        
    }



    return 0;
}