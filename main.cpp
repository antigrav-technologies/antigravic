#include "Processor.hpp"

int main(int argc, char *argv[]) {
    srand(time(0));
    Processor(std::make_shared<C64Terminal>(C64Terminal())).mainloop();
    return 0;
}
