#include "Process.h"

#include <algorithm>
#include <fstream>
#include <sys/types.h>
#include <pwd.h>

using namespace std;

string trim(string s)
{
    if (s.empty()) return s;

    auto first_not_space = find_if_not(s.begin(), s.end(), [](int c) {
        return isspace(c);
    });

    s.erase(s.begin(), first_not_space);

    auto last_not_space = find_if_not(s.rbegin(), s.rend(), [](int c) {
        return isspace(c);
    });

    s.erase(last_not_space.base(), s.end());

    return s;
}

Process::Process(filesystem::path path)
{
    this->m_path = path;

    this->m_command = "";
    this->m_pid = "";
    this->m_user = "";

    this->parse_status();
    this->parse_open_files();
}

Process::~Process()
{
    this->m_files.clear();
    this->m_files.shrink_to_fit();
}

Process::File::File()
{
    this->m_fd = "-";
    this->m_type = "-";
    this->m_node = "-";
    this->m_name = "-";
}

Process::File::~File()
{
}

void Process::parse_status()
{
    filesystem::path target = this->m_path / "status";
    if (!filesystem::exists(target)) throw runtime_error(target.string() + " does not exists.");

    fstream file;
    file.open(target, ios::in);
    if (file.is_open()) {
        string s;
        while (getline(file, s)) {
            string key, value;

            auto it = s.find_first_of(':');
            key = trim(s.substr(0, it));
            value = trim(s.substr(it + 1));

            if (key.length() <= 0 || value.length() <= 0) continue;
            else if (key == "Name") {
                this->m_command = value;
            }
            else if (key == "Pid") {
                this->m_pid = value;
            }
            else if (key == "Uid") {
                stringstream ss;
                ss << value;

                string euid, ruid, suid, fsuid;
                ss >> euid >> ruid >> suid >> fsuid;
                
                uid_t uid = atoi(ruid.c_str());

                struct passwd* pws = NULL;
                pws = getpwuid(uid);

                if (pws == NULL) {
                    this->m_user = "";
                }
                else {
                    this->m_user = pws->pw_name;
                }
            }
        }

        file.close();
    }
}

void Process::parse_open_files()
{
    // target = path / "maps";
    // if (!filesystem::exists(target)) return;

    // file.open(target);

    // if (file.is_open()) {
    //     string s;
    //     while (getline(file, s)) {
    //         cout << s << '\n';
    //     }

    //     file.close();
    // }
}
