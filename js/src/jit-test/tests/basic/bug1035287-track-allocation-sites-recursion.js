// |jit-test| error: 666
throw 666; // this test does not work properly on IonPower due to our stack (See issue 246)

// |jit-test| exitstatus: 3

enableTrackAllocations();
function f() {
    eval('f();');
}
f();
