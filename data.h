#pragma once

#include <string>
#include <mutex>
#include <shared_mutex>

using namespace std;

class Data {
private:
    int fields[3] = { 0, 0, 0 };
    mutable shared_mutex m0;
    mutable shared_mutex m1;
    mutable mutex m2;

public:
    int read(int index) const {
        if (index == 0) {
            shared_lock<shared_mutex> lock(m0);
            return fields[0];
        }
        else if (index == 1) {
            shared_lock<shared_mutex> lock(m1);
            return fields[1];
        }
        else {
            lock_guard<mutex> lock(m2);
            return fields[2];
        }
    }

    void write(int index, int value) {
        if (index == 0) {
            unique_lock<shared_mutex> lock(m0);
            fields[0] = value;
        }
        else if (index == 1) {
            unique_lock<shared_mutex> lock(m1);
            fields[1] = value;
        }
        else {
            lock_guard<mutex> lock(m2);
            fields[2] = value;
        }
    }

    explicit operator string() const {
        int a, b, c;
        {
            shared_lock<shared_mutex> lock0(m0);
            shared_lock<shared_mutex> lock1(m1);
            lock_guard<mutex> lock2(m2);
            a = fields[0];
            b = fields[1];
            c = fields[2];
        }
        return to_string(a) + " " + to_string(b) + " " + to_string(c);
    }
}; 
