#include "engine_interface.h"
#include "main_game_interface.h"
#include "cpputils/logger.hpp"

int main(int argc, char* argv[])
{
    IEngineInterface::enable_logs(Logger::LOG_LEVEL_TRACE | Logger::LOG_LEVEL_DEBUG | Logger::LOG_LEVEL_INFO);
    IEngineInterface::run<MainGameInterface>();
}
