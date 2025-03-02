#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

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

    for (int i = 0; i < nofProcess; i++)
    {
        int pid = fork();
        if (pid < 0)
        {
            fprintf(stderr, "Fork failed for TA %d\n", i);
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        { // Child process
            int passing = 0;
            int start = i * (numberOflines / nofProcess);
            int end = (i == nofProcess - 1) ? numberOflines : (i + 1) * (numberOflines / nofProcess);

            for (int j = start; j < end; j++)
            {
                if (arr[j][0] + arr[j][1] >= minGrade)
                    passing++;
            }

            exit(passing);
        }
    }

    for (int i = 0; i < nofProcess; i++)
    {
        int status;
        int child_pid = wait(&status);
        if (WIFEXITED(status))
        {
            int result = WEXITSTATUS(status);
            char *s = ((i == 0) ? "%d" : (i == nofProcess - 1) ? " %d\n"
                                                               : " %d");
            printf(s, result);
        }
    }

    freeMemory();
    return 0;
}
