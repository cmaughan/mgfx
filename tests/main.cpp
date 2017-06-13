#include <gtest/gtest.h>
#include "m3rdparty.h"

INITIALIZE_EASYLOGGINGPP

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

