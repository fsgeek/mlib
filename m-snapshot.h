/*
 * M*LIB - SNAPSHOT Module
 *
 * Copyright (c) 2017, Patrick Pelissier
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
#ifndef __M_SNAPSHOT
#define __M_SNAPSHOT

#include <assert.h>

#include "m-atomic.h"
#include "m-core.h"

/* Define a snapshot and it function
   USAGE: SNAPSHOT_DEF(name, type[, oplist]) */
#define SNAPSHOT_DEF(name, ...)				       \
  M_IF_NARGS_EQ1(__VA_ARGS__)                                  \
  (SNAPSHOTI_DEF2(name, __VA_ARGS__, () ),		       \
   SNAPSHOTI_DEF2(name, __VA_ARGS__ ))

/* Define the oplist of a snapshot.
   USAGE: SNAPSHOT_OPLIST(name[, oplist]) */
#define SNAPSHOT_OPLIST(...)                                            \
  M_IF_NARGS_EQ1(__VA_ARGS__)                                           \
  (SNAPSHOTI_OPLIST(__VA_ARGS__, M_DEFAULT_OPLIST ),                    \
   SNAPSHOTI_OPLIST(__VA_ARGS__ ))


/********************************** INTERNAL ************************************/

/* Define the oplist of a snapshot */
#define SNAPSHOTI_OPLIST(name, oplist)					\
  (INIT(M_C(name, _init)),						\
   INIT_SET(M_C(name, _init_set)),					\
   SET(M_C(name, _set)),						\
   CLEAR(M_C(name, _clear)),						\
   TYPE(M_C(name, _t)),							\
   SUBTYPE(M_C(name_, type_t))						\
   ,OPLIST(oplist)                                                      \
   ,M_IF_METHOD(INIT_MOVE, oplist)(INIT_MOVE(M_C(name, _init_move)),)	\
   ,M_IF_METHOD(MOVE, oplist)(MOVE(M_C(name, _move)),)			\
   )

/* Flag defining the atomic state of a snapshot:
 * - r: Index of the read buffer Range [0..2]
 * - w: Index of the write buffer  Range [0..2]
 * - f: Next index of the write buffer when a shot is taken Range [0..2]
 * - b: Boolean indicating that the read buffer shall be updated
 */
#define SNAPSHOTI_FLAG(r, w, f, b)					\
  ((unsigned char)( ( (r) << 4) | ((w) << 2) | ((f)) | ((b) << 6)))
#define SNAPSHOTI_R(flags)			\
  (((flags) >> 4) & 0x03u)
#define SNAPSHOTI_W(flags)			\
  (((flags) >> 2) & 0x03u)
#define SNAPSHOTI_F(flags)			\
  (((flags) >> 0) & 0x03u)
#define SNAPSHOTI_B(flags)			\
  (((flags) >> 6) & 0x01u)

/* NOTE: Due to atomic_load only accepting non-const pointer,
   we can't have any const in the interface. */
#define SNAPSHOTI_FLAGS_CONTRACT(flags)					\
  assert(SNAPSHOTI_R(flags) != SNAPSHOTI_W(flags)                       \
	 && SNAPSHOTI_R(flags) != SNAPSHOTI_F(flags)			\
	 && SNAPSHOTI_W(flags) != SNAPSHOTI_F(flags))
#define SNAPSHOTI_CONTRACT(snap)	do {                            \
    assert((snap) != NULL);						\
    unsigned char f = atomic_load (&(snap)->flags);                     \
    SNAPSHOTI_FLAGS_CONTRACT(f);                                        \
  } while (0)

#define SNAPSHOTI_DEF2(name, type, oplist)				\
									\
  typedef struct M_C(name, _s) {					\
    type  data[3];							\
    atomic_uchar flags;                                                 \
  } M_C(name, _t)[1];							\
  typedef type M_C(name, _type_t);                                      \
                                                                        \
  static inline void M_C(name, _init)(M_C(name, _t) snap)               \
  {									\
    assert(snap != NULL);						\
    for(int i = 0; i < 3; i++) {					\
      M_GET_INIT oplist(snap->data[i]);					\
    }									\
    atomic_init (&snap->flags, SNAPSHOTI_FLAG(0, 1, 2, 0));		\
    SNAPSHOTI_CONTRACT(snap);						\
  }									\
  									\
  static inline void M_C(name, _clear)(M_C(name, _t) snap)		\
  {									\
    SNAPSHOTI_CONTRACT(snap);						\
    for(int i = 0; i < 3; i++) {					\
      M_GET_CLEAR oplist(snap->data[i]);				\
    }									\
  }									\
									\
  static inline void M_C(name, _init_set)(M_C(name, _t) snap,		\
					  M_C(name, _t) org)		\
  {									\
    SNAPSHOTI_CONTRACT(org);						\
    assert(snap != NULL && snap != org);				\
    for(int i = 0; i < 3; i++) {					\
      M_GET_INIT_SET oplist(snap->data[i], org->data[i]);		\
    }									\
    atomic_init (&snap->flags, atomic_load(&org->flags));		\
    SNAPSHOTI_CONTRACT(snap);						\
  }									\
  									\
  static inline void M_C(name, _set)(M_C(name, _t) snap,		\
				     M_C(name, _t) org)			\
  {									\
    SNAPSHOTI_CONTRACT(snap);						\
    SNAPSHOTI_CONTRACT(org);						\
    for(int i = 0; i < 3; i++) {					\
      M_GET_SET oplist(snap->data[i], org->data[i]);			\
    }									\
    atomic_init (&snap->flags, atomic_load(&org->flags));		\
    SNAPSHOTI_CONTRACT(snap);						\
  }									\
  									\
  M_IF_METHOD(INIT_MOVE, oplist)(					\
    static inline void M_C(name, _init_move)(M_C(name, _t) snap,        \
					     M_C(name, _t) org)		\
    {									\
      SNAPSHOTI_CONTRACT(org);						\
      assert(snap != NULL && snap != org);				\
      for(int i = 0; i < 3; i++) {					\
	M_GET_INIT_MOVE oplist(snap->data[i], org->data[i]);		\
      }									\
      snap->flags = org->flags;						\
      atomic_init (&org->flags, 0);					\
      SNAPSHOTI_CONTRACT(snap);						\
    }									\
    ,) /* IF_METHOD (INIT_MOVE) */					\
									\
   M_IF_METHOD(MOVE, oplist)(                                           \
     static inline void M_C(name, _move)(M_C(name, _t) snap,            \
					 M_C(name, _t) org)		\
     {									\
       SNAPSHOTI_CONTRACT(snap);					\
       SNAPSHOTI_CONTRACT(org);						\
       assert(snap != org);						\
       for(int i = 0; i < 3; i++) {					\
	 M_GET_MOVE oplist(snap->data[i], org->data[i]);		\
       }								\
       snap->flags = org->flags;					\
       atomic_init(&org->flags, 0);					\
       SNAPSHOTI_CONTRACT(snap);					\
     }									\
     ,) /* IF_METHOD (MOVE) */						\
									\
  static inline type *M_C(name, _take)(M_C(name, _t) snap)              \
  {									\
    SNAPSHOTI_CONTRACT(snap);						\
    unsigned char nextFlags, origFlags = atomic_load (&snap->flags);	\
    do {								\
      nextFlags = SNAPSHOTI_FLAG(SNAPSHOTI_R(origFlags),		\
				 SNAPSHOTI_F(origFlags),		\
				 SNAPSHOTI_W(origFlags), 1);		\
    } while (!atomic_compare_exchange_weak (&snap->flags, &origFlags,	\
					    nextFlags));		\
    return &snap->data[SNAPSHOTI_W(nextFlags)];				\
  }									\
  									\
  static inline const type *M_C(name, _look)(M_C(name, _t) snap)	\
  {									\
    SNAPSHOTI_CONTRACT(snap);						\
    unsigned char nextFlags, origFlags = atomic_load (&snap->flags);	\
    do {								\
      if (!SNAPSHOTI_B(origFlags))					\
	break;								\
      nextFlags = SNAPSHOTI_FLAG(SNAPSHOTI_F(origFlags),		\
				 SNAPSHOTI_W(origFlags),		\
				 SNAPSHOTI_R(origFlags), 0);		\
    } while (!atomic_compare_exchange_weak (&snap->flags, &origFlags,	\
					    nextFlags));		\
    nextFlags = atomic_load(&snap->flags);				\
    return M_CONST_CAST(type, &snap->data[SNAPSHOTI_R(nextFlags)]);	\
  }									\
  									\
  static inline type *M_C(name, _get_produced)(M_C(name, _t) snap)	\
  {									\
    SNAPSHOTI_CONTRACT(snap);						\
    unsigned char flags = atomic_load(&snap->flags);			\
    return &snap->data[SNAPSHOTI_W(flags)];				\
  }									\
  									\
  static inline const type *M_C(name, _get_consummed)(M_C(name, _t) snap) \
  {									\
    SNAPSHOTI_CONTRACT(snap);						\
    unsigned char flags = atomic_load(&snap->flags);			\
    return  M_CONST_CAST(type, &snap->data[SNAPSHOTI_R(flags)]);	\
  }									\
  
// FIXME: Method SWAP ?

#endif
