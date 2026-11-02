#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <random>

#include "data.h"

using namespace std;
using namespace std::chrono;

const int READ = 0;
const int WRITE = 1;
const int STRING = 2;

struct Command {
    int type;
    int field;
    int value;
};

void generateFile(const string& filename, int count, int seed, const int weights[7]) {
    ofstream out(filename);
    mt19937 rng(seed);

    int total = 0;
    for (int i = 0; i < 7; i++) total += weights[i];

    for (int i = 0; i < count; i++) {
        int r = rng() % total;
        int k = 0;
        while (r >= weights[k]) {
            r -= weights[k];
            k++;
        }

        if (k == 0) out << "read 0\n";
        else if (k == 1) out << "write 0 1\n";
        else if (k == 2) out << "read 1\n";
        else if (k == 3) out << "write 1 1\n";
        else if (k == 4) out << "read 2\n";
        else if (k == 5) out << "write 2 1\n";
        else out << "string\n";
    }
}

vector<Command> loadFile(const string& filename) {
    vector<Command> result;
    ifstream in(filename);
    if (!in) {
        cout << "Не вдалося відкрити файл " << filename << endl;
        return result;
    }

    string word;
    while (in >> word) {
        Command c;
        c.field = 0;
        c.value = 0;
        if (word == "read") {
            c.type = READ;
            in >> c.field;
        }
        else if (word == "write") {
            c.type = WRITE;
            in >> c.field >> c.value;
        }
        else {
            c.type = STRING;
        }
        result.push_back(c);
    }
    return result;
}

double runOnce(const vector<vector<Command>>& cmds, vector<double>& times) {
    Data data;
    int n = cmds.size();
    times.assign(n, 0.0);
    vector<thread> threads;

    auto start = steady_clock::now();

    for (int t = 0; t < n; t++) {
        threads.push_back(thread([&data, &cmds, &times, t]() {
            auto s = steady_clock::now();
            for (const Command& c : cmds[t]) {
                if (c.type == READ) {
                    data.read(c.field);
                }
                else if (c.type == WRITE) {
                    data.write(c.field, c.value);
                }
                else {
                    string str = static_cast<string>(data);
                }
            }
            auto e = steady_clock::now();
            times[t] = duration<double, milli>(e - s).count();
            }));
    }

    for (int t = 0; t < n; t++) {
        threads[t].join();
    }

    auto end = steady_clock::now();
    return duration<double, milli>(end - start).count();
}

void testSet(const string& name, const string& prefix, int totalOps,
    const int weights[7], double result[3]) {
    cout << "===== " << name << " =====" << endl;

    for (int n = 1; n <= 3; n++) {
        int opsPerThread = totalOps / n;
        vector<vector<Command>> cmds(n);
        vector<string> files(n);

        for (int t = 0; t < n; t++) {
            files[t] = prefix + "_" + to_string(n) + "threads_file" + to_string(t + 1) + ".txt";
            generateFile(files[t], opsPerThread, 100 + t, weights);
            cmds[t] = loadFile(files[t]);
        }

        cout << "--- " << n << " thread(s) ---" << endl;

        vector<double> times;
        runOnce(cmds, times);

        double sum = 0;
        for (int run = 1; run <= 5; run++) {
            double total = runOnce(cmds, times);
            sum += total;
            cout << "Run " << run << ": " << total << " ms" << endl;
            for (int t = 0; t < n; t++) {
                cout << "    " << files[t] << ": " << times[t] << " ms" << endl;
            }
        }
        result[n - 1] = sum / 5;
        cout << "Average: " << result[n - 1] << " ms" << endl << endl;
    }
}

int main() {
    const int TOTAL_OPS = 3000000;

    int weightsA[7] = { 20, 5, 20, 5, 10, 20, 20 };
    int weightsB[7] = { 1, 1, 1, 1, 1, 1, 1 };
    int weightsC[7] = { 2, 35, 2, 35, 20, 5, 1 };

    double a[3], b[3], c[3];
    testSet("a) variant 12", "test_a", TOTAL_OPS, weightsA, a);
    testSet("b) equal", "test_b", TOTAL_OPS, weightsB, b);
    testSet("c) opposite", "test_c", TOTAL_OPS, weightsC, c);

    cout << fixed << setprecision(2);
    cout << "===== Results (ms) =====" << endl;
    cout << left << setw(15) << "" << setw(12) << "1 thread" << setw(12) << "2 threads" << setw(12) << "3 threads" << endl;
    cout << setw(15) << "a)" << setw(12) << a[0] << setw(12) << a[1] << setw(12) << a[2] << endl;
    cout << setw(15) << "b)" << setw(12) << b[0] << setw(12) << b[1] << setw(12) << b[2] << endl;
    cout << setw(15) << "c)" << setw(12) << c[0] << setw(12) << c[1] << setw(12) << c[2] << endl;

    return 0;
}