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

#include <cmath>
#include "tinyopt/math.h"

#if CATCH2_VERSION == 2
#include <catch2/catch.hpp>
#else
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#endif

#include <tinyopt/diff/gradient_check.h>

using namespace tinyopt;

TEST_CASE("tinyopt_check_gradient") {
  auto residuals = [](const auto &x, auto &grad, auto &H) {
    const Mat2 J = Vec2(3.0, 2.0).asDiagonal();
    Vec2 res = (J * x).array() - 2.0;
    // Manually update the H and gradient
    if constexpr (!traits::is_nullptr_v<decltype(grad)>) {
      grad = J.transpose() * res;
      H = J.transpose() * J;
    }
    return res;
  };

  Vec2 x(1.4, 7.2);
  REQUIRE(diff::CheckResidualsGradient(x, residuals));
}

TEST_CASE("tinyopt_check_gradient_sparse_H") {
  auto residuals = [](const auto &x, auto &grad, SparseMatrix<double> &H) {
    const Mat2 J = Vec2(3.0, 2.0).asDiagonal();
    Vec2 res = (J * x).array() - 2.0;
    // Manually update the H and gradient
    if constexpr (!traits::is_nullptr_v<decltype(grad)>) {
      grad = J.transpose() * res;
      H = (J.transpose() * J).sparseView();
    }
    return res;
  };

  Vec2 x(1.4, 7.2);
  REQUIRE(diff::CheckResidualsGradient(x, residuals));
}