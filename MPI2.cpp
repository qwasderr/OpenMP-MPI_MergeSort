#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#define INPUT_FILE "input.txt"

int *mergeArrays(int *leftArray, int leftSize, int *rightArray, int rightSize) {
    int leftIndex = 0, rightIndex = 0, mergedIndex = 0;
    int *mergedArray;
    int mergedSize = leftSize + rightSize;

    mergedArray = (int *)malloc(mergedSize * sizeof(int));

    while ((leftIndex < leftSize) && (rightIndex < rightSize)) {
        if (leftArray[leftIndex] <= rightArray[rightIndex]) {
            mergedArray[mergedIndex++] = leftArray[leftIndex++];
        } else {
            mergedArray[mergedIndex++] = rightArray[rightIndex++];
        }
    }

    while (leftIndex < leftSize)
        mergedArray[mergedIndex++] = leftArray[leftIndex++];
    while (rightIndex < rightSize)
        mergedArray[mergedIndex++] = rightArray[rightIndex++];

    return mergedArray;
}

void performSort(int *arr, int left, int right) {
    if (left >= right) return;

    int mid = (left + right) / 2;
    performSort(arr, left, mid);
    performSort(arr, mid + 1, right);

    int leftPartSize = mid - left + 1;
    int rightPartSize = right - mid;
    int *sortedArray = mergeArrays(arr + left, leftPartSize, arr + mid + 1, rightPartSize);

    for (int i = 0; i < leftPartSize + rightPartSize; i++)
        arr[left + i] = sortedArray[i];

    free(sortedArray);
}

int main(int argc, char **argv) {
    int *globalArray = NULL;
    int *localArray;
    int *receivedArray;
    int totalElements = 0, localSize;
    int processRank, totalProcesses;
    int mergeStep;
    double startTime, endTime;
    MPI_Status mpiStatus;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &processRank);
    MPI_Comm_size(MPI_COMM_WORLD, &totalProcesses);

    if (processRank == 0) {
        FILE *inputFile = fopen(INPUT_FILE, "r");
        int temp;
        while (fscanf(inputFile, "%d", &temp) == 1) {
            totalElements++;
        }
        globalArray = (int *)malloc(totalElements * sizeof(int));
        rewind(inputFile);
        for (int i = 0; i < totalElements; i++)
            fscanf(inputFile, "%d", &globalArray[i]);

        fclose(inputFile);
    }

    MPI_Bcast(&totalElements, 1, MPI_INT, 0, MPI_COMM_WORLD);

    localSize = totalElements / totalProcesses;
    int remainder = totalElements % totalProcesses;
    if (processRank < remainder) localSize++;

    localArray = (int *)malloc(localSize * sizeof(int));

    int *sendCounts = NULL;
    int *displacements = NULL;
    if (processRank == 0) {
        sendCounts = (int *)malloc(totalProcesses * sizeof(int));
        displacements = (int *)malloc(totalProcesses * sizeof(int));

        int currentIndex = 0;
        for (int i = 0; i < totalProcesses; i++) {
            sendCounts[i] = totalElements / totalProcesses + (i < remainder ? 1 : 0);
            displacements[i] = currentIndex;
            currentIndex += sendCounts[i];
        }
    }

    MPI_Scatterv(globalArray, sendCounts, displacements, MPI_INT, localArray, localSize, MPI_INT, 0, MPI_COMM_WORLD);

    startTime = MPI_Wtime();
    performSort(localArray, 0, localSize - 1);

    mergeStep = 1;
    while (mergeStep < totalProcesses) {
        if (processRank % (2 * mergeStep) == 0) {
            if (processRank + mergeStep < totalProcesses) {
                int receivedSize;
                MPI_Recv(&receivedSize, 1, MPI_INT, processRank + mergeStep, 0, MPI_COMM_WORLD, &mpiStatus);
                receivedArray = (int *)malloc(receivedSize * sizeof(int));
                MPI_Recv(receivedArray, receivedSize, MPI_INT, processRank + mergeStep, 0, MPI_COMM_WORLD, &mpiStatus);
                localArray = mergeArrays(localArray, localSize, receivedArray, receivedSize);
                localSize += receivedSize;
                free(receivedArray);
            }
        } else {
            int targetProcess = processRank - mergeStep;
            MPI_Send(&localSize, 1, MPI_INT, targetProcess, 0, MPI_COMM_WORLD);
            MPI_Send(localArray, localSize, MPI_INT, targetProcess, 0, MPI_COMM_WORLD);
            break;
        }
        mergeStep *= 2;
    }

    endTime = MPI_Wtime();

    if (processRank == 0) {
        printf("\nNumber of elements: %d; Number of processes: %d; Time: %f\n\n", totalElements, totalProcesses, endTime - startTime);
        free(globalArray);
    }

    free(localArray);
    if (processRank == 0) {
        free(sendCounts);
        free(displacements);
    }

    MPI_Finalize();
    return 0;
}

