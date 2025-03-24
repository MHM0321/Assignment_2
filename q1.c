#include<stdlib.h>
#include<pthread.h>
#include<stdio.h>
#include<stdbool.h>
#include<unistd.h>
#include<sys/syscall.h>
#include<sys/types.h>
#include<sys/wait.h>

typedef struct
{
    int* arr;
    int first;
    int second;
} SortArgs;


void insertionSort(int* arr, int first, int second)
{
    for (int i = first + 1; i <= second; i++)
    {
        int key = arr[i];
        int j = i - 1;
        while (j >= first && arr[j] > key)
        {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}


void* sortThread(void* args)        //thread
{
    SortArgs* data = (SortArgs*)args;
    insertionSort(data->arr, data->first, data->second);
    pthread_exit(NULL);
}


void merge(int* arr, int first, int mid, int second)
{
    int n1 = mid - first + 1;
    int n2 = second - mid;
    int* firstArr = (int*)malloc(n1 * sizeof(int));
    int* secondArr = (int*)malloc(n2 * sizeof(int));

    for (int i = 0; i < n1; i++)
    {
        firstArr[i] = arr[first + i];
    }
    for (int i = 0; i < n2; i++)
    {
        secondArr[i] = arr[mid + 1 + i];
    }

    int i = 0, j = 0, k = first;
    while (i < n1 && j < n2)
    {
        if (firstArr[i] <= secondArr[j])
        {
            arr[k++] = firstArr[i++];
        }
        else
        {
            arr[k++] = secondArr[j++];
        }
    }

    while (i < n1) arr[k++] = firstArr[i++];
    while (j < n2) arr[k++] = secondArr[j++];

    free(firstArr);
    free(secondArr);
}


void* mergeThread(void* args)       //thread
{
    SortArgs* data = (SortArgs*)args;
    int mid = (data->first + data->second) / 2;
    merge(data->arr, data->first, mid, data->second);
    pthread_exit(NULL);
}

int main()
{
    printf("Give size of array : ");
    int n;
    scanf("%d", &n);
    int arr[n];

    printf("\nGive elements of the array :\n");
    for(int i = 0;i<n;i++)
    {
        scanf("%d", &arr[i]);
    }
    
    int k;
    printf("Give number of subarrays : ");
    scanf("%d", &k);
    

    if (n < k) {
        printf("k should be <= array size\n");
        return 1;
    }

    int subSize = (n + k - 1) / k;      // sze of each subarray
    pthread_t sortThreads[k];
    SortArgs sortArgs[k];

    for (int i = 0; i < k; i++)
    {
        sortArgs[i].arr = arr;
        sortArgs[i].first = i * subSize;
        sortArgs[i].second = (i + 1) * subSize - 1;
        if (sortArgs[i].second >= n)
        {
            sortArgs[i].second = n - 1;
        }

        pthread_create(&sortThreads[i], NULL, sortThread, &sortArgs[i]);
    }

    for (int i = 0; i < k; i++) pthread_join(sortThreads[i], NULL);

    int numSubarrays = k;
    while (numSubarrays > 1)
    {
        int newNumSubarrays = numSubarrays / 2;
        pthread_t mergeThreads[newNumSubarrays];
        SortArgs mergeArgs[newNumSubarrays];

        for (int i = 0; i < newNumSubarrays; i++)
        {
            mergeArgs[i].arr = arr;
            mergeArgs[i].first = i * 2 * subSize;
            mergeArgs[i].second = (i * 2 + 2) * subSize - 1;
            if (mergeArgs[i].second >= n) mergeArgs[i].second = n - 1;

            pthread_create(&mergeThreads[i], NULL, mergeThread, &mergeArgs[i]);
        }

        for (int i = 0; i < newNumSubarrays; i++)
        {
            pthread_join(mergeThreads[i], NULL);
        }

        numSubarrays /= 2;
        subSize *= 2;
    }

    
    printf("Final Sorted Array: ");
    for (int i = 0; i < n; i++)
    {
        printf("%d ", arr[i]);
    }
    printf("\n");

    return 0;
}
