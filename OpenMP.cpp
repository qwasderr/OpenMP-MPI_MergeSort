#include <iostream>
#include <fstream>
#include <vector>
#include <omp.h>

void merge(std::vector<int>& arr, int l, int m, int r, std::vector<int>& tmp) {
    int i = l, j = m + 1, k = l;
    while (i <= m && j <= r) {
        tmp[k++] = (arr[i] < arr[j]) ? arr[i++] : arr[j++];
    }
    while (i <= m) tmp[k++] = arr[i++];
    while (j <= r) tmp[k++] = arr[j++];
    for (int x = l; x <= r; ++x) {
        arr[x] = tmp[x];
    }
}

void mergeSortParallel(std::vector<int>& arr, int l, int r, std::vector<int>& tmp, int depth = 0) {
    if (l < r) {
        int m = l + (r - l) / 2;
        #pragma omp task shared(arr, tmp) if(depth < 3)
        mergeSortParallel(arr, l, m, tmp, depth + 1);
        #pragma omp task shared(arr, tmp) if(depth < 3)
        mergeSortParallel(arr, m + 1, r, tmp, depth + 1);
        #pragma omp taskwait
        merge(arr, l, m, r, tmp);
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

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: ./merge_sort_omp <input_file> <num_threads>" << std::endl;
        return 1;
    }
    std::vector<int> arr;
    readFile(argv[1], arr);
    int n = arr.size();
    int num_threads = std::stoi(argv[2]);
    omp_set_num_threads(num_threads); 
    std::vector<int> tmp(n);
    double start_time = omp_get_wtime();
    #pragma omp parallel
    {
        #pragma omp single
        mergeSortParallel(arr, 0, n - 1, tmp); 
    }
    double end_time = omp_get_wtime();
    std::cout << "Execution time: " << (end_time - start_time) << " seconds" << std::endl;
    return 0;
}

