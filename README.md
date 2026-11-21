# Blox

_A simplified Lox to BrainF\*ck Compiler._

```js
array a[4];
a[0] = 'A';
a[1] = 'B';
a[2] = 'C';
a[3] = 'D';

fun modify(arr) {
    print arr;
    print arr[0];
    print arr[1];
    print arr[2];
    print arr[3];
    arr[0] = 'E';
}

// Should not return an array, only the first element
fun returnArr(fourthEl) {
    array ret[5];
    ret[3] = fourthEl;
    ret[0] = 'F';
    returns ret;
}
modify(a);
print a[0];
print(returnArr(4));
```

## Inspiration

Blox is a passion project I worked on to practice my C programming skills. While I was at it, I decided to try out a new editor, Zed, and learn some basic Helix keybindings to use my mouse less. My project started with me following the masterfully writen [Crafting Interpreters](https://craftinginterpreters.com/) by Robert Nystrom. I stopped near the classes and set off on my own to compile a simplified set of the Lox programming language to BrainF*ck (henceforth shortened to BF). One goal I set out with was to not use any AI for debugging, asking questions, or writing code. I ended up with a semi-finished project that made a working BF to BF interpreter (check out the [example](./examples/bfinbf.txt)).

## Getting Started

1. Clone the repo
2. Compile the code:

   ```sh
    make
   ```

3. Run the compiler on `input.txt` to produce `output.bf`:

   ```sh
   ./blox input.txt > output.bf
   ```

## Language Introduction

```js
// Comments start with //
// Lines must end in semi-colons

// Primitives
var t = true;  // = 1
var f = false; // = 0
var e = nil;   // = 0
var a = 'A';   // = 65
var n = 89;    // = 89

// Output with print
print 'A';
print 67; // C
// Get one character as input with input (BF=,)
var in = input;
print input;

// Variables start with var
var someName = 75;
someName = 33; // Reassign
// You cannot recreate a variable with the same name in the same scope
// The following is an error
// var someName = 43

// Math
print 55 + 10; // = 65 = A
print 68 - 2; //  = 66 = B
print 10 * 7; //  = 70 = F
// Integer division
print 90 / 7; //  = 12 = ␌ (Form feed character)
// Modulus
print 14 % 6; //  = 2  = ␂ (Start of text)

// Logic
print 1 < 3;  // 1
print 1 > 3;  // 0
print 1 <= 1; // 1
print 1 >= 2; // 0
print 'A' == 65; // 1
print nil != false; // 0
print 1 and 1; // 1
print 0 or 1; // 1
print !1;  // 0
print !65; // 0
print !0;  // 1

// Branching
if(0) {
  print 0; // Does not run
} else if(1 < 0) {
  print 0; // Does not run
} else {
  print 1; // Runs
}

// Short-circuiting
var maybeInput = 0 and input; // Does not request input
var howAboutThis = 1 or input; // Does not request input

// Loops
var flag = 1;
var pattern = 3;
while(flag) {
  pattern = pattern * pattern;
  flag = pattern > 200;
}

for(var i = 0; i < 9; i = i + 1) {
  print i;
}

// Arrays
array letters[4];    // Create array of size 4
letters[0] = 'a';
letters[1] = 'b';
letters[2] = 'c';
letters[3] = 'd';
letters[4] = 4;       // Allowed but unsafe and might mess up the rest of the program
print letters;        // Prints the full array = abcd
// If an array is used anywhere but print, it is only
// the first element
letters[2] = letters; // Sets index 2 to letters[0]
print letters[3];     // Prints d
// You cannot reassign to arrays: letters = 4;

// Functions (more like macros)
// - recursion is not Allowed
// - functions return 0 by default
fun someFun(arg1, arg2) {
  print arg1;
  returns arg1; // returns sets the function return value
  print arg2;   // everything after returns still runs
  // closures are allowed
  fun mult(n1, n2) {
    returns n1 * n2 * arg1;
  }
  returns mult(arg1, arg2); // return value is overloaded
}
print someFun(5, 3); // = 75 = 'K'
```