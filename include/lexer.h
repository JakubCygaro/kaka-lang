#ifndef LEXER_H
#define LEXER_H
#include "types.h"
#include <stdio.h>
#include <stddef.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include "stringmap.h"

TokenStream* get_token_stream(FILE*, StringMap*);

#endif