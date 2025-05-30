#include "../include/Utils.h"

#include <iostream>
#include <random>
#include <atomic>

extern std::atomic<bool> g_Running;

std::string RandomString(const int length)
{
    static constexpr char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<int> distribution(0, sizeof(charset) - 2);

    std::string result;
    result.reserve(length);

    for (int i = 0; i < length; ++i)
    {
        result += charset[distribution(generator)];
    }

    return result;
}

void SignalHandler(int signal)
{
    g_Running.store(false);
}