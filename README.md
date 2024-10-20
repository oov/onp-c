onp-c
=====

onp-c is an implementation of "An O(NP) Sequence Comparison Algorithm" by described by Sun Wu, Udi Manber and Gene Myers in C language.

Build & Install
---------------

Static link libraries can be built in the following ways:

```sh
$ git clone https://github.com/oov/onp-c.git
$ cd onp-c
$ mkdir build
$ cmake -S . -B build --preset release -DBUILD_SHARED_LIBS=OFF
$ cmake --build build
$ cmake --install build --prefix build/local
```

By changing `-DBUILD_SHARED_LIBS=OFF` to `-DBUILD_SHARED_LIBS=ON`, shared libraries will also be built.

Alternatively, you can simply add `onp.h` and `onp.c` to your project, and it will work.

How to use
----------

```c
#include <stdio.h>
#include <string.h>

#include "onp.h"

// Compare two elements of the input arrays.
// Return true if the elements are equal.
static bool compare(struct onp_params const *const p, size_t const aidx, size_t const bidx) {
  char const *const a = p->a;
  char const *const b = p->b;
  return a[aidx] == b[bidx];
}

// Output the steps of the shortest edit script (SES).
// If type is onp_ses_type_delete, delete the a[aidx] element.
// If type is onp_ses_type_insert, insert the b[bidx] element.
// If type is onp_ses_type_common, the a[aidx] element and the b[bidx] element are equal.
static void ses(struct onp_params const *const p, enum onp_ses_type const type, size_t const aidx, size_t const bidx) {
  char const *const a = p->a;
  char const *const b = p->b;
  char const op = "- +"[type + 1];
  char const c = type == onp_ses_type_delete ? a[aidx] : b[bidx];
  printf("%c %c\n", op, c);
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  // Calculate the edit distance between two strings "kitten" and "sitting".
  // This example uses an array of char type, but it can be used for any data type.
  static char const a[] = "kitten";
  static char const b[] = "sitting";
  static size_t const alen = sizeof(a) - 1;
  static size_t const blen = sizeof(b) - 1;

  // Calculate the edit distance.
  // If you want to calculate multiple edit distances,
  // you can reuse this context to reuse the allocated memory.
  struct onp_context *ctx = NULL;
  size_t const r = onp_calc_distance(&ctx,
                                     &(struct onp_params){
                                         .a = a,
                                         .b = b,
                                         .alen = alen,
                                         .blen = blen,
                                         .compare = compare,
                                         .ses = ses,
                                     });
  if (r == SIZE_MAX) {
    return 1;
  }

  printf("distance: %zu\n", r);

  onp_destroy(&ctx);
  return 0;
}
```
```
+ s
- k
  i
  t
  t
+ i
- e
  n
+ g
distance: 5
```

Credits
-------

onp-c is made possible by the following open source softwares.

### [gonp](https://github.com/cubicdaiya/gonp)

<details>
<summary>MIT License</summary>

```
Copyright (c) 2014 Tatsuhiko Kubo <cubicdaiya@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
```
</details>