#include "ProcessManagement.h"

#include <regex>

using namespace std;

ProcessManagement::ProcessManagement(const map<string, string>& args)
{
    this->m_args = args;
}

ProcessManagement::~ProcessManagement()
{
    this->m_args.clear();

    this->m_processes.clear();
    this->m_processes.shrink_to_fit();
}

void ProcessManagement::run()
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

            this->m_processes.push_back(process);
        }
    }

    sort(this->m_processes.begin(), this->m_processes.end());

    for (auto process : this->m_processes) {
        cout << process << '\n';
    }
}
