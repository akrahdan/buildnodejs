/* Copyright Joyent, Inc. and other Node contributors. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/*
 * This file is private to libuv. It provides common functionality to both
 * Windows and Unix backends.
 */

#ifndef UV_COMMON_H_
#define UV_COMMON_H_

#include <assert.h>
#include <stdarg.h>
#include <stddef.h>


#include <stdint.h>


#include "uv.h"
#include "tree.h"
#include "queue.h"

#if !defined(snprintf) && defined(_MSC_VER) && _MSC_VER < 1900
extern int snprintf(char*, size_t, const char*, ...);
#endif

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define container_of(ptr, type, member) \
  ((type *) ((char *) (ptr) - offsetof(type, member)))

#define STATIC_ASSERT(expr)                                                   \
  void uv__static_assert(int static_assert_failed[1 - 2 * !(expr)])

/* Allocator prototypes */
void *uv__calloc(size_t count, size_t size);
char *uv__strdup(const char* s);
char *uv__strndup(const char* s, size_t n);
void* uv__malloc(size_t size);
void uv__free(void* ptr);
void* uv__realloc(void* ptr, size_t size);

#endif /* UV_COMMON_H_ */
