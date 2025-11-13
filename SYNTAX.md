# Syntax

This is a strongly typed language with static type inference.  Type annotation is optional in most situations. Polymorphism is achieved through generics and function overloading. Semicolons are inserted automatically.

## Variable Declaration

To declare a 32-bit signed integer:

```
x : i32;
```

You can also assign to it:

```
x : i32 = 2;
```

If you do, you can omit the type annotation:

```
x := 2;
```

But if your desired type is different from that of the assigned value, you need to annotate or cast:

```
x : f32 = 2;
x := 2 as f32;
x := 2.0
```

Variable names consist of alphanumeric characters and the underscore.

## Numerical types

The following numerical types are supported:

```
i8,u8,i16,u16,i32,u32,i64,u64,f32,f64
```

The letter denotes signed(`i`) / unsigned(`u`) / floating point(`f`). The number denotes the size in bits.

When you do math with mixed types, operands implictly casted to the type with higher index in the above list.

```
x := 3 * 0.1;
y := (x + 1) / 2.0;
```

In addition to all the common math operators you'd expect a programming language to have, a unique pair of operators `<?=` (min equals) and `>?=` (max equals) are added to the language:

```
x := 3
x <?= 2    // x = MIN(x,2)
x >?= 4    // x = MAX(x,4)
```

## Vectors

Vectors are immutable collection of numerical values. You can do math with them just like you do with regular numbers.

Vectors are declared with its element type and dimension. For example, a 3D vector of floats:

```
v : vec[f32,3];
```

Vector literals can be expressed with the following syntax (verbose version):

```
v := vec[f32,3]{0.1, 0.2, 0.3};
```

Notice how the type annotation can be omitted. Since vectors are so commonly used, it gets a special shorthand:

```
v := {0.1, 0.2, 0.3};
```

To write it out fully (which you probably won't want):

```
v : vec[f32,3] = vec[f32,3]{0.1, 0.2, 0.3};
```

You can have 2-dimensional vectors, (or matrices as they're commonly called). For example, a 3 row by 2 columns matrix:

```
v : vec[f32,3,2];
```

You can use semicolons `;` in vector literals to delimit rows:

```
v := {0.1,0.2 ; 0.3,0.4 ; 0.5,0.6}
```

You can also have vectors of higher dimension. If you want to assign a literal to it, you can specify a 1D vector which will be automatically casted to the higer dimension, as long as the numbers of elements agree.

```
v : vec[i32,2,2,2] = {1,2, 3,4, 5,6, 7,8}
```

By default, a 1D vector is a row vector.

```
a : vec[i32,3] = {1,2,3}; // row vector
b : vec[i32,3,1] = {1,2,3}; // also row vector

c : vec[i32,1,3] = {1,2,3}; // column vector
d := {1; 2; 3}; // also column vector
```

You can do math with vectors (and numbers):

```
x := {1.0, 2.0}
x *= 2.0
x += {3.0, 4.0}
```

The regular `*` multiplies individual components (Hadamard product). There's a special operator `@*` for matrix multiplication:

```
x := {1.0, 2.0; 3.0, 4.0}
y := {5.0; 6.0}
z := x @* y;
```


You can index vectors with `0`,`1`,`2`,... or `x`,`y`,`z`,`w`, or even `r`,`g`,`b`,`a`. You can also do swizzling:

```
u := {1.0, 2.0, 3.0, 4.0};
v := u.x;
w := u.xyxy;
x := u[0];
y := u.bgr[1];
i := 3;
z := u[i];
```

You must not assign to individual components of the vector as they're immutable. You can assign a different vector to a variable of the same vector type. The dimension is also fixed, as it is part of the type. Think of vectors as values.

For mutable container types, see `list`s and `dict`s.

## Strings and characters

Strings are immutable. Strings literals can be expressed with double quotes `"`. 

```
x : str = "hello world"
```

You can use `%{}` syntax to do string interpolation. Strings can also go over mutliple lines.

```
x := 1
y := "the answer to x+1 is %{x+1}
when x equals %{x}.
nice!"
```

Single quotes denotes a character, which is a number representing the ASCII/unicode value.

```
x := 'h'
x == 104 // true
"hi"[0] == x // true
```

## Control Flow

Control flow is C-like.

If statement:

```
if (x == 1){
  // do something
}else if (y == 2 && z == 3){ // && and || are short-circuiting
  // do the other thing
}else{
  // do the other other thing
}
```

Loops:

```
for (i := 1; i < 10; i++){
  // do something in each iteration
  if (x == 1){
    break; // exit loop
  }
}
while (x == 1){
  // do something in each iteration
  if (y == 1){
    continue; // skip to next iteration
  }
}
do {
  // do something in each iteration
} while (x == 1);
```

## Tuples

Tuples are immutable collection of several variable of potentially different types. Tuple literals are denoted with square brackets `[]`.

```
x : tup[f32,str,i32] = [1.0, "hi", 2];
y := ["hello", {1,2,3}];
```

Tuples and vectors can be used for deconstructive assignment:

```
t := [1,2];
[a,b] := t;

v := {1,2,3};
({a,b}) = v.xy;
```

The parenthesis is necessary to prevent ambigous interpretation of `{}`.

## Functions

Functions can be declared with `func` keyword, and called like so:

```
func f(x:i32, y:i32):i32{
  return x+y;
}

func g(x:f32){
  return; // nothing
}

a := f(1,2);
```

Note that type annotation is necessary (for template matching purposes). You can overload a function to take different types and number of parameters:

```
func f(x:i32, y:i32){
  return x+y;
}
func f(x:i32, y:i32, z:i32){
  return x+y+z;
}
func f(x:i32){
  return f(x,1);
}
a := f(1,2)
b := f(1,2,3)
c := f(1)
```

You can also write a function one time to work with multiple data types through generics / templates:

```
func f[T](x:T, y:T): T{
  return x+y;
}
f(1,2);
f("hello","world")
```

Name the type to be matched in brackets `[]` after the function name, then annotate the rest of the function using that name. When you call the function, a version that matches the types of your arguments is generated automatically (compile time). A more convoluted example:

```
func f[T,S](x:T, y:S, z:vec[T,2]): T{
  a : S = y + 1;
  b : T = x * 2;
  z[0] = b;
  return z[1];
}
p := f(1.0, 2, {3.0,4.0});
q := f(5, "hi", {6,7});
```

You can assign a function to a variable and call it later.

```
func f(x:i32, y:i32):i32{
  return x+y;
}
a : func[tup[i32,i32],i32] = f; // type annotated for clarity
b := a(1,2)
```

Closures capture values, but not references. (if a captured variable is re-assigned elsewhere later, the function will not notice. However, if captured mutable-type variable is has its fields mutated, the change will be noticed).

```
func f(x:i32): func[tup[i32],i32]{
  a := x + 1;
  func g(y:i32):i32{
    return a + y; // a is captured from surrounding scope
  }
  return g;
}
h := f(3);
z := h(4); // 8
```

Function call supports unpacking vectors or tuples into arguments, e.g.:

```
func f(x:i32, y:i32, z:i32, w:i32){

}

x := {1,2,3};
y := [1,2];

f(...x, 4);
f(5, ...y, 6);
```

## Classes

Classes are declared with `typedef` keyword. Fields are listed inside.

```
typedef P = {
  x : f32;
  y : f32 = 1.0;
  z : f32 = 2.0;
}
```

Object literal uses the following syntax:

```
p := P{x:3.0, y:4.0}
```

You can also put functions in a type definition. You can refer the the object itself with `this`. To access a field, you can either do `this.x` or simply `.x` as a shorthand.

```
typedef P = {
  x : f32 = 0.0;
  y : f32 = 0.0;
  func f(z:f32) : f32{
    return this.x + .y + z;
  }
}
```

Type definition can also use templates/generics. For example:

```
typedef P[T] = {
  x : T;
  y : T;
  func f(z:T) : T{
    return this.x + .y + z;
  }
}
p := P[f32]{1.0, 2.0};
q := P[str]{"hello", "world"};
```

You can also use `typedef` to make an alias for a type, for example:

```
typedef v3f = vec[f32,3];

x : v3f = {1.0,2,3};
```

## Namespaces

Namespaces can be specified and refrenced like so:

```
namespace N{
  a := 3;
  b := 4;
  func f(x:i32) : i32{
    return x + 1;
  }
}

N.b = N.f(N.a)
```

## Import

Imports can be made with the `include` keyword:

```
include "path/to/my/script.dh"
include "path/to/my/c_library.so"
include "path/to/my/package"
```

If the path ends with `.so`, a dynamic library (binary) is imported. If the path is a folder, the compiler will determine which file to use inside the folder. Otherwise, a text file is imported.

## Lists

Lists are mutable containers of variable size, indexed with integer.

```
x := list[i32]{1,2,3};
```

To do useful things with lists, the list library needs to be imported:

```
include "std/list"

x := list[i32]{1,2,3,4,5};
list.length(x) == 5;
```

For `list`s (and other built-in types `vec`s, `dict`s and `str`s), a shorthand can be used for standard library functions that take them as the first argument:

```
x.length() == 5;
x.push(4);
x.slice(2,4);
```

You can nest object/list/vec literals.

```
x := list[vec[i32,5]]{
  { 1, 2, 3, 4, 5},  // ok
  vec[i32,5]{ 6, 7, 8, 9, 0} // also ok
};
x := list[P]{
  P{x:1,y:2}, 
  P{x:3,y:4}, 
};
```

## Dicts

Dicts are mutable containers that associate keys with values. Key can be any immutable type.

```
x := dict[str,i32]{"hi":2, "lo":3}

y := dict[i32,str]{1:"thing", 2:"stuff"};

z := dict[vec[i32,2],f32]{
  vec[i32,2]{1,2} : 3.0,
  vec[i32,2]{3,4} : 5.0,
}
```

# Standard Libraries

## io

```
include "std/io"

io.println("hello world");

io.print("how are you\n");

```

## str

```
include "std/str"

str.chr(104) // "h"
"hello".length() // 5
```

## vec

```
include "std/vec"

x := {1.0,2.0,3.0}
x.mag() // length / L2 norm
x.dir() // normalized
```


## list

```
include "std/list"

x := list[i32]{1,2,3,4,5}
x.length(); // number of items
x.push(4);  // append at the end
x.slice(2,4); // copy range (start, end(
```


## math

```
include "std/math"

x := 1.0
y := 2.0
math.sin(x)
math.atan2(y,x)
math.hypot(x,y)
math.PI
// ... and all the other stuff you can expect
```

## dr

A simple Processing/p5 style 2D graphics library.

Instead of having magical `setup` and `draw` functions, you just write a normal program.

```
include "std/drw"

drw.size(640,480) // make canvas

while (1){ // draw loop

  // red bg, color components are floats 0-1 :
  drw.background(1.0, 0.5, 0.5); 

  // draw stuff

  // refresh display and grab mouse/keyboard events
  event := drw.poll(); 
}

```

See examples for more usage.

## snd

A minimal library that writes to sound card.

```
include "std/snd"

snd.init(44100, 2); // sample rate, channels

while (1){
  if (!snd.buffer_full()){
    snd.put_sample( /* f32 from -1. to 1. */)
  }
}

snd.exit();
```
See examples for more usage.
