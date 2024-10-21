#include <onp.h>

#ifndef DEFAULT_ALLOCATOR
#  define DEFAULT_ALLOCATOR 1
#endif

#if DEFAULT_ALLOCATOR == 0
#  define def_realloc NULL
#  define def_free NULL
#else
#  include <stdlib.h>

static void *def_realloc(void *const ptr, size_t const size, void *const userdata) {
  (void)userdata;
  return realloc(ptr, size);
}

static void def_free(void *const ptr, void *const userdata) {
  (void)userdata;
  free(ptr);
}
#endif

struct xyr {
  size_t x, y, r;
};

struct xy {
  size_t x, y;
};

struct onp_context {
  onp_realloc_fn realloc;
  onp_free_fn free;

  struct onp_params const *params;

  void const *a;
  void const *b;
  size_t m;
  size_t n;

  size_t fppathlen;
  size_t *fp;
  size_t *path;

  size_t xyrlen;
  size_t xyrcap;
  struct xyr *xyr;

  size_t xylen;
  size_t xycap;
  struct xy *xy;

  bool swapped;
};

static inline size_t grow_buffer_size(size_t const x) { return x < 8 ? 8 : x * 3 / 2; }

static bool grow(struct onp_context *const ctx, void *const p, size_t const newsize) {
  void *np = ctx->realloc(*(void **)p, newsize, ctx->params->userdata);
  if (!np) {
    return false;
  }
  *(void **)p = np;
  return true;
}

void onp_destroy(struct onp_context **const ctxp) {
  if (!ctxp || !*ctxp) {
    return;
  }
  struct onp_context *const ctx = *ctxp;
  if (ctx->fp) {
    ctx->free(ctx->fp, ctx->params->userdata);
  }
  if (ctx->xyr) {
    ctx->free(ctx->xyr, ctx->params->userdata);
  }
  if (ctx->xy) {
    ctx->free(ctx->xy, ctx->params->userdata);
  }
  ctx->free(ctx, ctx->params->userdata);
  *ctxp = NULL;
}

static inline void
skip_common(struct onp_context const *const ctx, size_t const m, size_t const n, size_t *x, size_t *y) {
  size_t xx = *x;
  size_t yy = *y;
  while (xx < m && yy < n && ctx->params->compare(ctx->params, xx, yy)) {
    ++xx;
    ++yy;
  }
  *x = xx;
  *y = yy;
}

static size_t snake(struct onp_context *const ctx, size_t const k, size_t const offset) {
  size_t *const fp = ctx->fp;
  size_t const y1 = fp[k - 1 + offset] + 1;
  size_t const y2 = fp[k + 1 + offset];
  bool const y1_gt_y2 = y1 + 1 > y2 + 1;
  size_t y = y1_gt_y2 ? y1 : y2;
  size_t x = y - k;

  if (ctx->swapped) {
    skip_common(ctx, ctx->n, ctx->m, &y, &x);
  } else {
    skip_common(ctx, ctx->m, ctx->n, &x, &y);
  }

  if (ctx->params->ses) {
    size_t const r = ctx->path[(y1_gt_y2 ? k - 1 : k + 1) + offset];
    ctx->path[k + offset] = ctx->xyrlen;
    if (ctx->xyrlen == ctx->xyrcap) {
      size_t const newcap = grow_buffer_size(ctx->xyrcap);
      if (!grow(ctx, &ctx->xyr, newcap * sizeof(ctx->xyr[0]))) {
        return SIZE_MAX;
      }
      ctx->xyrcap = newcap;
    }
    ctx->xyr[ctx->xyrlen++] = (struct xyr){x, y, r};
  }
  return y;
}

static inline void call_ses(struct onp_context *const ctx, enum onp_ses_type type, size_t const px, size_t const py) {
  if (ctx->swapped) {
    ctx->params->ses(ctx->params, -type, py, px);
  } else {
    ctx->params->ses(ctx->params, type, px, py);
  }
}

static bool generate_ses(struct onp_context *const ctx, size_t const ridx) {
  ctx->xylen = 0;
  size_t r = ctx->path[ridx];
  while (r != SIZE_MAX) {
    struct xyr const *const xyr = &ctx->xyr[r];
    if (ctx->xylen == ctx->xycap) {
      size_t const newcap = grow_buffer_size(ctx->xycap);
      if (!grow(ctx, &ctx->xy, newcap * sizeof(ctx->xy[0]))) {
        return false;
      }
      ctx->xycap = newcap;
    }
    ctx->xy[ctx->xylen++] = (struct xy){xyr->x, xyr->y};
    r = xyr->r;
  }

  size_t px = 0;
  size_t py = 0;
  bool const skip_common = ctx->params->ses_skip_common;
  for (size_t i = ctx->xylen - 1; i < ctx->xylen; --i) {
    struct xy const *const xy = &ctx->xy[i];
    while ((px < xy->x) || (py < xy->y)) {
      size_t const xyd = xy->y - xy->x + 1;
      size_t const pd = py - px + 1;
      if (xyd > pd) {
        call_ses(ctx, onp_ses_type_add, px, py);
        ++py;
      } else if (xyd < pd) {
        call_ses(ctx, onp_ses_type_delete, px, py);
        ++px;
      } else {
        if (!skip_common) {
          call_ses(ctx, onp_ses_type_common, px, py);
        }
        ++px;
        ++py;
      }
    }
  }
  return true;
}

size_t onp_calc_distance(struct onp_context **const ctxp, struct onp_params const *const o) {
  if (!ctxp || !o || !o->compare) {
    return SIZE_MAX;
  }
  struct onp_context *ctx = *ctxp;
  if (!ctx) {
    onp_realloc_fn const realloc_fn = o->realloc ? o->realloc : def_realloc;
    onp_free_fn const free_fn = o->free ? o->free : def_free;
    if (!realloc_fn || !free_fn) {
      return SIZE_MAX;
    }
    ctx = realloc_fn(NULL, sizeof(struct onp_context), o->userdata);
    if (!ctx) {
      return SIZE_MAX;
    }
    *ctx = (struct onp_context){
        .realloc = realloc_fn,
        .free = free_fn,
    };
    *ctxp = ctx;
  }

  ctx->swapped = o->alen > o->blen;
  if (ctx->swapped) {
    ctx->a = o->b;
    ctx->b = o->a;
    ctx->m = o->blen;
    ctx->n = o->alen;
  } else {
    ctx->a = o->a;
    ctx->b = o->b;
    ctx->m = o->alen;
    ctx->n = o->blen;
  }
  ctx->params = o;

  size_t const offset = ctx->m + 1;
  size_t const delta = ctx->n - ctx->m;
  size_t const fplen = ctx->m + ctx->n + 3;

  size_t const fppathlen = fplen + (ctx->params->ses ? fplen : 0);
  if (ctx->fppathlen < fppathlen) {
    if (!grow(ctx, &ctx->fp, fppathlen * sizeof(*ctx->fp))) {
      return SIZE_MAX;
    }
    ctx->fppathlen = fppathlen;
  }
  size_t *const fp = ctx->fp;
  for (size_t i = 0; i < fppathlen; ++i) {
    fp[i] = SIZE_MAX;
  }

  ctx->path = fp + fplen;
  ctx->xyrlen = 0;
  for (size_t p = 0;; ++p) {
    // -p <= k <= delta - 1
    for (size_t k = 0; k <= p + delta - 1; ++k) {
      if ((fp[k + offset - p] = snake(ctx, k - p, offset)) == SIZE_MAX) {
        return SIZE_MAX;
      }
    }
    // delta + 1 <= k <= delta + p
    for (size_t k = delta + p; k >= delta + 1; --k) {
      if ((fp[k + offset] = snake(ctx, k, offset)) == SIZE_MAX) {
        return SIZE_MAX;
      }
    }
    // delta == k
    if ((fp[delta + offset] = snake(ctx, delta, offset)) == SIZE_MAX) {
      return SIZE_MAX;
    }
    if (fp[delta + offset] == ctx->n) {
      if (ctx->params->ses) {
        if (!generate_ses(ctx, delta + offset)) {
          return SIZE_MAX;
        }
      }
      return delta + 2 * p;
    }
  }
}
