// Copyright 2016 The Android Open Source Project
//
// This software is licensed under the terms of the GNU General Public
// License version 2, as published by the Free Software Foundation, and
// may be copied, distributed, and modified under those terms.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#include "android/base/TypeUtils.h"

#include <gtest/gtest.h>

#include <functional>
#include <list>
#include <vector>

namespace android {
namespace base {

TEST(TypeUtils, IsCallable) {
    class C;
    C* c = nullptr;

    auto lambda = [c](bool) -> C* { return nullptr; };

    static_assert(is_callable<void(), void()>::value, "simple function");
    static_assert(is_callable<void(&)(), void()>::value, "function reference");
    static_assert(is_callable<void(*)(), void()>::value, "function pointer");
    static_assert(is_callable<int(C&, C*), int(C&, C*)>::value,
                  "function with arguments and return type");
    static_assert(is_callable<decltype(lambda), C*(bool)>::value, "lambda");
    static_assert(is_callable<std::function<bool(int)>, bool(int)>::value,
                  "std::function");

    static_assert(!is_callable<int, void()>::value, "int should not be callable");
    static_assert(!is_callable<C, void()>::value, "incomplete type");
    static_assert(!is_callable<void(), void(int)>::value, "different arguments");
    static_assert(!is_callable<int(), void()>::value, "different return types");
    static_assert(!is_callable<int(), short()>::value,
                  "slightly different return types");
    static_assert(!is_callable<int(int), int(int, int)>::value,
                  "more arguments");
    static_assert(!is_callable<int(int, int), int(int)>::value,
                  "less arguments");

    static_assert(!is_callable<int(int), int>::value,
                  "bad required signature");
}

TEST(TypeUtils, IsTemplateInstantiation) {
    static_assert(!is_template_instantiation<int, std::vector>::value,
                  "int is not an instance of vector");
    static_assert(!is_template_instantiation<std::list<std::vector<int>>, std::vector>::value,
                  "list is not an instance of vector");

    static_assert(is_template_instantiation<std::vector<int>, std::vector>::value,
                  "std::vector<int> is an instance of vector");
    static_assert(is_template_instantiation<std::vector<std::vector<std::vector<int>>>, std::vector>::value,
                  "nested std::vector<> is an instance of vector");
}

}  // namespace base
}  // namespace android
