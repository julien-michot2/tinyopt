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

#pragma once

#include <sstream>
#include <string>
#include <type_traits>

#include <Eigen/src/Core/ArrayBase.h>
#include <Eigen/src/Core/MatrixBase.h>

namespace tinyopt::traits {

// Trait to check if a type is an Eigen matrix
template <typename T>
struct is_eigen_matrix_or_array
    : std::disjunction<std::is_base_of<Eigen::MatrixBase<T>, T>,
                       std::is_base_of<Eigen::ArrayBase<T>, T>> {};

template <typename T>
constexpr int is_eigen_matrix_or_array_v = is_eigen_matrix_or_array<T>::value;

// Default parameters trait

template <typename T, typename = void> struct params_trait {
  using Scalar = typename T::Scalar; // The scalar type
  static constexpr int Dims = T::Dims; // Compile-time parameters dimensions

  // Execution-time parameters dimensions
  static int dims(const T &v) { return Dims == Eigen::Dynamic ? v.dims() : Dims;}

  // Conversion to string
  static std::string toString(const T& v) {
    std::stringstream ss;
    ss << v;
    return ss.str();
  }

  // Cast to a new type, only needed when using automatic differentiation
  template <typename T2>
  static auto cast(const T &v) {
    return v.template cast<T2>();
  }

  // Define update / manifold
  static void pluseq(T& v, const Eigen::Vector<Scalar, Dims>& delta) {
    v += delta;
  }
};

// Trait specialization for scalar (float, double)
template <typename T>
struct params_trait<T, std::enable_if_t<std::is_scalar_v<T>>> {
  using Scalar = T; // The scalar type
  static constexpr int Dims = 1; // Compile-time parameters dimensions
  // Execution-time parameters dimensions
  static constexpr int dims(const T &) { return 1;}
  // Conversion to string
  static std::string toString(const T& v) {
    return std::to_string(v);
  }
  // Cast to a new type, only needed when using automatic differentiation
  template <typename T2>
  static T2 cast(const T &v) {
    return T2(v);
  }
  // Define update / manifold
  static void pluseq(T& v, const Eigen::Vector<Scalar, Dims>& delta) {
    v += delta[0];
  }
  static void pluseq(T& v, const Scalar& delta) {
    v += delta;
  }
};

// Trait specialization for Eigen::MatrixBase
template <typename T>
struct params_trait<
    T, std::enable_if_t<std::is_base_of_v<Eigen::MatrixBase<T>, T>>> {
  using Scalar = typename T::Scalar; // The scalar type
  static constexpr int Dims = T::RowsAtCompileTime; // Compile-time parameters dimensions
  // Execution-time parameters dimensions
  static int dims(const T &m) { return m.size();}
  // Conversion to string
  static std::string toString(const T& m) {
    std::stringstream ss;
    if (m.cols() == 1)
      ss << m.transpose();
    else
      ss << m;
    return ss.str();
  }
  // Cast to a new type, only needed when using automatic differentiation
  template <typename T2>
  static auto cast(const T &v) {
    return v.template cast<T2>();
  }
  // Define update / manifold
  static void pluseq(T& v, const Eigen::Vector<Scalar, Dims>& delta) {
    v += delta;
  }
};

} // namespace tinyopt::traits
