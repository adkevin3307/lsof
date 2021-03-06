#include "ProcessManagement.h"

#include <regex>
#include <iomanip>

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

    cout << left << setw(40) << "COMMAND";
    cout << left << setw(10) << "PID";
    cout << left << setw(20) << "USER";
    cout << left << setw(5) << "FD";
    cout << left << setw(10) << "TYPE";
    cout << left << setw(10) << "NODE";
    cout << "NAME" << '\n';

    for (auto entry : filesystem::directory_iterator(base)) {
        if (regex_match(entry.path().c_str(), path_regex)) {
            try {
                Process process(entry.path());

                this->m_processes.push_back(process);
            }
            catch (const runtime_error& e) {
                cerr << e.what() << '\n';
            }
        }
    }

    sort(this->m_processes.begin(), this->m_processes.end());

    for (auto process : this->m_processes) {
        for (size_t i = 0; i < process.size(); i++) {
            map<string, string> info = process.info(i);

            if (this->m_args.find("c") != this->m_args.end()) {
                smatch string_match;
                regex command_regex(this->m_args["c"]);

                if (!regex_search(info["COMMAND"], string_match, command_regex)) continue;
            }

            if (this->m_args.find("t") != this->m_args.end()) {
                vector<string> valids{ "REG", "CHR", "DIR", "FIFO", "SOCK", "unknown" };

                if (find(valids.begin(), valids.end(), this->m_args["t"]) == valids.end()) {
                    cerr << "Invalid TYPE option." << '\n';

                    return;
                }

                if (info["TYPE"] != this->m_args["t"]) continue;
            }

            if (this->m_args.find("f") != this->m_args.end()) {
                smatch string_match;
                regex name_regex(this->m_args["f"]);

                if (!regex_search(info["NAME"], string_match, name_regex)) continue;
            }

            cout << left << setw(40) << info["COMMAND"];
            cout << left << setw(10) << info["PID"];
            cout << left << setw(20) << info["USER"];
            cout << left << setw(5) << info["FD"];
            cout << left << setw(10) << info["TYPE"];
            cout << left << setw(10) << info["NODE"];
            cout << info["NAME"] << '\n';
        }
    }
}
