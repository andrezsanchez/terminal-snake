#include <stdlib.h>
#include <stdbool.h>

#ifndef VEC2

#define VEC2_TYPE(type, suffix) \
  typedef struct { \
    type x; \
    type y; \
  } vec2##suffix;

#define VEC2_NEW_H(type, suffix) \
  vec2##suffix * vec2##suffix##_new(type x, type y)

#define VEC2_NEW(type, suffix) \
  VEC2_NEW_H(type, suffix) { \
    vec2##suffix * self = malloc(sizeof(vec2##suffix)); \
    if (self) { \
      *self = (vec2##suffix) { x, y }; \
    } \
    return self; \
  }

#define VEC2_CLONE_H(type, suffix) \
  vec2##suffix * vec2##suffix##_clone(vec2##suffix * v)

#define VEC2_CLONE(type, suffix) \
  VEC2_CLONE_H(type, suffix) { \
    return vec2##suffix##_new(v->x, v->y); \
  }

#define VEC2_COPY_H(type, suffix) \
  void vec2##suffix##_copy(vec2##suffix * dest, vec2##suffix source)

#define VEC2_COPY(type, suffix) \
  VEC2_COPY_H(type, suffix) { \
    dest->x = source.x; \
    dest->y = source.y; \
  }

#define VEC2_EQUALS_H(type, suffix) \
  bool vec2##suffix##_equals(vec2##suffix a, vec2##suffix b)

#define VEC2_EQUALS(type, suffix) \
  VEC2_EQUALS_H(type, suffix) { \
    return (a.x == b.x) && (a.y == b.y); \
  }

#define VEC2_ADD_H(type, suffix) \
  void vec2##suffix##_add(vec2##suffix * dest, vec2##suffix a, vec2##suffix b)

#define VEC2_ADD(type, suffix) \
  VEC2_ADD_H(type, suffix) { \
    dest->x = a.x + b.x; \
    dest->y = a.y + b.y; \
  }

#define VEC2_H(type, suffix) \
  VEC2_TYPE(type, suffix) \
  VEC2_NEW_H(type, suffix); \
  VEC2_CLONE_H(type, suffix); \
  VEC2_COPY_H(type, suffix); \
  VEC2_EQUALS_H(type, suffix); \
  VEC2_ADD_H(type, suffix);

#define VEC2(type, suffix) \
  VEC2_NEW(type, suffix) \
  VEC2_CLONE(type, suffix) \
  VEC2_COPY(type, suffix) \
  VEC2_EQUALS(type, suffix) \
  VEC2_ADD(type, suffix)

VEC2_H(int, i)
VEC2_H(long, l)
VEC2_H(float, f)
VEC2_H(double, d)

#endif
