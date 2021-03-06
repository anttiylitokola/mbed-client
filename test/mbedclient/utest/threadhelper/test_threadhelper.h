/*
 * Copyright (c) 2015 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef TEST_THREAD_HELPER_H
#define TEST_THREAD_HELPER_H

#include "mbed-client-linux/threadhelper.h"

class Test_ThreadHelper : public ThreadHelper
{
public:
    Test_ThreadHelper();

    virtual ~Test_ThreadHelper();

    void test_start();

    void test_join();

    void test_cancel();

    void run();

};

#endif // TEST_THREAD_HELPER_H
