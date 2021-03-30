#include <iostream>
#include <string>
#include <regex>
#include <filesystem>

#include "Process.h"

using namespace std;

int main()
{
    string base = "/proc";
    regex path_regex(R"(/proc/[0-9]+)");

    
    cout << std::left << std::setw(40) << "COMMAND";
    cout << std::left << std::setw(10) << "PID";
    cout << std::left << std::setw(20) << "USER";
    cout << std::left << std::setw(5) << "FD";
    cout << std::left << std::setw(10) << "TYPE";
    cout << std::left << std::setw(10) << "NODE";
    cout << "NAME" << '\n';

    for (auto entry : filesystem::directory_iterator(base)) {
        if (regex_match(entry.path().c_str(), path_regex)) {
            Process process(entry.path());

            cout << process << '\n';
        }
    }

    return 0;
}
