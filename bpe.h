/* bpe.h - v0.1 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) byte pair encoding without any dynamic memory allocation.

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#ifndef BPE_H
#define BPE_H

/* #############################################################################
 * # COMPILER SETTINGS
 * #############################################################################
 */
/* Check if using C99 or later (inline is supported) */
#if __STDC_VERSION__ >= 199901L
#define BPE_INLINE inline
#define BPE_API extern
#elif defined(__GNUC__) || defined(__clang__)
#define BPE_INLINE __inline__
#define BPE_API static
#elif defined(_MSC_VER)
#define BPE_INLINE __inline
#define BPE_API static
#else
#define BPE_INLINE
#define BPE_API static
#endif

#define BPE_STRLEN(s) (sizeof((s)) - 1)

typedef int bpe_bool;

BPE_API BPE_INLINE unsigned short bpe_convert_pair_to_id(unsigned char a, unsigned char b)
{
  return (unsigned short)((a << 8) | b);
}

BPE_API BPE_INLINE void bpe_convert_id_to_pair(unsigned short key, unsigned char *a, unsigned char *b)
{
  *a = (unsigned char)(key >> 8);
  *b = (unsigned char)(key & 0xFF);
}

#define BPE_MAX_SYMBOLS 65536 /* two chars combined 256 * 256 chars */
#define BPE_NUM_CHARS 256

#ifndef BPE_MAX_ITERATIONS
#define BPE_MAX_ITERATIONS 1024 /* Support up to 1024 replacements */
#endif

typedef struct bpe
{
  /* Provided by the user */
  char *text;
  unsigned int text_length;

  /* Provided by the library */
  unsigned short most_frequent_pair;
  unsigned int most_frequent_pair_count;

  unsigned char replacement_symbol;
  unsigned char replacement_symbols[BPE_MAX_ITERATIONS];
  unsigned short replacement_pairs[BPE_MAX_ITERATIONS]; /* Maps new symbols to original pairs */

  unsigned int iteration_count;

} bpe;

BPE_API BPE_INLINE void bpe_most_frequent_pair(bpe *model)
{
  unsigned int count[BPE_MAX_SYMBOLS] = {0};
  unsigned char used_chars[BPE_NUM_CHARS] = {0}; /* Track characters used in text to determine the replacement_symbol*/
  unsigned int i;
  unsigned short j;

  /* Count the frequency of each pair of characters */
  for (i = 0; i < model->text_length; i += 2)
  {
    unsigned char a;
    unsigned char b;

    /* For uneven buffer_size we cannot build a pair for the last one */
    if (i + 1 >= model->text_length)
    {
      break;
    }

    a = (unsigned char)model->text[i];
    b = (unsigned char)model->text[i + 1];

    used_chars[a] = 1;
    used_chars[b] = 1;

    count[bpe_convert_pair_to_id(a, b)]++;
  }

  /* Find the most frequent pair */
  model->most_frequent_pair_count = 0;

  for (j = 0; j < (BPE_MAX_SYMBOLS - 1); ++j)
  {
    if (count[j] > model->most_frequent_pair_count)
    {
      model->most_frequent_pair_count = count[j];
      model->most_frequent_pair = j;
    }
  }

  /* Find an unused character to use as a replacement */
  for (j = 128; j < 255; ++j)
  {
    if (!used_chars[j])
    {
      model->replacement_symbol = (unsigned char)j;
      model->replacement_symbols[model->iteration_count] = model->replacement_symbol;
      model->replacement_pairs[model->iteration_count] = model->most_frequent_pair;
      return;
    }
  }

  /* If no unused character is found, assign a new extended symbol */
  if (model->iteration_count < BPE_MAX_ITERATIONS)
  {
    model->replacement_symbol = (unsigned char)(256 + model->iteration_count);
    model->replacement_symbols[model->iteration_count] = model->replacement_symbol;
    model->replacement_pairs[model->iteration_count] = model->most_frequent_pair;
  }
  else
  {
    /* If we run out of replacement space, we stop further encoding */
    model->most_frequent_pair_count = 0;
  }
}

BPE_API BPE_INLINE void bpe_replace_pair(bpe *model)
{
  int i = 0;
  int j = 0;
  unsigned int replacements = 0;

  while (model->text[i] != '\0')
  {
    unsigned char first;
    unsigned char second;

    bpe_convert_id_to_pair(model->most_frequent_pair, &first, &second);

    /* If we find the pair, replace it with the new symbol */
    if (model->text[i] == (char)first && model->text[i + 1] == (char)second)
    {
      model->text[j++] = (char)model->replacement_symbol;
      i += 2;
      replacements++;
    }
    else
    {
      model->text[j++] = model->text[i++];
    }
  }

  model->text[j] = '\0';
  model->text_length -= replacements;
}

BPE_API BPE_INLINE bpe_bool bpe_forward(bpe *model)
{
  /* (1) Find most frequent pair*/
  bpe_most_frequent_pair(model);

  if (model->most_frequent_pair_count <= 1)
  {
    return (0);
  }

  /* (2) Replace the pair with a new symbol */
  bpe_replace_pair(model);

  model->iteration_count++;

  return (1);
}

BPE_API BPE_INLINE void bpe_decode(bpe *model)
{
  int i, j, k;
  char temp[BPE_MAX_SYMBOLS]; /* Buffer to hold expanding text */

  /* Start with the compressed text */
  unsigned int text_length = model->text_length;

  /* Process replacements in reverse order */
  for (i = (int)(model->iteration_count - 1); i >= 0; i--)
  {
    unsigned short replacement_pair = model->replacement_pairs[i]; /* Retrieve original pair */
    unsigned char replacement_symbol = model->replacement_symbols[i];
    unsigned char first, second;
    bpe_convert_id_to_pair(replacement_pair, &first, &second);

    j = 0;
    for (k = 0; k < (int)text_length; k++)
    {
      if ((unsigned char)model->text[k] == replacement_symbol)
      {
        /* Replace with the original pair */
        temp[j++] = (char)first;
        temp[j++] = (char)second;
      }
      else
      {
        temp[j++] = model->text[k];
      }
    }

    /* Copy back expanded text */
    text_length = (unsigned int)j;
    for (j = 0; j < (int)text_length; j++)
    {
      model->text[j] = temp[j];
    }
    model->text[text_length] = '\0';
  }

  model->text_length = text_length;
}

#endif /* BPE_H */

/*
   ------------------------------------------------------------------------------
   This software is available under 2 licenses -- choose whichever you prefer.
   ------------------------------------------------------------------------------
   ALTERNATIVE A - MIT License
   Copyright (c) 2025 nickscha
   Permission is hereby granted, free of charge, to any person obtaining a copy of
   this software and associated documentation files (the "Software"), to deal in
   the Software without restriction, including without limitation the rights to
   use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is furnished to do
   so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
   ------------------------------------------------------------------------------
   ALTERNATIVE B - Public Domain (www.unlicense.org)
   This is free and unencumbered software released into the public domain.
   Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
   software, either in source code form or as a compiled binary, for any purpose,
   commercial or non-commercial, and by any means.
   In jurisdictions that recognize copyright laws, the author or authors of this
   software dedicate any and all copyright interest in the software to the public
   domain. We make this dedication for the benefit of the public at large and to
   the detriment of our heirs and successors. We intend this dedication to be an
   overt act of relinquishment in perpetuity of all present and future rights to
   this software under copyright law.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
   WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
   ------------------------------------------------------------------------------
*/
