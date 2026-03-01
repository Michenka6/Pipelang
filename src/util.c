#include "../include/util.h"

void string_builder_reset(Nob_String_Builder *sb)
{
    sb->items = NULL;
    sb->capacity = 0;
    sb->count = 0;
}
