/*
 * M*LIB - RED BLACK TREE module
 *
 * Copyright (c) 2017-2018, Patrick Pelissier
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * + Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * + Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef MSTARLIB_RBTREE_H
#define MSTARLIB_RBTREE_H

#include "m-core.h"

/* Define a Red/Black binary tree of a given type.
   USAGE: RBTREE_DEF(name, type [, oplist_of_the_type]) */
#define RBTREE_DEF(name, ...)                                           \
  RBTREEI_DEF(M_IF_NARGS_EQ1(__VA_ARGS__)                               \
              ((name, __VA_ARGS__, M_GLOBAL_OPLIST_OR_DEF(__VA_ARGS__)(), \
                M_C(name, _t), struct M_C(name, _node_s), M_C(name, _it_t)), \
               (name, __VA_ARGS__ ,                                     \
                M_C(name, _t), struct M_C(name, _node_s), M_C(name, _it_t))))

/* Define the oplist of a rbtree of type.
   USAGE: RBTREE_OPLIST(name [, oplist_of_the_type]) */
#define RBTREE_OPLIST(...)                                              \
  RBTREEI_OPLIST(M_IF_NARGS_EQ1(__VA_ARGS__)                            \
                 ((__VA_ARGS__, M_DEFAULT_OPLIST),			\
                  (__VA_ARGS__ )))


/********************************** INTERNAL ************************************/

// deferred evaluation
#define RBTREEI_OPLIST(arg) RBTREEI_OPLIST2 arg

#define RBTREEI_OPLIST2(name, oplist)                                   \
  (INIT(M_C(name, _init)),						\
   INIT_SET(M_C(name, _init_set)),					\
   INIT_WITH(API_1(M_INIT_VAI)),                                        \
   SET(M_C(name, _set)),						\
   CLEAR(M_C(name, _clear)),						\
   INIT_MOVE(M_C(name, _init_move)),					\
   MOVE(M_C(name, _move)),						\
   SWAP(M_C(name, _swap)),						\
   TYPE(M_C(name,_t)),							\
   SUBTYPE(M_C(name, _type_t)),						\
   TEST_EMPTY(M_C(name,_empty_p)),                                      \
   IT_TYPE(M_C(name, _it_t)),						\
   IT_FIRST(M_C(name,_it)),						\
   IT_SET(M_C(name,_it_set)),						\
   IT_LAST(M_C(name,_it_last)),						\
   IT_END(M_C(name,_it_end)),						\
   IT_END_P(M_C(name,_end_p)),						\
   IT_LAST_P(M_C(name,_last_p)),					\
   IT_EQUAL_P(M_C(name,_it_equal_p)),					\
   IT_NEXT(M_C(name,_next)),						\
   IT_REF(M_C(name,_ref)),						\
   IT_CREF(M_C(name,_cref)),						\
   CLEAN(M_C(name,_clean)),						\
   PUSH(M_C(name,_push)),						\
   GET_MIN(M_C(name,_min)),						\
   GET_MAX(M_C(name,_max)),						\
   M_IF_METHOD(GET_STR, oplist)(GET_STR(M_C(name, _get_str)),),		\
   M_IF_METHOD(PARSE_STR, oplist)(PARSE_STR(M_C(name, _parse_str)),),   \
   M_IF_METHOD(OUT_STR, oplist)(OUT_STR(M_C(name, _out_str)),),		\
   M_IF_METHOD(IN_STR, oplist)(IN_STR(M_C(name, _in_str)),),		\
   M_IF_METHOD(EQUAL, oplist)(EQUAL(M_C(name, _equal_p)),),		\
   M_IF_METHOD(HASH, oplist)(HASH(M_C(name, _hash)),)			\
   ,M_IF_METHOD(NEW, oplist)(NEW(M_GET_NEW oplist),)                    \
   ,M_IF_METHOD(REALLOC, oplist)(REALLOC(M_GET_REALLOC oplist),)        \
   ,M_IF_METHOD(DEL, oplist)(DEL(M_GET_DEL oplist),)                    \
   )

/* Max depth of the binary tree
   It is at worst twice the depth of a perfectly even tree with maximum elements.
   The maximum number of elements is the max of size_t.
   A perfectly even tree is of depth log2(max(size_t))=CHAR_BIT*sizeof(size_t)
 */
#define RBTREEI_MAX_STACK (2*CHAR_BIT*sizeof (size_t))

/* Encapsulation of the color of the nodes. */
#define RBTREEI_SET_RED(x)   ((x)->color =  RBTREE_RED)
#define RBTREEI_SET_BLACK(x) ((x)->color =  RBTREE_BLACK)
#define RBTREEI_IS_RED(x)    ((x)->color == RBTREE_RED)
#define RBTREEI_IS_BLACK(x)  ((x)->color == RBTREE_BLACK)
#define RBTREEI_COPY_COLOR(x,y) ((x)->color = (y)->color)
#define RBTREEI_GET_COLOR(x) (true ? (x)->color : (x)->color)
#define RBTREEI_SET_COLOR(x, c) ((x)->color = (c))
#define RBTREEI_GET_CHILD(x, n) ((x)->child[n])
#define RBTREEI_SET_CHILD(x, n, y) ((x)->child[n] = (y))

typedef enum {
  RBTREE_BLACK = 0, RBTREE_RED
} rbtreei_color_e;

// General contact of a tree
#define RBTREEI_CONTRACT(tree) do {					\
    assert ((tree) != NULL);						\
    assert ((tree)->node == NULL || RBTREEI_IS_BLACK((tree)->node));	\
    assert ((tree)->size != 0 || (tree)->node == NULL);			\
  } while (0)

// Contract of a node (doesn't check for equal depth in black)
#define RBTREEI_CONTRACT_NODE(node) do {				\
    assert((node) != NULL);						\
    assert(RBTREEI_IS_BLACK(node) || RBTREEI_IS_RED(node));		\
    assert(RBTREEI_IS_BLACK(node)					\
	   || (((node)->child[0] == NULL || RBTREEI_IS_BLACK(node->child[0])) \
	       && ((node)->child[1] == NULL || RBTREEI_IS_BLACK(node->child[1])))); \
  } while (0)

//TODO: UPDATE shall use a separate method than push

// deferred evaluation
#define RBTREEI_DEF(arg) RBTREEI_DEF2 arg

#define RBTREEI_DEF2(name, type, oplist, tree_t, node_t, tree_it_t)     \
                                                                        \
  node_t {                                                              \
    node_t *child[2];                                                   \
    type data;                                                          \
    rbtreei_color_e color;                                              \
  };                                                                    \
                                                                        \
  typedef struct M_C(name, _s) {					\
    size_t size;                                                        \
    node_t *node;                                                       \
  } tree_t[1];                                                          \
  typedef struct M_C(name, _s) *M_C(name, _ptr);                        \
  typedef const struct M_C(name, _s) *M_C(name, _srcptr);               \
									\
  typedef struct M_C(name, _it_s) {					\
    node_t *stack[RBTREEI_MAX_STACK];                                   \
    char    which[RBTREEI_MAX_STACK];                                   \
    unsigned int cpt;                                                   \
  } tree_it_t[1];                                                       \
                                                                        \
  typedef type M_C(name, _type_t);					\
                                                                        \
  M_IF_METHOD(MEMPOOL, oplist)(                                         \
    MEMPOOL_DEF(M_C(name, _mempool), node_t)                            \
    M_GET_MEMPOOL_LINKAGE oplist M_C(name, _mempool_t) M_GET_MEMPOOL oplist; \
    static inline node_t *M_C(name,_int_new)(void) {			\
      return M_C(name, _mempool_alloc)(M_GET_MEMPOOL oplist);		\
    }                                                                   \
    static inline void M_C(name,_int_del)(node_t *ptr) {		\
      M_C(name, _mempool_free)(M_GET_MEMPOOL oplist, ptr);		\
    }                                                                   \
									\
    , /* No mempool allocation */                                       \
									\
    static inline node_t *M_C(name,_int_new)(void) {			\
      return M_CALL_NEW(oplist, node_t);                                \
    }                                                                   \
    static inline void M_C(name,_int_del)(node_t *ptr) {		\
      M_CALL_DEL(oplist, ptr);                                          \
    }                                                                 ) \
									\
    RBTREEI_DEF3(name, type, oplist, tree_t, node_t, tree_it_t)

#define RBTREEI_DEF3(name, type, oplist, tree_t, node_t, tree_it_t)     \
  									\
  static inline void                                                    \
  M_C(name, _init)(tree_t tree)						\
  {                                                                     \
    assert (tree != NULL);                                              \
    tree->size = 0;                                                     \
    tree->node = NULL;                                                  \
    RBTREEI_CONTRACT(tree);                                             \
  }                                                                     \
                                                                        \
  static inline void                                                    \
  M_C(name, _clean)(tree_t tree)                                        \
  {                                                                     \
    RBTREEI_CONTRACT(tree);                                             \
    node_t *stack[RBTREEI_MAX_STACK];                                   \
    unsigned int cpt = 0;                                               \
    if (tree->node == NULL) return;                                     \
    stack[cpt++] = tree->node;                                          \
    while (cpt > 0) {                                                   \
      node_t *n = stack[cpt-1];                                         \
      while (true) {                                                    \
        RBTREEI_CONTRACT_NODE(n);                                       \
        if (n->child[0] != NULL) {                                      \
          assert (cpt < RBTREEI_MAX_STACK);                             \
          stack[cpt++] = n->child[0];                                   \
          n = n->child[0];                                              \
          stack[cpt-2]->child[0] = NULL;                                \
        } else if (n->child[1] != NULL) {                               \
          assert (cpt < RBTREEI_MAX_STACK);                             \
          stack[cpt++] = n->child[1];                                   \
          n = n->child[1];                                              \
          stack[cpt-2]->child[1] = NULL;                                \
        } else                                                          \
          break;                                                        \
      }                                                                 \
      assert (n == stack[cpt - 1]);                                     \
      M_CALL_CLEAR(oplist, n->data);                                    \
      M_C(name,_int_del) (n);						\
      assert((stack[cpt-1] = NULL) == NULL);                            \
      cpt--;                                                            \
    }                                                                   \
    tree->node = NULL;                                                  \
    tree->size = 0;                                                     \
   }                                                                    \
                                                                        \
  static inline void                                                    \
  M_C(name, _clear)(tree_t tree)					\
  {                                                                     \
    M_C(name, _clean)(tree);						\
  }                                                                     \
                                                                        \
  static inline void                                                    \
  M_C(name, _push)(tree_t tree, type const data)			\
  {                                                                     \
    RBTREEI_CONTRACT(tree);                                             \
    node_t *tab[RBTREEI_MAX_STACK];                                     \
    char which[RBTREEI_MAX_STACK];                                      \
    unsigned int cpt = 0;                                               \
    node_t *n = tree->node;                                             \
    /* If nothing, create new node */                                   \
    if (n == NULL) {                                                    \
      n = M_C(name,_int_new)();						\
      if (M_UNLIKELY (n == NULL)) {                                     \
        M_MEMORY_FULL(sizeof (node_t));                                 \
        return;                                                         \
      }                                                                 \
      M_CALL_INIT_SET(oplist, n->data, data);                           \
      n->child[0] = n->child[1] = NULL;                                 \
      RBTREEI_SET_BLACK (n);                                            \
      tree->node = n;                                                   \
      assert(tree->size == 0);                                          \
      tree->size = 1;                                                   \
      RBTREEI_CONTRACT(tree);                                           \
      return;                                                           \
    }                                                                   \
    /* Search for insertion point */                                    \
    tab[cpt] = n;                                                       \
    while (n != NULL) {                                                 \
      RBTREEI_CONTRACT_NODE(n);                                         \
      int cmp = M_CALL_CMP(oplist, n->data, data);                      \
      if (cmp == 0) {                                                   \
        break;                                                          \
      } else if (cmp > 0) {                                             \
        which[cpt++] = 0;                                               \
        n = n->child[0];                                                \
      } else {                                                          \
        which[cpt++] = 1;                                               \
        n = n->child[1];                                                \
      }                                                                 \
      assert (cpt < RBTREEI_MAX_STACK);                                 \
      tab[cpt] = n;                                                     \
    }                                                                   \
    /* If found, update the data (default is set) */                    \
    if (n != NULL) {                                                    \
      M_IF_METHOD (UPDATE, oplist)(M_CALL_UPDATE(oplist, n->data, data) \
                                   , M_CALL_SET(oplist, n->data, data)); \
      RBTREEI_CONTRACT (tree);                                          \
      return;                                                           \
    }                                                                   \
    /* Create new */                                                    \
    n = M_C(name,_int_new)();						\
    if (M_UNLIKELY (n == NULL) ) {                                      \
      M_MEMORY_FULL (sizeof (node_t));                                  \
      return;                                                           \
    }                                                                   \
    M_CALL_INIT_SET(oplist, n->data, data);                             \
    n->child[0] = n->child[1] = NULL;                                   \
    RBTREEI_SET_RED (n);                                                \
    assert (tab[cpt] == NULL);                                          \
    tab[cpt] = n;                                                       \
    tree->size ++;                                                      \
    /* Add it in the tree */                                            \
    assert(tab[cpt-1]->child[0+which[cpt-1]] == NULL);                  \
    tab[cpt-1]->child[0+which[cpt-1]] = n;                              \
    /* Fix the tree */                                                  \
    while (cpt >= 2                                                     \
           && RBTREEI_IS_RED(tab[cpt-1])                                \
           && tab[cpt-2]->child[1-which[cpt-2]] != NULL                 \
           && RBTREEI_IS_RED(tab[cpt-2]->child[1-which[cpt-2]])) {      \
      RBTREEI_SET_BLACK(tab[cpt-1]);                                    \
      RBTREEI_SET_BLACK(tab[cpt-2]->child[1-which[cpt-2]]);             \
      RBTREEI_SET_RED(tab[cpt-2]);                                      \
      cpt-=2;                                                           \
    }                                                                   \
    /* root is always black */                                          \
    RBTREEI_SET_BLACK(tab[0]);                                          \
    if (cpt <= 1 || RBTREEI_IS_BLACK(tab[cpt-1])) {                     \
      RBTREEI_CONTRACT (tree);                                          \
      return;                                                           \
    }                                                                   \
    /* Read the grand-parent, the parent and the element */             \
    node_t *pp = tab[cpt-2];                                            \
    node_t *p  = tab[cpt-1];                                            \
    node_t *x  = tab[cpt];                                              \
    int i      = which[cpt-2];                                          \
    int j      = 1 - i;                                                 \
    assert (i == 0 || i == 1);                                          \
    /* We need to do some rotations */                                  \
    if (i == which[cpt-1]) {                                            \
      /* The child is the left child of its parent */                   \
      /* OR The child is the right child of its parent */               \
      /* Right rotation: cpt is the new grand-parent.                   \
         x is its left child, the grand-parent is the right one */      \
      pp->child[i] = p->child[j];                                       \
      p->child[i] = x;                                                  \
      p->child[j] = pp;                                                 \
      RBTREEI_SET_BLACK(p);                                             \
      RBTREEI_SET_RED(pp);                                              \
    } else {                                                            \
      assert (j == which[cpt-1]);                                       \
      /* The child is the right child of its parent */                  \
      /* OR The child is the left child of its parent */                \
      /* Left rotation */                                               \
      pp->child[i] = x->child[j];                                       \
      p->child[j]  = x->child[i];                                       \
      x->child[i]  = p;                                                 \
      x->child[j]  = pp;                                                \
      RBTREEI_SET_BLACK(x);                                             \
      RBTREEI_SET_RED(p);                                               \
      RBTREEI_SET_RED(pp);                                              \
      p = x;                                                            \
    }                                                                   \
    /* Insert the new grand parent */                                   \
    if (cpt == 2) {                                                     \
      tree->node = p;                                                   \
    } else {                                                            \
      assert (cpt >= 3);                                                \
      tab[cpt-3]->child[(int) which[cpt-3]] = p;                        \
    }                                                                   \
    /* Done */                                                          \
    RBTREEI_CONTRACT (tree);                                            \
  }                                                                     \
                                                                        \
  static inline size_t                                                  \
  M_C(name, _size)(const tree_t tree)					\
  {                                                                     \
    RBTREEI_CONTRACT (tree);                                            \
    return tree->size;                                                  \
  }                                                                     \
                                                                        \
  static inline void                                                    \
  M_C(name, _int_it)(tree_it_t it, const tree_t tree, int child)	\
  {                                                                     \
    RBTREEI_CONTRACT (tree);                                            \
    assert (it != NULL);                                                \
    assert (child == 0 || child == 1);                                  \
    unsigned int cpt = 0;                                               \
    if (tree->node != NULL) {                                           \
      it->which[cpt] = child;                                           \
      node_t *n = it->stack[cpt++] = tree->node;                        \
      while (n->child[child] != NULL) {                                 \
        assert (cpt < RBTREEI_MAX_STACK);                               \
        n = n->child[child];                                            \
        it->which[cpt] = child;                                         \
        it->stack[cpt++] = n;                                           \
      }                                                                 \
      assert (n == it->stack[cpt - 1]);                                 \
    }                                                                   \
    it->cpt = cpt;                                                      \
  }                                                                     \
                                                                        \
  static inline void                                                    \
  M_C(name, _it)(tree_it_t it, const tree_t tree)			\
  {                                                                     \
    M_C(name, _int_it)(it, tree, 0);					\
  }                                                                     \
                                                                        \
  static inline void                                                    \
  M_C(name, _it_last)(tree_it_t it, const tree_t tree)			\
  {                                                                     \
    M_C(name,_int_it)(it, tree, 1);					\
  }                                                                     \
                                                                        \
  static inline void                                                    \
  M_C(name, _it_end)(tree_it_t it, const tree_t tree)			\
  {                                                                     \
    RBTREEI_CONTRACT (tree);                                            \
    assert (it != NULL);						\
    it->cpt = 0;                                                        \
  }                                                                     \
                                                                        \
  static inline void                                                    \
  M_C(name, _it_set)(tree_it_t it, const tree_it_t ref)			\
  {                                                                     \
    assert (it != NULL && ref != NULL);                                 \
    *it = *ref;                                                         \
  }                                                                     \
                                                                        \
  static inline bool                                                    \
  M_C(name, _end_p)(const tree_it_t it)					\
  {                                                                     \
    assert (it != NULL);                                                \
    return it->cpt == 0;                                                \
  }                                                                     \
                                                                        \
  static inline void                                                    \
  M_C(name, _int_next)(tree_it_t it, int child)				\
  {                                                                     \
    assert (it != NULL);                                                \
    assert (child == 0 || child == 1);                                  \
    if (it->cpt == 0) return;                                           \
    unsigned int cpt = it->cpt - 1;                                     \
    node_t *n = it->stack[cpt];                                         \
    const int right = 1 ^ child;                                        \
    if (n->child[right] != NULL && it->which[cpt] == child) {           \
      /* Going right */                                                 \
      assert (cpt < RBTREEI_MAX_STACK);                                 \
      n = n->child[right];                                              \
      it->which[cpt++] = right;                                         \
      it->stack[cpt] = n;                                               \
      it->which[cpt++] = child;                                         \
      /* Going left */                                                  \
      while (n->child[child] != NULL) {                                 \
        assert (cpt < RBTREEI_MAX_STACK);                               \
        n = n->child[child];                                            \
        it->which[cpt] = child;                                         \
        it->stack[cpt++] = n;                                           \
      }                                                                 \
      assert (n == it->stack[cpt - 1]);                                 \
    } else {                                                            \
      /* Going up */                                                    \
      while (cpt > 0 && it->which[cpt-1] == right) cpt--;               \
    }                                                                   \
    it->cpt = cpt;                                                      \
  }                                                                     \
                                                                        \
  static inline void                                                    \
  M_C(name, _next)(tree_it_t it)					\
  {                                                                     \
    M_C(name, _int_next)(it, 0);					\
  }                                                                     \
                                                                        \
  static inline void                                                    \
  M_C(name, _previous)(tree_it_t it)					\
  {                                                                     \
    M_C(name, _int_next)(it, 1);					\
  }                                                                     \
                                                                        \
  static inline type *                                                  \
  M_C(name, _ref)(const tree_it_t it)					\
  {                                                                     \
    assert(it != NULL && it->cpt > 0);                                  \
    /* NOTE: partially unsafe if the user modify the order of the el */ \
    return &(it->stack[it->cpt-1]->data);                               \
  }                                                                     \
                                                                        \
  static inline type const *                                            \
  M_C(name, _cref)(const tree_it_t it)					\
  {                                                                     \
    return M_CONST_CAST(type, M_C(name, _ref)(it));			\
  }                                                                     \
                                                                        \
  static inline bool                                                    \
  M_C(name, _it_equal_p)(const tree_it_t it1,				\
			 const tree_it_t it2)				\
  {                                                                     \
    return it1->cpt == it2->cpt                                         \
      && it1->stack[it1->cpt-1] == it2->stack[it2->cpt-1];              \
  }                                                                     \
                                                                        \
                                                                        \
  static inline void                                                    \
  M_C(name, _it_from)(tree_it_t it,					\
		      const tree_t tree, type const data)		\
  {                                                                     \
    RBTREEI_CONTRACT (tree);                                            \
    assert (it != NULL);                                                \
    unsigned int cpt = 0;                                               \
    node_t *n = tree->node;                                             \
    it->stack[cpt] =  n;                                                \
    while (n != NULL) {                                                 \
      int cmp = M_CALL_CMP(oplist, n->data, data);                      \
      if (cmp == 0)                                                     \
        break;                                                          \
      int child = (cmp < 0);                                            \
      it->which[cpt++] = child;                                         \
      n = n->child[child];                                              \
      assert (cpt < RBTREEI_MAX_STACK);                                 \
      it->stack[cpt] =  n;                                              \
    }                                                                   \
    it->cpt = cpt;                                                      \
  }                                                                     \
                                                                        \
  static inline bool                                                    \
  M_C(name, _it_to_p)(tree_it_t it, type const data)			\
  {                                                                     \
    assert (it != NULL);                                                \
    if (it->cpt == 0) return true;                                      \
    assert (it->cpt > 0 && it->cpt < RBTREEI_MAX_STACK);                \
    node_t *n = it->stack[it->cpt-1];                                   \
    int cmp = M_CALL_CMP(oplist, n->data, data);                        \
    return (cmp >= 0);                                                  \
  }                                                                     \
                                                                        \
  static inline type *                                                  \
  M_C(name, _min)(const tree_t tree)					\
  {                                                                     \
    RBTREEI_CONTRACT (tree);                                            \
    node_t *n = tree->node;                                             \
    if (M_UNLIKELY (n == NULL) ) return NULL;                           \
    while (n->child[0] != NULL) {                                       \
      RBTREEI_CONTRACT_NODE (n);                                        \
      n = n->child[0];                                                  \
    }                                                                   \
    return &n->data;                                                    \
  }                                                                     \
  									\
  static inline type *                                                  \
  M_C(name, _max)(const tree_t tree)					\
  {                                                                     \
    RBTREEI_CONTRACT (tree);                                            \
    node_t *n = tree->node;                                             \
    if (M_UNLIKELY (n == NULL) ) return NULL;                           \
    while (n->child[1] != NULL) {                                       \
      RBTREEI_CONTRACT_NODE (n);                                        \
      n = n->child[1];                                                  \
    }                                                                   \
    return &n->data;                                                    \
  }                                                                     \
  									\
  static inline type const *                                            \
  M_C(name, _cmin)(const tree_t tree)					\
  {                                                                     \
    return M_CONST_CAST(type, M_C(name, _min)(tree));			\
  }                                                                     \
  									\
  static inline type const *                                            \
  M_C(name, _cmax)(const tree_t tree)					\
  {                                                                     \
    return M_CONST_CAST(type, M_C(name, _max)(tree));			\
  }                                                                     \
  									\
  static inline type *                                                  \
  M_C(name, _get)(const tree_t tree, type const data)			\
  {                                                                     \
    RBTREEI_CONTRACT (tree);                                            \
    node_t *n = tree->node;                                             \
    while (n != NULL) {                                                 \
      RBTREEI_CONTRACT_NODE (n);                                        \
      int cmp = M_CALL_CMP(oplist, n->data, data);                      \
      if (cmp == 0) {                                                   \
        return &n->data;                                                \
      } else if (cmp > 0) {                                             \
        n = n->child[0];                                                \
      } else {                                                          \
        n = n->child[1];                                                \
      }                                                                 \
    }                                                                   \
    return NULL;                                                        \
  }                                                                     \
                                                                        \
  static inline type const *                                            \
  M_C(name, _cget)(const tree_t tree, type const data)			\
  {                                                                     \
    return M_CONST_CAST(type, M_C(name, _get)(tree, data));		\
  }                                                                     \
  									\
  static inline node_t *                                                \
  M_C(name, _int_copyn)(const node_t *o)				\
  {                                                                     \
    if (o == NULL) return NULL;                                         \
    node_t *n = M_C(name,_int_new)();					\
    if (M_UNLIKELY (n == NULL) ) {                                      \
      M_MEMORY_FULL (sizeof (node_t));                                  \
      return NULL;                                                      \
    }                                                                   \
    M_CALL_INIT_SET(oplist, n->data, o->data);                          \
    n->child[0] = M_C(name, _int_copyn)(o->child[0]);			\
    n->child[1] = M_C(name, _int_copyn)(o->child[1]);			\
    RBTREEI_COPY_COLOR (n, o);                                          \
    return n;                                                           \
  }                                                                     \
  									\
  static inline void                                                    \
  M_C(name, _init_set)(tree_t tree, const tree_t ref)			\
  {                                                                     \
    RBTREEI_CONTRACT (ref);                                             \
    assert (tree != NULL && tree != ref);                               \
    tree->size = ref->size;                                             \
    tree->node = M_C(name, _int_copyn)(ref->node);			\
    RBTREEI_CONTRACT (tree);                                            \
  }                                                                     \
                                                                        \
  static inline void                                                    \
  M_C(name, _set)(tree_t tree, const tree_t ref)			\
  {                                                                     \
    RBTREEI_CONTRACT (tree);                                            \
    RBTREEI_CONTRACT (ref);                                             \
    if (tree == ref) return;                                            \
    M_C(name,_clear)(tree);						\
    M_C(name,_init_set)(tree, ref);					\
  }                                                                     \
                                                                        \
  static inline void                                                    \
  M_C(name, _init_move)(tree_t tree, tree_t ref)			\
  {                                                                     \
    RBTREEI_CONTRACT (ref);                                             \
    assert (tree != NULL && tree != ref);                               \
    tree->size = ref->size;                                             \
    tree->node = ref->node;                                             \
    ref->node = NULL;                                                   \
    ref->size = 0;                                                      \
    RBTREEI_CONTRACT (tree);                                            \
  }                                                                     \
                                                                        \
  static inline void                                                    \
  M_C(name, _move)(tree_t tree, tree_t ref)				\
  {                                                                     \
    RBTREEI_CONTRACT (tree);                                            \
    RBTREEI_CONTRACT (ref);                                             \
    assert (tree != ref);                                               \
    M_C(name,_clear)(tree);						\
    M_C(name,_init_move)(tree, ref);					\
    RBTREEI_CONTRACT (tree);                                            \
  }                                                                     \
  									\
  static inline void                                                    \
  M_C(name, _swap)(tree_t tree1, tree_t tree2)				\
  {                                                                     \
    RBTREEI_CONTRACT (tree1);                                           \
    RBTREEI_CONTRACT (tree2);                                           \
    M_SWAP(size_t, tree1->size, tree2->size);                           \
    M_SWAP(node_t *, tree1->node, tree2->node);                         \
    RBTREEI_CONTRACT (tree1);                                           \
    RBTREEI_CONTRACT (tree2);                                           \
  }                                                                     \
                                                                        \
  static inline bool                                                    \
  M_C(name, _empty_p)(const tree_t tree)				\
  {                                                                     \
    RBTREEI_CONTRACT (tree);                                            \
    return tree->size == 0;                                             \
  }                                                                     \
                                                                        \
  /* Take care of the case n == NULL too */                              \
  static inline bool                                                    \
  M_C(name, _int_is_black)(const node_t *n)				\
  {                                                                     \
    return (n == NULL) ? true : RBTREEI_IS_BLACK(n);                    \
  }                                                                     \
                                                                        \
  static inline void                                                    \
  M_C(name, _int_set_black)(node_t *n)                                  \
  {                                                                     \
    if (n != NULL) RBTREEI_SET_BLACK(n);                                \
  }                                                                     \
                                                                        \
  static inline node_t *                                                \
  M_C(name, _int_rotate)(node_t *pp, node_t *ppp, const bool right)	\
  {                                                                     \
    assert (pp != NULL && ppp != NULL);                                 \
    bool left = !right;                                                 \
    node_t *p = pp->child[right];                                       \
    assert (p != NULL);                                                 \
    pp->child[right] = p->child[left];                                  \
    p->child[left]  = pp;                                               \
    /* Fix grandparent with new parent */                               \
    assert(ppp->child[0] == pp || ppp->child[1] == pp);                 \
    ppp->child[(ppp->child[0] != pp)] = p;                              \
    return p;                                                           \
  }                                                                     \
                                                                        \
  M_IF_DEBUG(                                                           \
  static size_t                                                         \
  M_C(name, _int_depth)(const node_t *n)				\
  {                                                                     \
    if (n == NULL) return 1;                                            \
    return RBTREEI_IS_BLACK (n)                                         \
      + M_C(name, _int_depth)(n->child[0]);				\
  }                                                                     \
  )                                                                     \
                                                                        \
  static inline bool                                                    \
  M_C(name, _pop_at)(type *data_ptr, tree_t tree, type const key)       \
  {                                                                     \
    RBTREEI_CONTRACT (tree);                                            \
    node_t *tab[RBTREEI_MAX_STACK];                                     \
    unsigned char which[RBTREEI_MAX_STACK];                             \
    unsigned int cpt = 0;                                               \
    node_t *n = tree->node;                                             \
    which[0] = 0;                                                       \
    tab[cpt++] = (node_t*) &tree->node; /* FIXME: To clean! */          \
    /* Search for the deletion point */                                 \
    tab[cpt] = n;                                                       \
    while (n != NULL) {                                                 \
      RBTREEI_CONTRACT_NODE (n);                                        \
      assert(M_C(name, _int_depth)(n->child[0])				\
             == M_C(name, _int_depth)(n->child[1]));			\
      int cmp = M_CALL_CMP(oplist, n->data, key);                       \
      if (cmp == 0) {                                                   \
        break;                                                          \
      }                                                                 \
      int i = (cmp < 0);                                                \
      which[cpt++] = i;                                                 \
      n = n->child[i];                                                  \
      assert (cpt < RBTREEI_MAX_STACK);                                 \
      tab[cpt] = n;                                                     \
    }                                                                   \
    assert (tab[cpt] == n);                                             \
    /* If not found, fail */                                            \
    if (n == NULL) {                                                    \
      return false;                                                     \
    }                                                                   \
    unsigned int cpt_n = cpt;                                           \
    node_t *v = n;     /* the replacement node */                       \
    node_t *u;         /* the deleted node */                           \
    rbtreei_color_e v_color = RBTREEI_GET_COLOR(v);                     \
    /* Classical removal of a node from a binary tree */                \
    if (v->child[0] != NULL && v->child[1] != NULL) {                   \
      /* node has 2 child. */                                           \
      /* Get the element right next to the deleted one */               \
      v = v->child[1];                                                  \
      which[cpt++] = 1;                                                 \
      tab[cpt] = v;                                                     \
      while (v != NULL) {                                               \
        /* Always left node */                                          \
        RBTREEI_CONTRACT_NODE (v);                                      \
        assert(M_C(name, _int_depth)(v->child[0])			\
               == M_C(name, _int_depth)(v->child[1]));			\
        which[cpt++] = 0;                                               \
        v = v->child[0];                                                \
        assert (cpt < RBTREEI_MAX_STACK);                               \
        tab[cpt] = v;                                                   \
      }                                                                 \
      /* Pop the last element to get the last non-null element */       \
      v = tab[--cpt];                                                   \
      assert (v != NULL);                                               \
      u = v->child[1];                                                  \
      /* Replace 'v' by 'u' in the tree */                              \
      assert(tab[cpt-1]->child[which[cpt-1]] == v);                     \
      tab[cpt-1]->child[which[cpt-1]] = u;                              \
      /* Replace 'n' by 'v' in the tree */                              \
      assert(tab[cpt_n-1] != NULL);                                     \
      assert(tab[cpt_n-1]->child[which[cpt_n-1]] == n);                 \
      tab[cpt_n-1]->child[which[cpt_n-1]] = v;                          \
      v->child[0] = n->child[0];                                        \
      v->child[1] = n->child[1];                                        \
      v_color = RBTREEI_GET_COLOR(v);                                   \
      RBTREEI_COPY_COLOR(v, n);                                         \
      tab[cpt_n] = v;                                                   \
      /* For the algorithm, 'u' is now the deleted node */              \
    } else {                                                            \
      /* 1 or no child to the node. Replace the element */              \
      v = n;                                                            \
      u = v->child[(n->child[0] == NULL)];                              \
      assert (cpt_n >= 1 && tab[cpt_n-1]->child[which[cpt_n-1]] == n);  \
      assert (n->child[(n->child[0] != NULL)] == NULL);                 \
      tab[cpt_n-1]->child[which[cpt_n-1]] = u;                          \
      /* in all cases, this node shall be set to black */               \
    }                                                                   \
                                                                        \
    /* Rebalance from child to root */                                  \
    if (v_color == RBTREE_BLACK                                         \
        && M_C(name, _int_is_black)(u)) {                               \
      /* tab[0] is NULL, tab[1] is root, u is double black */           \
      node_t *p = u, *s;                                                \
      while (cpt >= 2) {                                                \
        p  = tab[--cpt];                                                \
        bool nbChild = which[cpt];                                      \
        assert (p != NULL && u == p->child[nbChild]);                   \
        s = p->child[!nbChild];                                         \
        /* if sibling is red, perform a rotation to move sibling up */  \
        if (!M_C(name, _int_is_black)(s)) {                             \
          p = M_C(name, _int_rotate) (p, tab[cpt-1], !nbChild);		\
          RBTREEI_SET_BLACK(p); /* was sibling */                       \
          tab[cpt] = p;                                                 \
          which[cpt++] = nbChild;                                       \
          p = p->child[nbChild];  /* was parent */                      \
          assert (p != NULL);                                           \
          RBTREEI_SET_RED(p);                                           \
          s = p->child[!nbChild];                                       \
          assert (M_C(name, _int_is_black)(s));                         \
        }                                                               \
        assert (p != NULL && u == p->child[nbChild]);                   \
        /* if both childreen of s are black */                          \
        /* perform recoloring and recur on parent if black */           \
        if (s != NULL                                                   \
            && M_C(name, _int_is_black)(s->child[0])                    \
            && M_C(name, _int_is_black)(s->child[1])) {                 \
          assert(M_C(name, _int_depth)(s->child[0]) == M_C(name, _int_depth)(s->child[1])); \
          RBTREEI_SET_RED(s);                                           \
          if (RBTREEI_IS_RED(p)) {                                      \
            RBTREEI_SET_BLACK(p);                                       \
            RBTREEI_CONTRACT_NODE(p);                                   \
            assert(M_C(name, _int_depth)(p->child[0]) == M_C(name, _int_depth)(p->child[1])); \
            break;                                                      \
          }                                                             \
          u = p;                                                        \
        } else  {                                                       \
          assert (s != NULL);                                           \
          /* at least one child of 's' is red */                        \
          /* perform rotation(s) */                                     \
          bool childIsRight =  !M_C(name, _int_is_black)(s->child[1]);  \
          rbtreei_color_e p_color = RBTREEI_GET_COLOR (p);              \
          if (childIsRight != nbChild) {                                \
            /* left-left or right-right case */                         \
            p = M_C(name, _int_rotate) (p, tab[cpt-1], childIsRight);	\
          } else {                                                      \
            s = M_C(name, _int_rotate) (s, p, childIsRight);		\
            p = M_C(name, _int_rotate) (p, tab[cpt-1], !nbChild);	\
          }                                                             \
          RBTREEI_SET_COLOR(p, p_color);                                \
          assert(p->child[0] != NULL && p->child[1] != NULL);           \
          RBTREEI_SET_BLACK(p->child[0]);                               \
          RBTREEI_SET_BLACK(p->child[1]);                               \
          RBTREEI_CONTRACT_NODE(p);                                     \
          assert(M_C(name, _int_depth)(p->child[0]) == M_C(name, _int_depth)(p->child[1])); \
          break;                                                        \
        }                                                               \
      } /* while */                                                     \
      if (cpt == 1 /* root has been reached? */ ) {                     \
        M_C(name, _int_set_black)(p);                                   \
        assert (tree->node == p);                                       \
      }                                                                 \
    } else {                                                            \
      M_C(name, _int_set_black)(u);                                     \
    }                                                                   \
    assert (tree->node == NULL || RBTREEI_IS_BLACK(tree->node));        \
    /* delete it */                                                     \
    if (data_ptr != NULL)                                               \
      M_DO_MOVE(oplist, *data_ptr, n->data);                            \
    else                                                                \
      M_CALL_CLEAR(oplist, n->data);                                    \
    M_C(name,_int_del) (n);						\
    tree->size --;                                                      \
    RBTREEI_CONTRACT (tree);                                            \
    return true;                                                        \
  }                                                                     \
                                                                        \
  M_IF_METHOD(EQUAL, oplist)(                                           \
  static inline bool M_C(name,_equal_p)(const tree_t t1, const tree_t t2) { \
    RBTREEI_CONTRACT(t1);                                               \
    RBTREEI_CONTRACT(t2);                                               \
    if (t1->size != t2->size) return false;                             \
    tree_it_t it1;                                                      \
    tree_it_t it2;                                                      \
    /* NOTE: We can't compare two tree directly as they can be          \
       structuraly different but functionnaly equal (you get this by    \
       constructing the tree in a different way). We have to            \
       compare the ordered value within the tree. */                    \
    M_C(name, _it)(it1, t1);						\
    M_C(name, _it)(it2, t2);						\
    while (!M_C(name, _end_p)(it1)					\
           && !M_C(name, _end_p)(it2)) {				\
      type const *ref1 = M_C(name, _cref)(it1);				\
      type const *ref2 = M_C(name, _cref)(it2);				\
      if (M_CALL_EQUAL(oplist, *ref1, *ref2) == false)                  \
        return false;                                                   \
      M_C(name, _next)(it1);						\
      M_C(name, _next)(it2);						\
    }                                                                   \
    return M_C(name, _end_p)(it1)					\
      && M_C(name, _end_p)(it2);					\
  }                                                                     \
  , /* NO EQUAL METHOD */ )                                             \
                            						\
  M_IF_METHOD(HASH, oplist)(                                            \
  static inline size_t M_C(name,_hash)(const tree_t t1) {               \
    RBTREEI_CONTRACT(t1);                                               \
    M_HASH_DECL(hash);                                                  \
    /* NOTE: We can't compute the hash directly for the same reason     \
       than for EQUAL operator. */                                      \
    tree_it_t it1;                                                      \
    M_C(name, _it)(it1, t1);						\
    while (!M_C(name, _end_p)(it1)) {					\
      type const *ref1 = M_C(name, _cref)(it1);				\
      M_HASH_UP(hash, M_CALL_HASH(oplist, *ref1));                      \
      M_C(name, _next)(it1);						\
    }                                                                   \
    return M_HASH_FINAL (hash);						\
  }                                                                     \
  , /* NO HASH METHOD */ )                                              \
			   						\
  M_IF_METHOD(GET_STR, oplist)(                                         \
  static inline void M_C(name, _get_str)(string_t str,                  \
					 tree_t const t1, bool append) { \
    RBTREEI_CONTRACT(t1);                                               \
    assert(str != NULL);                                                \
    (append ? string_cat_str : string_set_str) (str, "[");              \
    /* NOTE: The print is really naive, and not really efficient */     \
    bool commaToPrint = false;                                          \
    tree_it_t it1;                                                      \
    M_C(name, _it)(it1, t1);						\
    while (!M_C(name, _end_p)(it1)) {					\
      if (commaToPrint)                                                 \
        string_push_back (str, M_GET_SEPARATOR oplist);                 \
      commaToPrint = true;                                              \
      type const *ref1 = M_C(name, _cref)(it1);				\
      M_CALL_GET_STR(oplist, str, *ref1, true);                         \
      M_C(name, _next)(it1);						\
    }                                                                   \
    string_push_back (str, ']');                                        \
  }                                                                     \
  , /* NO GET_STR */ )                                                  \
			      						\
  M_IF_METHOD(OUT_STR, oplist)(                                         \
  static inline void                                                    \
  M_C(name, _out_str)(FILE *file, tree_t const rbtree)			\
  {                                                                     \
    RBTREEI_CONTRACT(rbtree);                                           \
    assert (file != NULL);                                              \
    fputc ('[', file);							\
    tree_it_t it;                                                       \
    bool commaToPrint = false;                                          \
    for (M_C(name, _it)(it, rbtree) ;					\
         !M_C(name, _end_p)(it);					\
         M_C(name, _next)(it)){						\
      if (commaToPrint)                                                 \
        fputc (M_GET_SEPARATOR oplist, file);                           \
      commaToPrint = true;                                              \
      type const *item = M_C(name, _cref)(it);				\
      M_CALL_OUT_STR(oplist, file, *item);                              \
    }                                                                   \
    fputc (']', file);							\
  }                                                                     \
  , /* no out_str */ )                                                  \
                                                                        \
  M_IF_METHOD(PARSE_STR, oplist)(                                       \
  static inline bool                                                    \
  M_C(name, _parse_str)(tree_t rbtree, const char str[], const char **endp) \
  {                                                                     \
    RBTREEI_CONTRACT(rbtree);                                           \
    assert (str != NULL);                                               \
    M_C(name,_clean)(rbtree);						\
    bool success = false;                                               \
    int c = *str++;                                                     \
    if (M_UNLIKELY (c != '[')) goto exit;                               \
    c = *str++;                                                         \
    if (M_UNLIKELY (c == ']')) { success = true; goto exit; }           \
    if (M_UNLIKELY (c == 0)) goto exit;                                 \
    str--;                                                              \
    type item;                                                          \
    M_CALL_INIT(oplist, item);                                          \
    do {                                                                \
      bool b = M_CALL_PARSE_STR(oplist, item, str, &str);               \
      do { c = *str++; } while (isspace(c));                            \
      if (b == false || c == 0) goto exit;                              \
      M_C(name, _push)(rbtree, item);					\
    } while (c == M_GET_SEPARATOR oplist);				\
    M_CALL_CLEAR(oplist, item);                                         \
    success = (c == ']');                                               \
  exit:                                                                 \
    if (endp) *endp = str;                                              \
    return success;                                                     \
  }                                                                     \
  , /* no parse_str */ )                                                \
                                                                        \
  M_IF_METHOD(IN_STR, oplist)(                                          \
  static inline bool                                                    \
  M_C(name, _in_str)(tree_t rbtree, FILE *file)                         \
  {                                                                     \
    RBTREEI_CONTRACT(rbtree);                                           \
    assert (file != NULL);                                              \
    M_C(name,_clean)(rbtree);						\
    int c = fgetc(file);						\
    if (M_UNLIKELY (c != '[')) return false;                            \
    c = fgetc(file);                                                    \
    if (M_UNLIKELY (c == ']')) return true;                             \
    if (M_UNLIKELY (c == EOF)) return false;                            \
    ungetc(c, file);                                                    \
    type item;                                                          \
    M_CALL_INIT(oplist, item);                                          \
    do {                                                                \
      bool b = M_CALL_IN_STR(oplist, item, file);                      \
      do { c = fgetc(file); } while (isspace(c));                       \
      if (b == false || c == EOF) break;				\
      M_C(name, _push)(rbtree, item);					\
    } while (c == M_GET_SEPARATOR oplist);				\
    M_CALL_CLEAR(oplist, item);                                         \
    return c == ']';                                                    \
  }                                                                     \
  , /* no in_str */ )                                                   \
			     						\

// TODO: specialized _sort shall do nothing, but shall check the requested order. How ?


#endif
