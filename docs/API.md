
# API Documentation

## Basic Concepts

In Tinyopt, you define two things
* The parameters `x`
* The Cost, Residuals or Accumulation functions

### Parameters

As of today, you can only pass on *one* parameter `x` to the Optimizer. There is a trick that allows
you to pack all parameters together. See [here](how-to-avoid-data-copies).
They can be float, double or a struct/class of your preference as long as a trait is defined or
the struct/class has expected members, see the [Supported Types](supported-types).

### Cost, Residuals and Accumulation functions

Depending on what you're trying to minimize, you have to provide different types of functions.

**Cost functions** (for general, unconstrained optimizations)

This is the most general function type, it takes *one* argument, the parameters and
return a Scalar value: the cost that will be minimized.
Automatic differentiation will be called in order to compute the jacobians/gradients.

Example: `double(const T &x);` with `T` being one of the [supported types](supported-types).

**Residuals functions**  (for NLLS optimizations)

A Residuals function only takes a single input argument (the parameters) and must returns a
Vector/Matrix or single scalar: the residuals. They are used for NLLS problems only.
Automatic differentiation will be called in order to compute the jacobians/gradients.

Example: `VecX(const T &x);`.

**Accumulation Function**

The above two cost and residuals functions are converted internally to an Accumulation function with
the automatic differentiation. But this step takes time and memory so `tinyopt` gives you
a way to directly accumulate the residuals and update the gradient (and Hessian), this is what
the Accumulation functions are.

An Accumulation function takes 2 to 3 input arguments:
* the parameters `x`,
* the gradient `g`,
* the Hessian `H`, optionally if you want to use (pseudo-)second order solvers.
It must returns a single scalar, the total cost, or a `Cost` struct in which you can store extra
info such as number of residuals, inliers ration, etc.

You are expected to accumulate all your residuals and update the gradient (and Hessian) in that
Accumulation function, manually, by yourself, like a grown up!

This is the **FASTEST** way to optimize something so if execution time matters, use this one!

Example: `Cost(const T &x, Vec3 &gradient, Mat3 &hessian);`.
The `Gradient` type must be a Eigen::Vector, The Hessian Type must be either a dense or sparse Eigen Matrix.


## Full Documentation

Dive into the glorious depths of our documentation on [ReadTheDocs](https://tinyopt.readthedocs.io/en/latest).
It's packed with all the juicy details, and maybe a few hidden jokes if you look hard enough.

## Simple API

`tinyopt` is inspired by the simple syntax of python so it is very developer friendly*, just call `Optimize` and give it something to optimize, say `x` and something to minimize.

`Optimize` performs automatic differentiation so you just have to specify the residual(s),
no jacodians/derivatives to calculate because you know the pain, right? No pain, vanished, thank you Julien.


\* but not compiler friendly, sorry gcc/clang but you'll have to work double because it's all templated.

### Example: What's the square root of 2?
Beause using `std::sqrt` is over hyped, let's try to recover it using `tinyopt`, here is how to do:

```cpp
// Define 'x', the parameter to optimize, initialized to '1' (yeah, who doesn't like 1?)
double x = 1;
Optimize(x, [](const auto &x) { return x * x - 2.0; }); // Let's minimize ε = x*x - 2
// 'x' is now √2, amazing.
```
That's it. Is it too verbose? Well remove the comments then. Come on, it's just two lines, I can't do better.

### Example: Fitting a circle to a set of points
In this use case, you're given `n` 2D points your job is to fit a circle to them.
Today is your lucky day, `tinyopt` is here to help! Let's see how.

```cpp
Mat2Xf obs(2, n); // fill the observations (n 2D points)

// loss is the sum of || ||p - center||² - radius² ||
auto loss = [&obs]<typename Derived>(const MatrixBase<Derived> &x) {
  using T = typename Derived::Scalar;
  const auto &center = x.template head<2>(); // the first two elements are the cicle position
  const auto radius2 = x.z() * x.z(); // the last one is its radius, taking the square to avoid a sqrt later on
  // Here we compute the squared distances of each point to the center
  auto residuals = (obs.cast<T>().colwise() - center).colwise().squaredNorm();
  // Here we compute the difference of of squared distances are the circle's squared radius
  return (residuals.array() - radius2).eval();
  // Make sure the returned type is a scalar or Eigen::Matrix (thus the .eval())
};


// Define 'x', the parameter to optimize, using the following parametrization: x = {center (x, y), radius}
Vec3 x(0, 0, 1);
Optimize(x, loss); // Optimize!
// 'x' should have converged to a reasonable cicle
std::cout << "Residuals: " << loss(x) << "\n"; // Let's print the final residuals
```

Ok so this is quite more verbose than in the first example but I'm trying to help you understand the syntax, it's easy no?

## Supported Types

With `tinyopt`, you can directly optimize several types of parameters `x`, namely

* `float` or `double` scalar types
* `std:array` of scalars or another type
* `std::vector` of a scalars or another type
* `Eigen::Vector` of fixed or dynamic size
* `Eigen::Matrix` of fixed or dynamic size
* Your custom class or struct, see [User defined parameters](#user-defined-parameters)

You can also use any levels of nesting for known types, e.g. `std::array<Vec2f, 2>` or  `std::pair<std::vector<Vec3>, VecX>`.

`tinyopt` will detect whether the size is known at compile time and use this information to make the optimization faster.

Residuals of the following types can also be returned

* `float` or `double` (or the typename `T`, typically a `Jet<S,N>`)
* `Eigen::Vector` or `Eigen::Matrix`

## Advanced API

### Direct Accumulation, a faster - but manual - way.

When working with more whan one residuals, `tinyopt` allows you to avoid storing a full vector of residuals.
You can directly accumulate the residuals and manually update the **full** (real or approx.) Hessian and gradient matrices this way:

```cpp

// Define 'x', the parameter to optimize, initialized to '1'
double x = 1;

// Define the residuals / loss function, here ε² = ||x*x - 2||²
auto loss = [](const auto &x, auto &grad, auto &H) {
  float res = x * x - 2; // since we want x to be sqrt(2), x*x should be 2
  float J   = 2 * x; // residual's jacobian/derivative w.r.t x
  // Manually update the Hessian H ~= Jt * J and Gradient = Jt * residuals
  if constexpr (!traits::is_nullptr_v<decltype(grad)>) { // Gradient requested?
    H(0, 0) = J * J;   // normal matrix (initialized to 0s before so only update what is needed)
    grad(0) = J * res; // gradient (half of it actually)
  }
  // Return both the norm and the number of residuals (here, we have only one)
  return Cost(std::sqrt(res*res), 1);
};

// Setup optimizer options (optional)
Options options;
// Optimize!
const auto &out = Optimize(x, loss, options);
// 'x' is now std::sqrt(2.0), you can check the convergence with out.Converged()
```

For second order solvers, `H` and `grad` are the only things you need to update for LM to solve the normal equations and optimize `x`. It looks a bit rustic I know but we can't all live in a fancy city with sleek buidlings,
sometimes it's good to go back to your (square) roots, so take your boots and start coding.

*NOTE* that you only need to fill the upper triangle part only since `H` is assumed to be symmetric.

### Checking Gradients

Manually filling the gradient and eventually the Hessian (approximation as Jt*J) can be a shaky process because you know, your math might not be as sharp as you would like! So tinyopt provides a function that checks them for you, how nice!
It does not check your second derivatives, sorry, only if H is `Jt*J`.

The relevant functions are `CheckGradient` for general optimizations and `CheckResidualsGradient` for non-linear least squares.
You can do this type of checks for NLLS:
```cpp
// First, define your residuals (for NLLS)
auto residuals = [&](const auto &x, auto &grad, auto &H) {
  float res = x * x - 2;  // since we want x to be sqrt(2), x*x should be 2
  float J = 2 * x;        // residual's jacobian/derivative w.r.t x
  // Manually update the hessian and gradient
  if constexpr (!traits::is_nullptr_v<decltype(grad)>) {
    grad(0) = J * res;
    H(0, 0) = J * J;
  }
  return res; // Returns the residual
};
// Let's check the gradient and Hessian approx.
float x = 1;
REQUIRE(diff::CheckResidualsGradient(x, residuals)); // well, let's check my math!

// Ok, let's optimize then!
auto loss = [&](const auto &x, auto &grad, auto &H) {
  const auto r = residuals(x, grad, H);
  return std::abs(r); // or r²
};
Optimize(x, loss);
```
Note that
* we use numerical differentiation so that you don't need all the castings, but then you may need to adjust the epsilon, e.g. `CheckResidualsGradient(x, res, 1e-3)`.
* you cannot check the loss function directly for NLLS, you can only check the residuals' gradients.

For general optimization it's simpler:

```cpp
auto loss = [&](const auto &x, auto &grad) {
  double y = x - 42.0;
  double cost = 3 * y * y + std::pow(y, 4.0) - 2.0;
  if constexpr (!traits::is_nullptr_v<decltype(grad)>)
    grad(0) = 2 * 3 * y + 4 * std::pow(y, 3.0);
  return cost;
};

double x = 40.1;
REQUIRE(diff::CheckGradient(x, loss, 1e-3)); // well, let's check my math!
Optimize(x, loss); // ok, let's optimize
```

### Sparse Systems

Ok so you have quite large and sparse systems? Just tell `Tinyopt` about it by simply
defining you loss to have a `SparseMatrix<double> &H` type instead of a Dense Matrix or `auto`.

*NOTE* that automatic differentation is not supported for sparse Hessian matrices but is for first order solvers.
In that case, simply use this loss signature `auto loss = []<typename T>(const auto &x, SparseMatrix<T> &gradient)`.

```cpp
auto loss = [](const auto &x, auto &grad, SparseMatrix<double> &H) {
  // Define your residuals
  const VecX res = 10 * x.array() - 2; // the residuals
  // Update the full gradient matrix, using the Jacobian J of the residuals w.r.t 'x'
  MatX J = Matx::Zero(res.rows(), x.size());
  for (int i = 0; i < x.size(); ++i) J(i, i) = 10;
  grad = J.transpose() * res;
  // Update the (approx. or real) Hessian
  std::vector<Eigen::Triplet<double>> triplets;
  triplets.reserve(x.size());
  for (int i = 0; i < x.size(); ++i) triplets.emplace_back(i, i, 10 * 10);
  H.setFromTriplets(triplets.begin(), triplets.end());
  // Returns the norm + number of residuals
  return Cost(res.squaredNorm(), res.size()); // you can also return Cost(res), we'll take it's squared L2 norm!
};

```

As an alternative, you can use the `Optimizer<SparseMatrix<MatX>>` class instead of the `Optimize` function.

There are many ways to fill `H` in Eigen, have a look at `tests/sparse.cpp` for some examples.

### User defined parameters

Let's say you have a fancy class/struct, say a `Rectangle` and you want to optimize its dimensions.
For instance,

```cpp
template <typename T> // Template is only needed if you need automatic differentiation
struct Rectangle {
  T area() const { return (p2 - p1).norm(); }
  T width() const { return p2.x() - p1.x(); }
  T height() const { return p2.y() - p1.y(); }
  Eigen::Vector<T, 2> center() const { return T(0.5) * (p1 + p2); }
  Eigen::Vector<T, 2> p1, p2; // top left and bottom right positions
};
```

Now if you wan to simply call `Optimize(rectangle, loss)` on your rectangle struct, you need to either add specific members and methods or use
a trait specialization of `params_trait` for you object type:

```cpp
namespace tinyopt::traits { // must be defined in tinyopt::traits

template <typename T>
struct params_trait<Rectangle<T>> {
  using Scalar = T;              // The scalar type
  static constexpr Index Dims = 4; // Compile-time parameters dimensions (use Eigen::Dynamic if unknown)
  // Execution-time parameters dimensions [OPTIONAL, if Dims is known)
  static int dims(const Rectangle<T> &) { return Dims; }

  // Convert a Rectangle to another type 'T2', e.g. T2 = Jet<T> [OPTIONAL, if no Jet]
  // Not needed if you use manual Jacobians instead of automatic differentiation
  template <typename T2> static Rectangle<T2> cast(const Rectangle<T> &rect) {
    return Rectangle<T2>(rect.p1.template cast<T2>(),
                         rect.p2.template cast<T2>());
  }

  // Define how to update the object members (parametrization and manifold)
  static void PlusEq(Rectangle<T> &rect,
                     const Eigen::Vector<Scalar, Dims> &delta) {
    // In this case delta is defined as 2 deltas for p1 and p2: [dx1, dy1, dx2, dy2]
    rect.p1 += delta.template head<2>();
    rect.p2 += delta.template tail<2>();
  }
};

} // namespace tinyopt::traits
```

You can also skip the trait all together if you add these members and methods directly to the class.
Now you can start optimizing your custom object, e.g.

```cpp
// Let's say I want the rectangle area to be 10*20, the width = 2 * height and
// the center at (1, 2).
auto loss = [&]<typename T>(const Rectangle<T> &rect) {
  using std::max;
  Eigen::Vector<T, 4> residuals;
  residuals[0] = rect.area() - 10.0f * 20.0f;
  residuals[1] = 100.0f * (rect.width() / max(rect.height(), T(1e-8f)) -
                            2.0f); // the 1e-8 is to prevent division by 0
  residuals.template tail<2>() = rect.center() - Eigen::Vector<T, 2>(1, 2);
  return residuals;
};

Rectangle rectangle;
Optimize(rectangle, loss);
// That's it, rectangle is now fitted to your loss
```

### How to avoid data copies?

How do I create a parameters wrapper struct that uses external data sources without expensive copies?
Well here is a way I enjoy using because it doesn't involve any pointers:

```cpp

template <typename MyPoses>
struct ParamsWrapper {
  static constexpr Index Dims = Dynamic;

  template <typename M = MyPoses, std::enable_if_t<std::is_reference_v<M>, int> = 0>
  // Constructor
  ParamsWrapper(MyPoses _poses) : poses{_poses} {}
  // Move constructor or constructor with a reference if !is_reference_v<M>
  ParamsWrapper(MyPoses &_poses) : poses{_poses} {}
  // Move constructor
  template <typename M = MyPoses, std::enable_if_t<!std::is_reference_v<M>, int> = 0>
  ParamsWrapper(MyPoses &&_poses) : poses{std::move(_poses)} {}

  int dims() const { return poses.size(); }

  // Returns a copy where the scalar is converted to another type 'T2'.
  // This is only used by auto differentiation
  template <typename T2>
  inline auto cast() const {
    auto poses2 = poses.template cast<T2>(); // This is a copy... but it's needed.
    using MyPoses2 = std::decay_t<decltype(poses2)>; // A tad verbose...
    return ParamsWrapper<MyPoses2>(std::move(poses2)); // Note the missing reference '&'!
  }

  // Define update / manifold
  ParamsWrapper &operator+=(const auto &delta) {
    poses += delta;
    return *this;
  }

  MyPoses poses;
  // And you can add more parameters here, so fun!
};

// You can now optimize x and the poses will be updated
MyPoses poses;
...
ParamsWrapper<MyPoses&> x(poses); // No copies here, Amazing! Note the reference '&'.
Optimize(x, loss);

```
Ok, there will be copies when using Auto Diff (which calls the `cast()` method), one per iteration.

### Numerical Differentiation
Not all cost functions are the same. By default, `tinyopt` will try to use automatic differentiation
when the function has only one parameter `x` but if your function does not allow it,
you can use a numerical differentiation one. Here is an example
```cpp

auto original_loss = [&](const auto &x) -> Vec3 { return 2 * (x - y_prior); };
auto new_loss = CreateNumDiffFunc1(x, original_loss);
// you can now pass this 'new_loss' to an optimizer, e.g. Optimize(x, new_loss);

```
*NOTE* `CreateNumDiffFunc1` is when using first order optimizers which use the gradient only and `CreateNumDiffFunc2` for
second or pseudo-second order methods, which use both gradient and Hessian.

### Losses and Norms
You can play with different losses, robust norms and M-estimators, have a look at `losses.h`.

Here is an example of a loss that uses a Mahalanobis distance with a covariance `C`.
```cpp

auto loss = [&]<typename T>(const Eigen::Vector<T, 2> &x) {
  const Matrix<T, 2, 2> C_ = C.template cast<T>();
  return MahaSquaredNorm(x - y, C_);  // return res.T * C.inv() * res
};

```
