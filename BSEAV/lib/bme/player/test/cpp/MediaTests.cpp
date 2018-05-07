/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include <gtest/gtest.h>
#include "Debug.h"

int main(int argc, char **argv)
{
    Broadcom::debugInitialize();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
