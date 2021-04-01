#include "Process.h"

#include <algorithm>
#include <fstream>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <regex>

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

vector<string> split(string s)
{
    vector<string> result;

    size_t index = 0;
    s = trim(s);

    while ((index = s.find(' ')) != string::npos) {
        string token = trim(s.substr(0, index));

        result.push_back(token);

        s.erase(0, index + 1);
        s = trim(s);
    }

    result.push_back(s);

    return result;
}

string type(const filesystem::path& path)
{
    if (!filesystem::exists(path)) {
        return "";
    }
    else if (filesystem::is_directory(path)) {
        return "DIR";
    }
    else if (filesystem::is_regular_file(path)) {
        return "REG";
    }
    else if (filesystem::is_character_file(path)) {
        return "CHR";
    }
    else if (filesystem::is_fifo(path)) {
        return "FIFO";
    }
    else if (filesystem::is_socket(path)) {
        return "SOCK";
    }
    else {
        return "unknown";
    }
}

string inode(const filesystem::path& path)
{
    int ret;
    struct stat file_stat;

    ret = stat(path.c_str(), &file_stat);

    if (ret >= 0) {
        return to_string(file_stat.st_ino);
    }

    return "";
}

Process::Process(const filesystem::path& path)
{
    this->m_path = path;

    this->m_command = "";
    this->m_pid = "";
    this->m_user = "";

    this->parse_basic_info();
    this->parse_status();
    this->parse_maps();
    this->parse_fd();
}

Process::~Process()
{
    this->m_files.clear();
    this->m_files.shrink_to_fit();
}

Process::File::File()
{
    this->m_fd = "";
    this->m_type = "";
    this->m_node = "";
    this->m_name = "";
}

Process::File::~File()
{
}

void Process::parse_basic_info()
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

            if (key.length() <= 0 || value.length() <= 0) {
                continue;
            }
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
    }

    file.close();
}

void Process::parse_status()
{
    string parse_targets[] = { "cwd", "root", "exe" };

    for (auto parse_target : parse_targets) {
        File file;
        filesystem::path target = this->m_path / parse_target;

        file.m_fd = parse_target;

        if (access(target.c_str(), R_OK) == 0) {
            file.m_type = type(target);
            file.m_node = inode(target);
            file.m_name = filesystem::read_symlink(target);
        }
        else {
            file.m_type = "unknown";
            file.m_node = "";
            file.m_name = target.string() + " (readlink: Permission denied)";
        }

        this->m_files.push_back(file);
    }
}

void Process::parse_maps()
{
    regex maps_regex(R"(^\[.+\])");
    filesystem::path target = this->m_path / "maps";

    if (access(target.c_str(), R_OK) == 0) {
        fstream file_stream;
        file_stream.open(target, ios::in);

        if (file_stream.is_open()) {
            string s;
            while (getline(file_stream, s)) {
                vector<string> tokens = split(s);

                if (tokens.size() == 6) {
                    if (regex_match(tokens[5], maps_regex)) continue;

                    File file;

                    file.m_fd = ((tokens[5].find("deleted") == string::npos) ? "mem" : "del");
                    file.m_type = type(filesystem::path(tokens[5]));
                    file.m_node = tokens[4];
                    file.m_name = tokens[5];

                    this->m_files.push_back(file);
                }
            }
        }

        file_stream.close();
    }
}

void Process::parse_fd()
{
    filesystem::path target_fd = this->m_path / "fd";
    filesystem::path target_fdinfo = this->m_path / "fdinfo";

    if (access(target_fd.c_str(), R_OK) == 0 && access(target_fdinfo.c_str(), R_OK) == 0) {
        for (auto entry : filesystem::directory_iterator(target_fd)) {
            if (access(entry.path().c_str(), R_OK) == 0) {
                File file;
                string open_mode;

                fstream file_stream;
                file_stream.open(target_fdinfo / entry.path().filename(), ios::in);

                if (file_stream.is_open()) {
                    string s;
                    while (getline(file_stream, s)) {
                        if (s.find("flags") != string::npos) {
                            s = trim(s);

                            if (s.back() == '0') {
                                open_mode = 'r';
                            }
                            if (s.back() == '1') {
                                open_mode = 'w';
                            }
                            if (s.back() == '2') {
                                open_mode = 'u';
                            }
                        }
                    }
                }

                file_stream.close();

                file.m_fd = entry.path().filename().string() + open_mode;
                file.m_type = type(entry.path());
                file.m_node = inode(entry.path());
                file.m_name = filesystem::read_symlink(entry.path());

                this->m_files.push_back(file);
            }
        }
    }
    else {
        File file;

        file.m_fd = "NOFD";
        file.m_type = "";
        file.m_node = "";
        file.m_name = target_fd.string() + " (opendir: Permission denied)";

        this->m_files.push_back(file);
    }
}

bool Process::operator<(const Process& process) const
{
    return atoi(this->m_pid.c_str()) < atoi(process.m_pid.c_str());
}

const size_t Process::size() const
{
    return this->m_files.size();
}

map<string, string> Process::info(size_t index) const
{
    map<string, string> result;

    result["COMMAND"] = this->m_command;
    result["PID"] = this->m_pid;
    result["USER"] = this->m_user;
    result["FD"] = this->m_files[index].m_fd;
    result["TYPE"] = this->m_files[index].m_type;
    result["NODE"] = this->m_files[index].m_node;
    result["NAME"] = this->m_files[index].m_name;

    return result;
}
