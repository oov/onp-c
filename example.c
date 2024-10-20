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
