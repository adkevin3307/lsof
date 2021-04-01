#pragma once

#include <map>
#include <vector>
#include <string>

#include "Process.h"

class ProcessManagement {
private:
    std::map<std::string, std::string> m_args;
    std::vector<Process> m_processes;

public:
    ProcessManagement(const std::map<std::string, std::string>& args);
    ~ProcessManagement();

    void run();
};
