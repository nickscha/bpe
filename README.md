# bpe (in development)
A C89 standard compliant, single header, nostdlib (no C Standard Library) byte pair encoding without any dynamic memory allocation.

See also: https://en.wikipedia.org/wiki/Byte_pair_encoding

For more information please look at the "bpe.h" file or take a look at the "examples" or "tests" folder.

## Quick Start

Download or clone bpe.h and include it in your project.

```C
#include "bpe.h"

#include "stdio.h" /* Only needed here for demonstrational printf */

int main() {

    char input[] = "abababab";

    bpe model = {0};
    model.text = input;
    model.text_length = BPE_STRLEN(input);
    model.replacement_symbol = (char)65; /* Start from an arbitrary unused symbol */

    /* Run until there are no more replacements. Alternativly you can limit it by fixed iteration loop */
    while (bpe_forward(&model))
    {
        /*
          Intermmediate Access

          model.most_frequent_pair_count
          model.most_frequent_pair
          model.text
          model.text_length
          model.replacement_symbol
        */
    }

    /* Final text*/
    printf("compressed text: %s\n", model.text);

    return 0;
}
```
