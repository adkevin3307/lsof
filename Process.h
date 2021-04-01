#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <map>

class Process {
private:
    class File {
    private:
        std::string m_fd;
        std::string m_type;
        std::string m_node;
        std::string m_name;

    public:
        File();
        ~File();

        friend class Process;

        friend bool operator==(const File& f1, const File& f2)
        {
            return ((f1.m_fd == f2.m_fd) && (f1.m_type == f2.m_type) && (f1.m_node == f2.m_node) && (f1.m_name == f2.m_name));
        }

        friend std::ostream& operator<<(std::ostream& os, const File& file)
        {
            os << std::left << std::setw(5) << file.m_fd;
            os << std::left << std::setw(10) << file.m_type;
            os << std::left << std::setw(10) << file.m_node;
            os << file.m_name;

            return os;
        }
    };

    std::filesystem::path m_path;
    std::string m_command;
    std::string m_pid;
    std::string m_user;
    std::vector<File> m_files;

    void parse_basic_info();
    void parse_status();
    void parse_maps();
    void parse_fd();

public:
    Process(const std::filesystem::path& path);
    ~Process();

    bool operator<(const Process& process) const;

    const size_t size() const;
    std::map<std::string, std::string> info(size_t index) const;

    friend std::ostream& operator<<(std::ostream& os, const Process& process)
    {
        for (size_t i = 0; i < process.m_files.size(); i++) {
            os << std::left << std::setw(40) << process.m_command;
            os << std::left << std::setw(10) << process.m_pid;
            os << std::left << std::setw(20) << process.m_user;
            os << process.m_files[i];

            if (i != process.m_files.size() - 1) {
                os << '\n';
            }
        }

        return os;
    }
};
