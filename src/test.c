#include <stdio.h>
#include <stdlib.h>

typedef struct Data {
    int *arr;
} Data;

typedef struct DataArr {
    Data *datas;
} DataArr;

void createData(Data *data, DataArr *dataArr) {
    data->arr = malloc(sizeof(int) * 10);
    for (int i = 0; i < 10; i++) {
        data->arr[i] = i;
    }
}

void createDataArr(DataArr *dataArr) {
    dataArr->datas = malloc(2 * sizeof(Data));
}

int main() {
    DataArr dataArr = {0};
    Data data = {0};
    createDataArr(&dataArr);
    createData(&data, &dataArr);
    for (int i = 0; i < 10; i++) {
        printf("%d\n", data.arr[i]);
    }
}
