// Компілятор: GCC (MinGW-w64 / CLion)
#include <algorithm>
#include <cmath>
#include <compare>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class WeightedRandomPicker
{
public:
    WeightedRandomPicker(vector<long long> values,
                          const vector<long long>& frequencies)
        : values_(move(values))
    {
        if (values_.empty())
            throw invalid_argument("The sequence of numbers is empty.");
        if (values_.size() != frequencies.size())
            throw invalid_argument(
                "The number of values and the number of frequencies do not match.");

        vector<long long> sortedCopy = values_;
        ranges::sort(sortedCopy);
        if (ranges::adjacent_find(sortedCopy) != sortedCopy.end())
            throw invalid_argument(
                "The input values must be distinct (no duplicates).");

        for (long long f : frequencies)
        {
            if (f <= 0)
                throw invalid_argument(
                    "Each frequency must be a natural number (> 0).");
        }

        vector<double> weights(frequencies.begin(), frequencies.end());

        distribution_ = discrete_distribution<size_t>(
            weights.begin(), weights.end());

        random_device rd;
        engine_.seed(rd());
    }

    long long operator()()
    {
        size_t index = distribution_(engine_);
        return values_[index];
    }

private:
    vector<long long> values_;
    mt19937 engine_;
    discrete_distribution<size_t> distribution_;
};


class FrequencyStat
{
public:
    FrequencyStat(long long value, long long givenFrequency,
                  long long expected, long long obtained)
        : value_(value), givenFrequency_(givenFrequency),
          expected_(expected), obtained_(obtained)
    {
    }

    [[nodiscard]] long long value() const noexcept { return value_; }
    [[nodiscard]] long long givenFrequency() const noexcept { return givenFrequency_; }
    [[nodiscard]] long long expected() const noexcept { return expected_; }
    [[nodiscard]] long long obtained() const noexcept { return obtained_; }
    [[nodiscard]] long long discrepancy() const noexcept
    {
        return llabs(expected_ - obtained_);
    }

    [[nodiscard]] strong_ordering operator<=>(
        const FrequencyStat& other) const
    {
        return discrepancy() <=> other.discrepancy();
    }
    [[nodiscard]] bool operator==(const FrequencyStat& other) const
    {
        return discrepancy() == other.discrepancy();
    }

private:
    long long value_;
    long long givenFrequency_;
    long long expected_;
    long long obtained_;
};


struct InputData
{
    long long trialCount = 0;
    vector<long long> values;
    vector<long long> frequencies;
};
/**
 * Зчитує вхідні дані з текстового файлу.
 * Формат файлу:
 * 1-й рядок: K (натуральне число) — кількість генерацій (випробувань).
 * 2-й рядок: N (натуральне число) — кількість унікальних чисел у послідовності.
 * 3-й рядок: N цілих чисел, розділених пробілом — значення.
 * 4-й рядок: N натуральних чисел, розділених пробілом — їх задані частоти.
 */
InputData readInput(const string& path)
{
    ifstream in(path, ios::binary);
    if (!in.is_open())
        throw runtime_error("Could not open file: " + path);

    char bom[3] = {};
    in.read(bom, 3);
    const bool hasBom = static_cast<unsigned char>(bom[0]) == 0xEF &&
                         static_cast<unsigned char>(bom[1]) == 0xBB &&
                         static_cast<unsigned char>(bom[2]) == 0xBF;
    in.clear();
    in.seekg(hasBom ? 3 : 0, ios::beg);

    InputData data;
    long long n = 0;

    if (!(in >> data.trialCount))
        throw runtime_error("Could not read the number of trials K.");
    if (data.trialCount <= 0)
        throw runtime_error("K must be a natural number.");

    if (!(in >> n))
        throw runtime_error("Could not read the number of values N.");
    if (n <= 0)
        throw runtime_error("N must be a natural number.");

    data.values.resize(static_cast<size_t>(n));
    data.frequencies.resize(static_cast<size_t>(n));

    for (auto& v : data.values)
        if (!(in >> v))
            throw runtime_error("Not enough numbers in the value sequence.");

    for (auto& f : data.frequencies)
        if (!(in >> f))
            throw runtime_error("Not enough numbers in the frequency sequence.");

    return data;
}

int main(int argc, char* argv[])
{
    try
    {

        const string inputPath = (argc > 1) ? argv[1] : "input.txt";

        InputData data = readInput(inputPath);

        WeightedRandomPicker picker(data.values, data.frequencies);

        const long long totalFrequency = accumulate(
            data.frequencies.begin(), data.frequencies.end(), 0LL,
            plus<long long>());

        unordered_map<long long, long long> obtainedCounts;
        for (long long v : data.values)
            obtainedCounts[v] = 0;

        for (long long i = 0; i < data.trialCount; ++i)
            ++obtainedCounts[picker()];


        auto expectedCount = [&](long long freq) -> long long
        {
            const double share =
                static_cast<double>(freq) / static_cast<double>(totalFrequency);
            return static_cast<long long>(
                llround(share * static_cast<double>(data.trialCount)));
        };

        vector<FrequencyStat> stats;
        stats.reserve(data.values.size());
        for (size_t i = 0; i < data.values.size(); ++i)
        {
            const long long value = data.values[i];
            stats.emplace_back(value, data.frequencies[i],
                                expectedCount(data.frequencies[i]),
                                obtainedCounts[value]);
        }

        ranges::sort(stats, greater<>());

        cout << "K (number of generations) = " << data.trialCount << "\n\n";
        cout << left << setw(10) << "Value" << setw(16)
                  << "Given frequency" << setw(16) << "Expected count"
                  << setw(16) << "Obtained count" << "Discrepancy"
                  << "\n";

        for (const auto& s : stats)
        {
            cout << left << setw(10) << s.value() << setw(16)
                      << s.givenFrequency() << setw(16) << s.expected()
                      << setw(16) << s.obtained() << s.discrepancy() << "\n";
        }


        if (!stats.empty())
        {
            const auto& maxStat = stats.front(); // Беремо перший елемент
            cout << "\nLargest frequency discrepancy: " << maxStat.discrepancy()
                 << " (for value " << maxStat.value() << ")\n";
        }
    }
    catch (const exception& ex)
    {
        cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}