#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @enum onp_ses_type
 * @brief Enum representing the type of SES(shortest edit scripts) operation.
 */
enum onp_ses_type {
  onp_ses_type_delete = -1,
  onp_ses_type_common = 0,
  onp_ses_type_add = 1,
};

struct onp_params;

/**
 * @typedef onp_compare_fn
 * @brief Function pointer type for comparing elements.
 *
 * @param p Pointer to the onp_params structure.
 * @param aidx Index in the first sequence.
 * @param bidx Index in the second sequence.
 * @return True if elements are equal, false otherwise.
 */
typedef bool (*onp_compare_fn)(struct onp_params const *const p, size_t const aidx, size_t const bidx);

/**
 * @typedef onp_ses_fn
 * @brief Function pointer type for handling SES (shortest edit scripts) operations.
 *
 * @param p Pointer to the onp_params structure.
 * @param type Type of the SES operation.
 * @param aidx Index in the first sequence.
 * @param bidx Index in the second sequence.
 */
typedef void (*onp_ses_fn)(struct onp_params const *const p,
                           enum onp_ses_type const type,
                           size_t const aidx,
                           size_t const bidx);

/**
 * @typedef onp_realloc_fn
 * @brief Function pointer type for reallocating memory.
 *
 * @param ptr Pointer to the memory block to be reallocated.
 * @param size New size of the memory block.
 * @param userdata User data passed as o.userdata in onp_calc_distance.
 * @return Pointer to the reallocated memory block, or NULL if the reallocation fails.
 */
typedef void *(*onp_realloc_fn)(void *const ptr, size_t const size, void *const userdata);

/**
 * @typedef onp_free_fn
 * @brief Function pointer type for freeing memory.
 *
 * @param ptr Pointer to the memory block to be freed.
 * @param userdata User data passed as o.userdata in onp_calc_distance.
 */
typedef void (*onp_free_fn)(void *const ptr, void *const userdata);

/**
 * @struct onp_params
 * @brief Structure holding options for onp_calc_distance.
 */
struct onp_params {
  void const *a;
  void const *b;
  size_t alen;
  size_t blen;
  void *userdata;

  // Callback for comparing elements.
  onp_compare_fn compare;

  // Optional callback for handling SES operations.
  // You can set this to NULL if you don't need SES.
  onp_ses_fn ses;
  // If ses_skip_common is true, the SES callback is not called for common elements.
  bool ses_skip_common;

  // You can specify custom allocators by setting realloc and free.
  // If these are set to NULL, the default implementations are used.
  // These parameters are ignored when reusing context.
  onp_realloc_fn realloc;
  onp_free_fn free;
};

struct onp_context;

/**
 * @brief Calculates the diff distance between two sequences.
 *
 * This function calculates the Levenshtein distance.
 * If you need to calculate the distance multiple times, using the same context allows buffer reuse.
 *
 * @param d Double pointer to the onp_context structure.
 * @param o Pointer to the onp_params structure containing the sequences and options.
 * @return The Levenshtein distance. SIZE_MAX returned on error.
 */
size_t onp_calc_distance(struct onp_context **const d, struct onp_params const *const o);

/**
 * @brief Destroys the diff context.
 *
 * This function frees the memory pointed to by the diff context and sets the pointer to NULL.
 *
 * @param d Double pointer to the onp_context structure.
 */
void onp_destroy(struct onp_context **const d);
