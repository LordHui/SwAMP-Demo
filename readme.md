# SwAMP Demo User's Manual

Using this demo is supposed to be straightforward: one needs only to
open Matlab, go to the current folder and run the command `demo`.

When the demo starts, a compilation will take place.
SwAMP is written in C and must be compiled using Matlab's MEX API. If you
have a C compiler on your computer, everything should (hopefully) go smoothly!
We have tested the compilation using `gcc` in different platforms, but we'd
expect it to work with other compilers as well. Make sure to run `mex -setup` if you have no previously used Matlab's MEX feature. 

If you have problems, you can try the Python version which, in spite of
being much slower, achieves the same results.

## Key Reference
A. Manoel, F. Krzakala, E. W. Tramel, L. Zdeborovà, 
"Sparse Estimation with the Swept Approximated Message-Passing Algorithm," *arXiv submitted.*

## Contributors to this Repository
* **Andre Manoel,** *Original Source Author* `[andremanoel@gmail.com]`
* **Eric W. Tramel,** *Maintainer* `[eric.tramel@gmail.com]`

## A few details

- The demo script calls functions from the the `examples`
  folder. By exploring these, one may get a better grasp of how to use
  SwAMP.

- SwAMP's source code is located on the `src` folder; in particular, the
  bulk of the algorithm is contained in the `src/solvers/amp.c` file. 
  This version
  follows exactly the listings in the paper, and is already optimized to
  work with sparse matrices. Additionally, 3 other versions are present in
  the same folder: 
    * `gamp.c`, which implements G-SwAMP; 
    * `amp_dense.c`, a version that isn't optimized for sparse matrices; 
    * and `amp_alt.c`, a slight modification of the algorithm that, in spite of reaching the same results, sometimes converges faster.

## Trouble Shooting (Mac)
- Some distributions of MacTeX include a binary named `mex` which is generally located in your `/usr/texbin` directory. This may cause some issues if this is your first time building with Matlab's MEX compiler. Make sure that your Matlab binary directory is included at the *front* of your environment PATH. For example, when using MATLAB 2013b, the following lines would be added to the user's `.bashrc` or `.bash_profile`, 
```bash
    PATH=/Applications/MATLAB_R2013b.app/bin:$PATH
    export PATH
```
