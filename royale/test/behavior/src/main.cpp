#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include <PlatformResources.hpp>

int main (int argc, const char *argv[])
{
    sample_utils::PlatformResources resources;

    return Catch::Session().run (argc, argv);
}