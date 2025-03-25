#include <iostream>
#include <thread>
#include <string>
#include <map>
#include <chrono>
using namespace std::chrono_literals;

void RefreshForecast(std::map<std::string, int> forecastMap)
{
    while (true)
    {
        for (auto item : forecastMap)
        {
            forecastMap[item.first]++;
            std::cout << item.first << " - " << item.second << std::endl;
        }

        std::this_thread::sleep_for(2000ms);
    }
}

int main()
{
    std::map<std::string, int> forecastMap = {
        {"City A", 15},
        {"City B", 25},
        {"City C", 10},
        {"City D", 30},
        {"City E", 18},
    };

    // std::thread bgWorker(RefreshForecast, forecastMap);

    unsigned int c = std::thread::hardware_concurrency();
    std::cout << c << std::endl;

    system("pause>nul");
}