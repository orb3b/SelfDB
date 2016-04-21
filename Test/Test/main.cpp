#include "Server.h"

using namespace std;

//const char* FILE_NAME = "data50m.bin";
//const int SIZE = 50000000;
//const int BUFFER_SIZE = 10000000;

//char* categories[] = {
//    "Sugar",
//    "Salt",
//    "Milk",
//    "Meat",
//    "Chicken",
//    "Vegetables",
//    "Beer",
//    "Oils",
//    "Fish",
//    "Juice",
//    "Sigarrets",
//    "Bread"
//};
//int categoriesSize = 12;

//struct Row {
//    int id;
//    char category[50];
//};

//class Database {
//    Row *data;
//    int blocksCount;

//public:
//    Row* at(int index) {
//        //auto block = index / BUFFER_SIZE;
//        //auto offset = index % BUFFER_SIZE;

//        return &(data[index]);
//    }

//    void sort()
//    {
//        qsort(data, SIZE, sizeof(Row), Database::compare);
//    }

//    bool load() {
//        auto fp = fopen(FILE_NAME, "rb");
//        if (!fp) {
//            printf("Cannot open %s for reading!", FILE_NAME);
//            return false;
//        }

//        fseek(fp, 0, SEEK_SET);

//        blocksCount = SIZE / BUFFER_SIZE;
//        if (SIZE % BUFFER_SIZE != 0) {
//            blocksCount++;
//        }

//        data = new Row[SIZE];
//        for (auto i = 0; i < blocksCount; i++) {
//            fread(data + i * BUFFER_SIZE, sizeof(Row), BUFFER_SIZE, fp);
//        }

//        fclose(fp);

//        return true;
//    }
//private:
//    static int compare(const void* p1, const void* p2)
//    {
//        auto row1 = (Row*)p1;
//        auto row2 = (Row*)p2;

//        return row1->id - row2->id;
//    }
//};

//bool generate(int size)
//{
//    auto fp = fopen(FILE_NAME, "wb");
//    if (!fp) {
//        printf("Cannot open %s for writing!", FILE_NAME);
//        return false;
//    }

//    auto blocksCount = SIZE / BUFFER_SIZE;
//    if (SIZE % BUFFER_SIZE != 0) {
//        blocksCount++;
//    }

//    Row *array = new Row[BUFFER_SIZE];
//    for (auto i = 0; i < blocksCount; i++) {
//        for (auto i = 0; i < BUFFER_SIZE; i++) {
//            array[i].id = rand();
//            strcpy(array[i].category, categories[rand() % categoriesSize]);
//        }
//        fwrite(array, sizeof(Row), BUFFER_SIZE, fp);
//    }
//    delete[] array;

//    fclose(fp);

//    return true;
//}

//void writeLine(char *line)
//{
//    cout << line << endl;
//}

//void writeLine(time_t val)
//{
//    cout << val << endl;
//}

int main(int argc, char *argv[])
{
    Server server;
    server.run();

    return 0;
}
//    // Run
////    if (!generate(SIZE))
////        return 1;
////    else
////        return 0;

//    Database db;

//    cout << "Loading file..." << endl;
//    if (!db.load())
//        return 2;

//    //    // set some initial values:
//    //    int r = rand() * rand();
//    //    for (int i = 1; i <= 1000000; i++) {
//    //        myset.insert(val);
//    //    }

//    //    for (it = myset.begin(); it != myset.end(); it++) {
//    //        cout << *it << ' ';
//    //    }

//    auto r = 18765;
//    printf("Random number: %i\n", r);

//    // Bench start
//    long long elapsed = 0;
//    long long t_start = QDateTime::currentMSecsSinceEpoch();

//    // Group by
//    cout << "Group by..." << endl;

//    auto counts = new int[categoriesSize];
//    memset(counts, 0, categoriesSize * sizeof(int));

//    for (auto i = 0; i < SIZE; i++) {
//        auto category = db.at(i)->category;
//        for (auto j = 0; j < categoriesSize; j++) {
//            if (strcmp(category, categories[j]) == 0) {
//                counts[j]++;
//                break;
//            }
//        }
//    }

//    for (auto i = 0; i < categoriesSize; i++) {
//        printf("%s: %i\n", categories[i], counts[i]);
//    }

////    // Sort
////    for (auto i = 1000000; i < 1000100; i++) {
////        auto row = db.at(i);
////        printf("%i %s\n", row->id, row->category);
////    }

////    cout << "Sorting..." << endl;

////    db.sort();

////    for (auto i = 1000000; i < 1000100; i++) {
////        auto row = db.at(i);
////        printf("%i %s\n", row->id, row->category);
////    }

//    // Bench end
//    long long t_end = QDateTime::currentMSecsSinceEpoch();
//    elapsed = t_end - t_start;

//    printf("Elapsed time: %i\n", elapsed);


