/* bpe.h - v0.1 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) byte pair encoding without any dynamic memory allocation.

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#include "../bpe.h"
#include "test.h" /* Simple testing framework */

static const char *redacted_msg = "... (too much to display)";

bpe bpe_test_process(char txt[], unsigned int txtLength)
{
  bpe model = {0};
  model.text = txt;
  model.text_length = txtLength;

  printf("[bpe]   text_in: %s (%i)\n", txtLength > 40 ? redacted_msg : model.text, model.text_length);

  while (bpe_forward(&model))
  {
    unsigned char first, second;
    bpe_convert_id_to_pair(model.most_frequent_pair, &first, &second);

    printf("[bpe] [%2d] text: %s (%i) (most_frequent: '%c%c', occured: %d, replace_with: '%c')\n", model.iteration_count, txtLength > 40 ? redacted_msg : model.text, model.text_length, first, second, model.most_frequent_pair_count, model.replacement_symbol);
  }

  printf("[bpe]  text_out: %s (%i)\n", txtLength > 40 ? redacted_msg : model.text, model.text_length);
  printf("[bpe] --------------------------------------------------\n\n");
  return (model);
}

void bpe_test_simple_even(void)
{
  char input[] = "abababab";
  bpe finalModel = bpe_test_process(input, BPE_STRLEN(input));
  assert(finalModel.most_frequent_pair_count == 1);
  assert(finalModel.text_length == 2);
}

void bpe_test_simple_even_upper(void)
{
  char input[] = "ABABABAB";
  bpe finalModel = bpe_test_process(input, BPE_STRLEN(input));
  assert(finalModel.most_frequent_pair_count == 1);
  assert(finalModel.text_length == 2);
}

void bpe_test_simple_uneven(void)
{
  char input[] = "ababababr";
  bpe finalModel = bpe_test_process(input, BPE_STRLEN(input));
  assert(finalModel.most_frequent_pair_count == 1);
  assert(finalModel.text_length == 3);
}

void bpe_test_simple_uneven_upper(void)
{
  char input[] = "ABABABABr";
  bpe finalModel = bpe_test_process(input, BPE_STRLEN(input));
  assert(finalModel.most_frequent_pair_count == 1);
  assert(finalModel.text_length == 3);
}

void bpe_test_simple_even_multicompress(void)
{
  char input[] = "ababababrarara";
  bpe finalModel = bpe_test_process(input, BPE_STRLEN(input));
  assert(finalModel.most_frequent_pair_count == 1);
  assert(finalModel.text_length == 5);
}

void bpe_test_simple_numbers(void)
{
  char input[] = "01010202333333";
  bpe finalModel = bpe_test_process(input, BPE_STRLEN(input));
  assert(finalModel.most_frequent_pair_count == 1);
  assert(finalModel.text_length == 7);

  printf("encoded: %s\n", finalModel.text);

  bpe_decode(&finalModel);

  printf("decoded: %s\n", finalModel.text);

  assert(finalModel.text_length == 14);
}

void bpe_test_simple_special_characters(void)
{
  char input[] = "$&(){}\\\\(){}()\\\\a$&$&";
  bpe finalModel = bpe_test_process(input, BPE_STRLEN(input));
  assert(finalModel.most_frequent_pair_count == 1);
  assert(finalModel.text_length == 11);
}

void bpe_test_long_text(void)
{
  char input[] = "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum.";
  bpe finalModel;

  assert(BPE_STRLEN(input) == 508);

  finalModel = bpe_test_process(input, BPE_STRLEN(input));

  assert(finalModel.most_frequent_pair_count == 1);
  assert(finalModel.text_length == 243);
}

void bpe_test_decode(void)
{
  char input[] = "abababab";
  bpe finalModel = bpe_test_process(input, BPE_STRLEN(input));
  assert(finalModel.most_frequent_pair_count == 1);
  assert(finalModel.text_length == 2);

  printf("encoded: %s\n", finalModel.text);

  bpe_decode(&finalModel);

  printf("decoded: %s\n", finalModel.text);

  assert(finalModel.text_length == 8);
}

void bpe_test_decode_long_text(void)
{
  char input[] = "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum.";
  bpe finalModel;

  assert(BPE_STRLEN(input) == 508);

  finalModel = bpe_test_process(input, BPE_STRLEN(input));

  assert(finalModel.most_frequent_pair_count == 1);
  assert(finalModel.text_length == 243);

  printf("encoded: %s\n", finalModel.text);

  bpe_decode(&finalModel);

  printf("decoded: %s\n", finalModel.text);

  assert(finalModel.text_length == 508);
}

void bpe_test_unicode_to_utf8(void)
{
  int i;
  int length;

  unsigned char utf8[4];           /* UTF-8 can be up to 4 bytes */
  unsigned long unicode = 0x1F600; /* 😀 (Unicode U+1F600) */

  length = bpe_unicode_to_utf8(unicode, utf8);

  assert(length == 4);
  assert(utf8[0] == 0xF0);
  assert(utf8[1] == 0x9F);
  assert(utf8[2] == 0x98);
  assert(utf8[3] == 0x80);

  printf("UTF-8 Encoding: ");
  for (i = 0; i < length; ++i)
  {
    printf("%02X ", utf8[i]);
  }
  printf("\n");
}

void bpe_test_utf8_to_unicode(void)
{
  int length;
  unsigned char utf8[] = {0xF0, 0x9F, 0x98, 0x80}; /* UTF-8 encoding of 😀 (U+1F600) */

  unsigned long unicode = bpe_utf8_to_unicode(utf8, &length);

  assert(length == 4);
  assert(unicode == 128512);

  if (length > 0)
  {
    printf("Decoded Unicode: U+%lX\n", unicode);
  }
  else
  {
    printf("Invalid UTF-8 sequence.\n");
  }
}

int main(void)
{

  bpe_test_simple_even();
  bpe_test_simple_even_upper();
  bpe_test_simple_uneven();
  bpe_test_simple_uneven_upper();
  bpe_test_simple_even_multicompress();
  bpe_test_simple_numbers();
  bpe_test_simple_special_characters();
  bpe_test_long_text();
  bpe_test_decode();
  bpe_test_decode_long_text();
  bpe_test_unicode_to_utf8();
  bpe_test_utf8_to_unicode();

  return 0;
}

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
