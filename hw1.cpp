#include <string>
#include <map>
#include <unistd.h>

#include "ProcessManagement.h"

using namespace std;

map<string, string> argparse(int argc, char** argv)
{
    int opt = 0;
    map<string, string> args;

    while (true) {
        opt = getopt(argc, argv, "c:t:f:");

        if (opt == -1) break;

        switch (opt) {
            case 'c':
                args["c"] = optarg;

                break;
            case 't':
                args["t"] = optarg;

                break;
            case 'f':
                args["f"] = optarg;

                break;
            default:
                break;
        }
    }

    return args;
}

int main(int argc, char** argv)
{
    map<string, string> args = argparse(argc, argv);

    ProcessManagement process_management(args);
    process_management.run();

    return 0;
}
