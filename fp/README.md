# Environment
- g++11
- Ubuntu 21.04
- Intel i7-1165G7

In order to illustrate results independent of output formatting, all results get converted to an unsigned integer value before printing.

# The 80-bit x87 FPU

Floating point math is inherently lossy due to limited precision for intermediates and the final result. 

Consider the following operation:

```
float a,b,c,d = ...;
float e = a * b + c * d;
```

This operation produces the intermediates `a*b`, `c*d`, before adding them together.

On modern x86 processors, fp ops should be compiled into SSE+ instructions, and the intermediates stored in SIMD registers:

```
ja@ja-ZenBook-UX363EA-UX363EA:~/proj/arch_diff/fptest$ g++ -g fp_nosetmask.cc 
ja@ja-ZenBook-UX363EA-UX363EA:~/proj/arch_diff/fptest$ cat inputs | ./a.out 
3121610752
ja@ja-ZenBook-UX363EA-UX363EA:~/proj/arch_diff/fptest$ objdump -S ./a.out 
...
        float cp = (*af)*(*df) - (*bf)*(*cf);
    1270:	48 8b 45 d8          	mov    -0x28(%rbp),%rax
    1274:	f3 0f 10 08          	movss  (%rax),%xmm1
    1278:	48 8b 45 f0          	mov    -0x10(%rbp),%rax
    127c:	f3 0f 10 00          	movss  (%rax),%xmm0
    1280:	f3 0f 59 c1          	mulss  %xmm1,%xmm0
    1284:	48 8b 45 e0          	mov    -0x20(%rbp),%rax
    1288:	f3 0f 10 10          	movss  (%rax),%xmm2
    128c:	48 8b 45 e8          	mov    -0x18(%rbp),%rax
    1290:	f3 0f 10 08          	movss  (%rax),%xmm1
    1294:	f3 0f 59 ca          	mulss  %xmm2,%xmm1
    1298:	f3 0f 5c c1          	subss  %xmm1,%xmm0
    129c:	f3 0f 11 45 d0       	movss  %xmm0,-0x30(%rbp)

...
```

On ARM, (TODO but it produces basically the same as above):


```
```

In both cases, the intermediates are stored at the same precision as their operands, and we get the same values.

However, x86 processors also contain a legacy x87 FPU, which stores its intermediates at 80-bit precision by default. As a result, we can actually get a completely different result by disabling SSE. 

Note that on Linux, you must install the cross-compile toolchain for 32-bit and compile to that. SSE2 is mandatory for AMD64 processors and 64-bit libstdc on Linux is compiled with this assumption in mind, so g++ will complain.

```
# sudo apt install g++-multilib
ja@ja-ZenBook-UX363EA-UX363EA:~/proj/arch_diff/fptest$ g++ -g -m32 -mno-sse fp_nosetmask.cc 
ja@ja-ZenBook-UX363EA-UX363EA:~/proj/arch_diff/fptest$ cat inputs | ./a.out 
3122002152
ja@ja-ZenBook-UX363EA-UX363EA:~/proj/arch_diff/fptest$ objdump -S ./a.out 
...
        float cp = (*af)*(*df) - (*bf)*(*cf);
    1273:	8b 45 e0             	mov    -0x20(%ebp),%eax
    1276:	d9 00                	flds   (%eax)
    1278:	8b 45 ec             	mov    -0x14(%ebp),%eax
    127b:	d9 00                	flds   (%eax)
    127d:	de c9                	fmulp  %st,%st(1)
    127f:	8b 45 e4             	mov    -0x1c(%ebp),%eax
    1282:	d9 00                	flds   (%eax)
    1284:	8b 45 e8             	mov    -0x18(%ebp),%eax
    1287:	d9 00                	flds   (%eax)
    1289:	de c9                	fmulp  %st,%st(1)
    128b:	de e9                	fsubrp %st,%st(1)
    128d:	d9 5d dc             	fstps  -0x24(%ebp)
...
```

There are a couple of workarounds. GCC provides `-ffloat-store` which requires that all intermediates be written back to an actual register, thus rounding them back down. This will likely cause a perf hit in real workloads.

```
ja@ja-ZenBook-UX363EA-UX363EA:~/proj/arch_diff/fptest$ g++ -g -ffloat-store -m32 -mno-sse fp_nosetmask.cc
ja@ja-ZenBook-UX363EA-UX363EA:~/proj/arch_diff/fptest$ cat inputs | ./a.out
3121610752
        // cross product
        float cp = (*af)*(*df) - (*bf)*(*cf);
    1273:	8b 45 e0             	mov    -0x20(%ebp),%eax
    1276:	d9 00                	flds   (%eax)
    1278:	d9 5d c4             	fstps  -0x3c(%ebp)
    127b:	8b 45 ec             	mov    -0x14(%ebp),%eax
    127e:	d9 00                	flds   (%eax)
    1280:	d9 5d c8             	fstps  -0x38(%ebp)
    1283:	d9 45 c4             	flds   -0x3c(%ebp)
    1286:	d8 4d c8             	fmuls  -0x38(%ebp)
    1289:	d9 5d cc             	fstps  -0x34(%ebp)
    128c:	8b 45 e4             	mov    -0x1c(%ebp),%eax
    128f:	d9 00                	flds   (%eax)
    1291:	d9 5d d0             	fstps  -0x30(%ebp)
    1294:	8b 45 e8             	mov    -0x18(%ebp),%eax
    1297:	d9 00                	flds   (%eax)
    1299:	d9 5d d4             	fstps  -0x2c(%ebp)
    129c:	d9 45 d0             	flds   -0x30(%ebp)
    129f:	d8 4d d4             	fmuls  -0x2c(%ebp)
    12a2:	d9 5d d8             	fstps  -0x28(%ebp)
    12a5:	d9 45 cc             	flds   -0x34(%ebp)
    12a8:	d8 65 d8             	fsubs  -0x28(%ebp)
    12ab:	d9 5d dc             	fstps  -0x24(%ebp)
    12ae:	d9 45 dc             	flds   -0x24(%ebp)
    12b1:	d9 5d c0             	fstps  -0x40(%ebp)
```

The other option is we can set the legacy FPU to use the correct percision for intermediates:

```
ja@ja-ZenBook-UX363EA-UX363EA:~/proj/arch_diff/fptest$ g++ -g -m32 -mno-sse fp.cc
ja@ja-ZenBook-UX363EA-UX363EA:~/proj/arch_diff/fptest$ cat inputs | ./a.out 
3121610752
ja@ja-ZenBook-UX363EA-UX363EA:~/proj/arch_diff/fptest$ objdump -S ./a.out
...
	    fpu_control_t mask;
	    _FPU_GETCW(mask);
    1273:	d9 7d ca             	fnstcw -0x36(%ebp)
	    mask &= ~_FPU_EXTENDED; // disable 80-bit ext prec
    1276:	0f b7 45 ca          	movzwl -0x36(%ebp),%eax
    127a:	80 e4 fc             	and    $0xfc,%ah
    127d:	66 89 45 ca          	mov    %ax,-0x36(%ebp)
	    mask |= _FPU_SINGLE; // enable single prec
    1281:	0f b7 45 ca          	movzwl -0x36(%ebp),%eax
    1285:	66 89 45 ca          	mov    %ax,-0x36(%ebp)
	    _FPU_SETCW(mask);
    1289:	d9 6d ca             	fldcw  -0x36(%ebp)
        // cross product
        float cp = (*af)*(*df) - (*bf)*(*cf);
    128c:	8b 45 e0             	mov    -0x20(%ebp),%eax
    128f:	d9 00                	flds   (%eax)
    1291:	8b 45 ec             	mov    -0x14(%ebp),%eax
    1294:	d9 00                	flds   (%eax)
    1296:	de c9                	fmulp  %st,%st(1)
    1298:	8b 45 e4             	mov    -0x1c(%ebp),%eax
    129b:	d9 00                	flds   (%eax)
    129d:	8b 45 e8             	mov    -0x18(%ebp),%eax
    12a0:	d9 00                	flds   (%eax)
    12a2:	de c9                	fmulp  %st,%st(1)
    12a4:	de e9                	fsubrp %st,%st(1)
    12a6:	d9 5d dc             	fstps  -0x24(%ebp)
...
```

# FMA (Fused Multiply-Add)
Another interesting intricacy of the x86 ISA is that FMA instructions (a*b + c) have effectively infinite intermediates and so do not produce the same result as a mulss + addss. This is only a problem on a subset of computations, for example `1.2 * 2.3 + 3.4` which is the example below.

From my testing, g++ will not enable this automatically until
 
You can automatically enable FMA by using the flag `-mfma`.
```
ja@ja-ZenBook-UX363EA-UX363EA:~/proj/arch_diff/fptest$ g++ -g -mfma fma.cc 
ja@ja-ZenBook-UX363EA-UX363EA:~/proj/arch_diff/fptest$ cat fma_inputs | ./a.out 
FMA Result: 4618621561853538467
a*b + c Result: 4618621561853538468
```





