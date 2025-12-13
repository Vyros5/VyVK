
#include <Vy/Engine.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() 
{
    Vy::VyLogger::init();
    
    Vy::VyEngine app{};

    try 
    {
        app.initialize();

        app.run();
    } 
    catch (const std::exception& e) 
    {
        VY_ERROR_TAG("Main", "VyEngine failed to start - Error: {}", e.what());

        return EXIT_FAILURE;
    }

    Vy::VyLogger::shutdown();

    return EXIT_SUCCESS;
}