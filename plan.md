# Plan/Language Features Needed

Order
* Functions  
  fun tester(someParam, someOtherParam) {print someParam; return someOtherParam;} tester(65, 66);
* String
* Modulu operator
* Add characters '+'

- integer that can span more than u8 since memory is big (u8 u32...?)
- character
- array-like type
- while loop
- if and comparison
- plus/minus/increment/deincrement
- read input output
- Pointer needs to be ux where x is the max array size (30,000) - size of pointer/other used vars

Fixes: 
(functions) do not work in branches, local params are not removed?
```
fun trialOne(a, b) {
}

var i = 2;

trialOne(1, 2);

var j = 3;

trialOne(2, 3);

print 65 + i;
print 65 + j;

if(1) {
  trialOne(2, 3);
  print 65 + j;
}

var c = 3;

print 65 + i;

print 65 + c;
```

BONUS
- strings as arrays
- multiplication, devision

Other Thoughts
- Input reads until new line or 255 chars with a size stored as the first register
- Possibly need 256 * 256 * 256 ... cells for the pointer
