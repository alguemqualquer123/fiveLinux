#include "command_interface.h"
#include "fivemlinux/types.h"

int main(int argc, char* argv[]) {
    fml::CommandInterface cli;
    return cli.run(argc, argv);
}
