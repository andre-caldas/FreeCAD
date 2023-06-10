# GCS: Geometrical Constraint System

This GCS is not, in fact, geometrical.
The only geometric part is that the fundamental equations we use
might have geometrical meaning.

What we really have is a system of equations we want to solve.
We do not manage "coincident points", "horizontal constraint",
"vertical constraint", "distance constraint", etc.
But we do have classes that represent analytical conditions:
*two equal parameters*,
*difference of two parameters equal to some third parameter*,
*three points 'colinearity'*.
All the higher level geometrical stuff must be kept outside the GCS.

## Constraints

Constraints are made from equations like:
```math
\left\{
\begin{aligned}
  x_1 {}&-&{} x_2 {}&&{} &= 5
  \\
  {}&&{}x_2^2 {}&+&{} x_3^2 &= 64.
\end{aligned}
\right.
```
Let's make them all equal to zero...
```math
\left\{
\begin{aligned}
  x_1 {}&-&{} x_2 {}&+&{} {}&-&{} 5 &= 0
  \\
  {}&&{}x_2^2 {}&+&{} x_3^2 {}&-&{} 64 &= 0.
\end{aligned}
\right.
```
So, we are actually dealing with a system of equations:
```math
\left\{
\begin{aligned}
  f_1(x_1, x_2, \times) &= 0
  \\
  f_2(\times, x_2, x_3) &= 0.
\end{aligned}
\right.
```
Usually, each row depends on just a few parameters.
And of course, we are talking about much more than 3 parameters and 2 equations! :-)
```math
  F: \mathbb{R}^p \rightarrow \mathbb{R}^q.
```
We want to find a solution for
```math
  F(x_1, \dotsc, x_p) = (0, \dotsc, 0).
```
Or, $F(x) = 0$, for short.


# Qualitative analysis

If we know that $F(x) = a$,
we might call $a$ the "error"...
and we want to figure out an amount $\Delta x$ such that
$$F(x + \Delta x) = 0.$$
That is,
$$F(x + \Delta x) - F(x) = -a.$$
We do not know where $F$ assumes the value $0$.
Through a linear approximation, we can deal with the easier linear problem:
$$DF(x) \Delta x = -a.$$
Here, $DF(x)$ is the derivative of $F$ at the point $x$.
It is a linear transform.
It can be thought as a matrix where the $j$-th *row* is the *gradient*
of the corresponding $f\_j$ at $x$.
That is,
```math
DF(x) =
\begin{bmatrix}
  \rule[.5ex]{2.5ex}{0.5pt} & \nabla f_1 & \rule[.5ex]{2.5ex}{0.5pt}
  \\
  &\vdots&
  \\
  \rule[.5ex]{2.5ex}{0.5pt} & \nabla f_n & \rule[.5ex]{2.5ex}{0.5pt}
\end{bmatrix}
```


## Fully constrained

Suppose we have found a point $x$ where $f\_1, \dotsc, f\_n$ are satisfied.
We say that $F$ is fully constrained if there are no paths departing from $x$
such all $f\_j$ are kept constant.
Let us be a little euristic and assume that $F$ is fully constrained when
$\nabla f\_1(x), \dotsc, \nabla f\_n(x)$ spawn $\mathbb{R}^p$:
they are a **generator**.


## Overconstrained

And we want to add another equation to our system, given by the function $f\_{q+1}$.
Suppose we have found a point $x$ where $f\_1, \dotsc, f\_n$ are satisfied.
We say that $f\_{q+1}$ overconstraints the system $F$
if we cannot move in a direction perpendicular to
$\nabla f\_1(x), \dotsc, \nabla f\_n(x)$
but at the same time non-perpendicular to $\nabla f\_{q+1}(x)$.
The idea is that, at the point $x$,
we cannot change $f\_{q+1}$ without changing the $f\_1, \dotsc, f\_n$.

Let us say that $f\_{q+1}$ overconstraints the system at $x$
when $\nabla f\_{q+1}(x)$ linearly dependent to $\nabla f\_1(x), \dotsc, \nabla f\_n(x)$.

So, we say that a system $f\_1, \dotsc, f\_n$ has no overconstraints at $x$ when
$\nabla f\_1(x), \dotsc, \nabla f\_n(x)$
are linearly independent.

The overconstraining $f\_{q+1}$ can be compatible with $f\_1, \dotsc, f\_n$,
in the sense that $F(x) = 0$ can be satisfied/solved.
In this case, the new constraint is just redundant.

**OBS:**
I this implementation, we do not discard overconstraints.
We treat them as "constraints for checking".
If those cannot be satisfied, we alert the user.

## The point of evaluation

Sometimes the gradient does not depend on the point being considered (linear constraint).
When some gradient is dependent of the point,
all the above considerations are dependent on the point.
So, in search for a solution to $F(x) = 0$,
whenever the point $x$ is not a solution and at the given point the system is overconstrained,
we might disturb the system and evaluate the situation at a (or some) nearby point(s).

## Orthonormal basis (reduced QR-decomposition)

A set of vectors $\vec{v}\_1, \dotsc, \vec{v}\_n$ is [orthonormal](https://en.wikipedia.org/wiki/Orthonormality) when
$$\vec{v}\_j \cdot \vec{v}\_k = \delta\_{jk}.$$
Orthonormal vectors are nice because:
1. It is very easy to decompose vectors:
$$\vec{w} = \alpha\_1 \vec{v}\_1 + \dotsb + \alpha\_n \vec{v}\_n + \vec{w}^\perp.$$
Where $\alpha\_j = \vec{w} \cdot \vec{v}\_j$ and for all $j$,
$\vec{w}^\perp \cdot \vec{v}\_j = 0$.
2. If $M$ is a matrix whose columns (or rows) are orthonormal vectors,
it is very easy to invert $M$: $M^{-1} = M^T$.

The so called [Gram-Schmidt process](https://en.wikipedia.org/wiki/Gram%E2%80%93Schmidt_process)
allow us to "gradually build up" from
linearly independent $\vec{v}\_1, \dotsc, \vec{v}\_n$,
a sequence of orthonormal vectors
$\vec{q}\_1, \dots, \vec{q}\_n$ such that
```math
\begin{aligned}
  \vec{v}_1 &= \alpha_{11} \vec{q}_1
  \\
  \vec{v}_2 &= \alpha_{21} \vec{q}_1 + \alpha_{22} \vec{q}_2
  \\
  &\vdots
  \\
  \vec{v}_n &= \alpha_{n1} \vec{q}_1 + \dotsb + \alpha_{nn} \vec{q}_n.
\end{aligned}
```
In matrix notation
```math
\begin{bmatrix}
  \vert & & \vert
  \\
  \vec{v}_1 & \ldots & \vec{v}_n
  \\
  \vert & & \vert
\end{bmatrix} =
\begin{bmatrix}
  \vert & & \vert
  \\
  \vec{q}_1 & \ldots & \vec{q}_n
  \\
  \vert & & \vert
\end{bmatrix}
\begin{bmatrix}
  \alpha_{11} & \ldots      & \ldots & \alpha_{n1}
  \\
              & \alpha_{22} & \ldots & \alpha_{n2}
  \\
              &             & \ddots & \vdots
  \\
              &             &        & \alpha_{nn}
\end{bmatrix}
```
For short,
$$V = QR.$$

The process can be followed through, even if the original
$\vec{v}\_1, \dotsc, \vec{v}\_n$ are not linearly independent.
Except, that whenever $\vec{v}\_j$ is dependent on the vectors that came before,
$\vec{q}\_j$ is zero.
And, in this case, $\vec{q}\_j$ is removed from $Q$,
and so is the $j$-th line of $R$.

**OBS:**
I am not sure, but I belive that
[the $Q$ matrix is often "completed"](https://en.wikipedia.org/wiki/QR\_decomposition#Square\_natrix)
to become an orthonormal basis (therefore, a square matrix).
I don't think that in FreeCAD we need this completion.
Here, we do not assum $Q$ is squared.
In this work, the space spawned by the columns of $Q$
is exactly the same as the space spawned by the initial vectors
$\vec{v}\_1, \dotsc, \vec{v}\_n$.
If we want to project a vector $\vec{w}$
into the space spawned by
$\vec{v}\_1, \dotsc, \vec{v}\_n$, we just have to calculate $QQ^T\vec{w}$.

### Overconstraints

So, not necessarily the QR-decomposition,
but the Gram-Schmidit process is a nice way to determine,
given an **ordered** list of equations,
at which point the system becomes overconstrained.

### Constraints

In the case of constraints, the vectors $\vec{v}\_1, \dotsc, \vec{v}\_n$
are in fact the gradients $\nabla f\_1(x), \dotsc, \nabla f\_n(x)$.

### Adding or removing vectors/constraints/geometries

It is very important to notice that this process is **gradual**.
That is,
if we keep a record of the orthonormalized vectors $\vec{q}\_1, \dotsc, \vec{q}\_n$,
it is very easy to add a new vector $\vec{v}\_{n+1}$ (constraint) to the list.
All we need to do is calculate
$$\tilde{q}\_{n+1} = \vec{v}\_{n+1} - \sum\_{j=1}^m (\vec{v}\_{n+1} \cdot \vec{q}\_j) \vec{q}\_j,$$
and then normalize it:
$$\vec{q}\_{n+1} = \frac{1}{\lVert \tilde{q}\_{n+1} \rVert} \tilde{q}\_{n+1}.$$
Later we discuss how to add a constraint in the middle of the list.

Adding a new dimension (geometry) is even easier.
We can simply add a new entry to every vector.
We set it to zero (new unconstrained geometry).

Removing a dimension is complicated
unless every vector has the corresponding entry equal to zero.
Since this is the case of unconstrained geometries,
it is very easy to remove those.

Removing a constraint is a little more complicated,
unless it is the last constraint in the list.
In this case, we can simply remove it.
To remove an arbitrary constraint,
we can keep swapping it with the next one until it becomes the last constraint.
Then we remove it.

Swapping to consecutive constraints $\vec{v}\_j$ and $\vec{v}\_{j+1}$
only affects $\vec{q}\_j$ and $\vec{q}\_{j+1}$.
Make
```math
\begin{aligned}
  \tilde{p}_j &= \vec{q}_{j+1} + (\vec{v}_{j+1} \cdot \vec{q}_j) \vec{q}_j
  \\
  \tilde{p}_{j+1} &= \vec{q}_j - (\vec{v}_{j+1} \cdot \vec{q}_j) \vec{q}_{j+1}
  \\
  \vec{q}_{j} &= \frac{1}{\lVert \tilde{p}_{j} \rVert} \tilde{q}_{j}
  \quad\text{and}\quad
  \vec{q}_{j+1} = \frac{1}{\lVert \tilde{p}_{j+1} \rVert} \tilde{q}_{j+1}.
\end{aligned}
```

### Disturbing a vector

If we want to substitute $\vec{v}\_j$ by $\vec{v}\_j + \vec{w}$ and recompute
the reduced QR-decomposition, one thing we can do is:
1. Get rid of the $\vec{w}$ components for indexes before $j$:
$$\vec{w} -= \sum\_{k=1}^{j} (\vec{w} \cdot \vec{q}\_k) \vec{q}\_k.$$
2. If $\vec{w}$ becomes $\vec{0}$, there is nothing to be done.
3. Replace $\vec{q}\_j += \vec{w}$ and normalize it.
4. For $k = j+1, \dotsc, n$, replace
$$\vec{q}\_k -= (\vec{w} \cdot \vec{q}\_k) \vec{q}\_k$$
and normalize it.

In the case of constraints,
we might want to list first the gradients for the linear functions,
because they do not depend on the point being evaluated.
In this case,
adding a new linear constraint is the case of adding it to the middle of the list.
The procedure is very similar to the described above.

### Adding a constraint to the middle of the list

Suppose we want to add a constraint just after the vectors
$\vec{v}\_1, \dotsc, \vec{v}\_j$
and before
$\vec{v}\_{j+1}, \dotsc, \vec{w}\_n$.
We can simply add it to the end of the first list, as we usually do.
Call it $\vec{q}$, by now.
Then, for $k = 1, 2, \dotsc$ make
```math
  \tilde{q}_{j+1+k} = \vec{q}_{j+1+k} - (\vec{q}_{j+1+k} \cdot) \vec{q}) \vec{q}.
```
and normalize to obtain $\vec{q}\_{j+1+k}$.
And finally, make $\vec{q}\_j = \vec{q}$.

### Overconstrained only in a particular point

In general,
the gradient $\nabla f(x)$ at the point $x$ **does depend** on the chosen point $x$.
If we are only interested in analyzing constraintness,
we do not need to calculate it at every point.
Mathematically speaking,
the condition of being linearly independent is "open".

We need to decide how to choose points for evaluation.
Maybe, the class that implements each equation could give us a hint.

## Solving

We want to find a point $X$ such that $F(X) = 0$.
We search for such a point using linear approximations.
Starting at a point $x$, we want to go a step further, aiming at $F = 0$.
We calculate $F(x)$ and look for a *"step"* $\Delta x$ such that
```math
  DF(x)\Delta x = -F(x).
```
Heuristically,
```math
  F(x + \Delta x) - F(x) \approx DF(x)\Delta x = -F(x)
  \Rightarrow
  F(x + \Delta x) \approx 0.
```

### Step vector

In order to solve the equation
```math
  DF(x)\Delta x = -F(x),
```
we use a [least squares solver](https://docs.w3cub.com/eigen3/group__leastsquares).
[Eigen3](https://eigen.tuxfamily.org/)
is the library FreeCAD uses for solving this kind of problem.
Some options are:
1. Use [least squares conjugate gradient](https://eigen.tuxfamily.org/dox/classEigen_1_1LeastSquaresConjugateGradient.html).
2. Use [BiCGSTAB](https://eigen.tuxfamily.org/dox/classEigen_1_1BiCGSTAB.html) to solve
$DF(x)^T DF(x) \Delta x = -DF(x)^T F(x)$.
3. Use [LDLT Cholesky decomposition](https://eigen.tuxfamily.org/dox/classEigen_1_1LDLT.html) to solve
$DF(x)^T DF(x) \Delta x = -DF(x)^T F(x)$.

Notice that our main target is not to solve $DF(x) \Delta x = -F(x)$.
We are actually looking for $\Delta x$ such that $F(x + \Delta x) = 0$.
The linear problem gives us a **"long shot"**.
Therefore, **we do not need** *great precision*.
And we need to define what we mean by *"great precision"*.
The options [least squares conjugate gradient](https://eigen.tuxfamily.org/dox/classEigen_1_1LeastSquaresConjugateGradient.html) and [BiCGSTAB](https://eigen.tuxfamily.org/dox/classEigen_1_1BiCGSTAB.html) allow setting a "tolerance" by using the method `setTolerance()`.
On the other hand,
[documentation mentions](https://docs.w3cub.com/eigen3/group__leastsquares#LeastSquaresNormalEquations) that
[LDLT Cholesky decomposition](https://eigen.tuxfamily.org/dox/classEigen_1_1LDLT.html)
is the fastest method.

#### Further improvement

The gradient lines are not straight.
It is probably true that a better $\Delta x$ is a little bigger or smaller.
We also want to avoid too big overshooting that could make the method unstable.
Therefore, we shall search for the $\alpha \in [0,1.5]$ such that
$\lvert F(x + \alpha \Delta x)$
is minimized.

For example:
1. Start with $\alpha = 1$ and $r = 0.5$.
2. Choose among $F(x + (\alpha - r)\Delta x)$, $F(x + \alpha \Delta x)$
and $F(x + (\alpha + r)\Delta x)$, the one that is closest to $0$.
3. Call the best one, the new $\alpha$. and make $r /= 2$.
4. Goto "2" many times. :-)
5. Substitute $\Delta x *= \alpha$.

### Giving up

We also need to find a "good" criteria for giving up the search.
Could be a hard number of iterations, for example.

### Goal

We are done when $\lvert F(x) \rvert < \varepsilon$ for some $\varepsilon > 0$.
How do we know what is a good $\varepsilon$?


# Some pathologies and ideas

## Vibrations

Imagine that we have many parameters A, B, C, ..., Z constrained to be **equal**.
Suppose this constraint is satisfied: $A = B = \dotsb = Z = 0$.
Suppose that the constraint is implemented as:
$A = B$, $B = C$, $C = D$, ..., $Y = Z$,
like a spring.
Now, suppose that we impose $A = 1$.
Then, A will pull B and B will pull A.
In the next step, A will be pulled back to 1, B will pull A, C will pull B and B will pull C.
This would generate a "vibration" that would probably make convergences slow.

An improvement would be to constraint all parameters to being equal to only one of them.
That would reduce the amount of vibrations.
But we can do better!

## Redundant constraints

Redundant constraints are not bad!
We do not want them if we are still figuring out about under and overconstraints.
But when we are solving $DF(x)\Delta x = -F(x)$,
having redundant constraints would reduce vibrations.
The constraint $A = B = C = D$ can be translated to 6 constraints:
$A = B$, $A = C$, $A = D$, $B = C$, $B = D$ and $C = D$.
Those would make each of those parameters "feel" very heavy to move.

So, outside the GCS, the `ConstraintCoincident` class takes many points as parameters.
And it generates many constraints, including coincident ones.
Maybe we can have a limit on those, to avoid having too large a matrix.

Having points P and Q over the line that starts at point A and ends at B can be translated as:
evey set of three points (P, Q, A), (P, Q, B), (P, A, B) and (Q, A, B) are colinear.

For parameters being equal, we actually have a better suggestion:
proxied parameters.

## Proxied parameters

This is all **outside the GCS**.
The Sketcher shall have an array of parameters.
Each geometric object has pointers to those parameters:
proxied parameters.
When two parameters are supposed to be equal,
we do not actually pass the class `EquationEqual` to the GCS.
We simply make the two proxies point to the same parameter.

This can reduce the number of parameters, making the problem much easier to solve.
Do not forget that "trouble" grows exponentially with the number of parameters!

Also, some constraints can use those proxies to judge smarter ways
to choose the appropriate equations to use.
We have mentioned already that points over a line can be translated as points being colinear.
But, what if two of those points are constrained to have the same "y" coordinate (horizontal)?
Then all points being over the line becomes: the "x" coordinates of all of them are equal!
A complicated non linear set of constraints just disappeared!
All those points have now their x coordinate proxy pointing to the same parameter.

## GUI to help with overconstraints

Iterate through suggestions of underconstrained parameters (geometries).

Rotate "assertion constraints".
