#include <time.h>
#include <iostream>
#include <set>
#include <stdlib.h>
#include <conio.h>
#include <QDateTime>

using namespace std;

const char* FILE_NAME = "data50m.bin";
const int SIZE = 50000000;
const int BUFFER_SIZE = 1000000;

struct Row {
    int id;
    char category[80];
};

class Database {
    Row **data;
    int blocksCount;

public:
    Row* at(int index) {
        auto block = index / BUFFER_SIZE;
        auto offset = index % BUFFER_SIZE;

        return &(data[block][offset]);
    }

    bool load() {
        auto fp = fopen(FILE_NAME, "rb");
        if (!fp) {
            printf("Cannot open %s for reading!", FILE_NAME);
            return false;
        }

        fseek(fp, 0, SEEK_SET);

        blocksCount = SIZE / BUFFER_SIZE;
        if (SIZE % BUFFER_SIZE != 0) {
            blocksCount++;
        }

        data = new Row*[blocksCount];
        for (auto i = 0; i < blocksCount; i++) {
            data[i] = new Row[BUFFER_SIZE];
            fread(data[i], sizeof(Row), BUFFER_SIZE, fp);
        }

        fclose(fp);

        return true;
    }
};

bool generate(int size)
{
    auto fp = fopen(FILE_NAME, "wb");
    if (!fp) {
        printf("Cannot open %s for writing!", FILE_NAME);
        return false;
    }

    char* categories[] = {
        "Бакалея, сахар, соль",
        "Консервы"
    };

    auto blocksCount = SIZE / BUFFER_SIZE;
    if (SIZE % BUFFER_SIZE != 0) {
        blocksCount++;
    }

    Row *array = new Row[BUFFER_SIZE];
    for (auto i = 0; i < blocksCount; i++) {
        for (auto i = 0; i < BUFFER_SIZE; i++) {
            array[i].id = rand();
            strcpy(array[i].category, categories[rand() % 2]);
        }
        fwrite(array, sizeof(Row), BUFFER_SIZE, fp);
    }
    delete[] array;

    fclose(fp);

    return true;
}

//bool load(Row **container)
//{
//    auto fp = fopen(FILE_NAME, "rb");
//    if (!fp) {
//        printf("Cannot open %s for reading!", FILE_NAME);
//        return false;
//    }

//    fseek(fp, 0, SEEK_SET);

//    *container = new Row[SIZE];
//    fread(*container, sizeof(Row), SIZE, fp);

//    fclose(fp);

//    return true;
//}

void writeLine(char *line)
{
    cout << line << endl;
}

void writeLine(time_t val)
{
    cout << val << endl;
}

int main(int argc, char *argv[])
{
    /* initialize random seed: */
    srand(time(NULL));

    // Run
//    if (!generate(SIZE))
//        return 1;
//    else
//        return 0;

    Database db;

    cout << "Loading file..." << endl;
    if (!db.load())
        return 2;

    //    // set some initial values:
    //    int r = rand() * rand();
    //    for (int i = 1; i <= 1000000; i++) {
    //        myset.insert(val);
    //    }

    //    for (it = myset.begin(); it != myset.end(); it++) {
    //        cout << *it << ' ';
    //    }

    auto r = 18765;
    printf("Random number: %i\n", r);

    // Bench start
    long long elapsed = 0;
    long long t_start = QDateTime::currentMSecsSinceEpoch();

//    cout << "Searching value..." << endl;
//    auto count1 = 0;
//    auto count2 = 0;
//    for (auto i = 0; i < SIZE; i++) {
//        if (strcmp(db.at(i)->category, "Консервы") == 0) {
//            count1++;
//        } else {
//            count2++;
//        }
//    }

    // Bench end
    long long t_end = QDateTime::currentMSecsSinceEpoch();
    elapsed = t_end - t_start;

    printf("Count Консервы: %i\n", count1);
    printf("Count Бакалея, сахар, соль: %i\n", count2);

    printf("Elapsed time: %i\n", elapsed);

    return 0;
}


