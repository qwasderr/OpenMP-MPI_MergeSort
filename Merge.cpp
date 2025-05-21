#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>

void merge(std::vector<int>& arr, std::vector<int>& temp, int left, int mid, int right) {
    int i = left, j = mid + 1, k = left;
    while (i <= mid && j <= right) {
        if (arr[i] <= arr[j]) {
            temp[k++] = arr[i++];
        } else {
            temp[k++] = arr[j++];
        }
    }
    while (i <= mid) {
        temp[k++] = arr[i++];
    }
    while (j <= right) {
        temp[k++] = arr[j++];
    }
    for (i = left; i <= right; i++) {
        arr[i] = temp[i];
    }
}

void mergeSort(std::vector<int>& arr, std::vector<int>& temp, int left, int right) {
    if (left < right) {
        int mid = (left + right) / 2;
        mergeSort(arr, temp, left, mid);
        mergeSort(arr, temp, mid + 1, right);
        merge(arr, temp, left, mid, right);
    }
}

void readFile(const std::string& filename, std::vector<int>& arr) {
    std::ifstream file(filename);
    int num;
    while (file >> num) {
        arr.push_back(num);
    }
    file.close();
}

int main() {
    std::vector<int> arr;
    readFile("input.txt", arr);
    std::vector<int> temp(arr.size());
    clock_t start_time = clock();
    mergeSort(arr, temp, 0, arr.size() - 1);
    clock_t end_time = clock();
    double execution_time = double(end_time - start_time) / CLOCKS_PER_SEC;
    std::cout << "Execution time: " << execution_time << " seconds" << std::setprecision(10) << std::endl;

    return 0;
}

