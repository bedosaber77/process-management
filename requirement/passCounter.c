#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
int numberOflines;
int **arr; 

void readfromfile(const char *filename)
{
    FILE *fptr = fopen(filename, "r");
    if (!fptr)
    {
        fprintf(stderr, "File does not exist\n");
        exit(EXIT_FAILURE);
    }

    fscanf(fptr, "%d", &numberOflines);

    arr = (int **)malloc(numberOflines * sizeof(int *));
    for (int i = 0; i < numberOflines; i++)
        arr[i] = (int *)malloc(2 * sizeof(int));

    for (int i = 0; i < numberOflines; i++)
    {
        fscanf(fptr, "%d %d", &arr[i][0], &arr[i][1]);
    }

    fclose(fptr);
}

void freeMemory()
{
    for (int i = 0; i < numberOflines; i++)
        free(arr[i]);
    free(arr);
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        fprintf(stderr, "Usage: %s <filename> <num_processes> <min_grade>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *filename = argv[1];
    int nofProcess = atoi(argv[2]);
    int minGrade = atoi(argv[3]);

    readfromfile(filename);

    int shmid = shmget(IPC_PRIVATE, nofProcess * sizeof(int), IPC_CREAT | 0666);
    if (shmid == -1)
    {
        perror("Shared memory creation failed");
        exit(EXIT_FAILURE);
    }

    int *shm_arr = (int *)shmat(shmid, NULL, 0);
    if (shm_arr == (void *)-1)
    {
        perror("Shared memory attachment failed");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < nofProcess; i++)
    {   
        int pid = fork();
        if (pid < 0) {
            fprintf(stderr, "Fork failed for TA %d\n", i);
            exit(EXIT_FAILURE);
        } 
        else if (pid == 0) {  // Child process
            int passing = 0;
            int start = i * (numberOflines / nofProcess);
            int end = (i == nofProcess - 1) ? numberOflines : (i + 1) * (numberOflines / nofProcess);

            for (int j = start; j <end; j++)
            {
                if(arr[j][0]+arr[j][1] >= minGrade)
                    passing++;
            }
            shm_arr[i] = passing;

            shmdt(shm_arr);
            exit(0); 
        } 
    }
    
    for (int i = 0; i < nofProcess; i++)
    wait(NULL);

    for (int i = 0; i < nofProcess; i++)
        {
            char * out = (i == 0 )? "%d":" %d";
            printf(out,shm_arr[i]);
        }

    freeMemory();
    shmdt(shm_arr);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}
