// Copyright (C) 2025 Julien Michot. All Rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>
#include <utility>

#if CATCH2_VERSION == 2
#include <catch2/catch.hpp>
#else
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#endif

#include <tinyopt/diff/auto_diff.h>
#include <tinyopt/diff/num_diff.h>
#include <tinyopt/log.h>
#include <tinyopt/losses/classif.h>

using Catch::Approx;
using namespace tinyopt;
using namespace tinyopt::loss::classif;

void TestLosses() {
  SECTION("SoftMax") {
    Vec4 x = Vec4::Random();
    const auto &[s, Js] = Softmax(x, true);
    TINYOPT_LOG("loss = [{}, \nJ:{}]", s, Js);
    auto J = diff::CalculateJac(x, [](const auto x) { return Softmax(x); });
    TINYOPT_LOG("Jad:{}", J);
    REQUIRE((J - Js).cwiseAbs().maxCoeff() == Approx(0.0).margin(1e-5));
  }
  SECTION("SafeSoftMax") {
    Vec4 x = Vec4::Random();
    const auto &[s, Js] = SafeSoftmax(x, true);
    TINYOPT_LOG("loss = [{}, \nJ:{}]", s, Js);
    auto J = diff::CalculateJac(x, [](const auto x) { return SafeSoftmax(x); });
    TINYOPT_LOG("Jad:{}", J);
    REQUIRE((J - Js).cwiseAbs().maxCoeff() == Approx(0.0).margin(1e-5));
  }
}

TEST_CASE("tinyopt_losses_classif") { TestLosses(); }