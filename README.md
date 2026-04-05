# badtoml
## not very good toml library

this exists because i needed a toml library that didnt depend on anything, as i intended to use this alongside the playstation 3 sdk for creating prx modules. the sdk's linker is mean and hates you, and wont let you link very basic functionality half the time<br><br>

this library supports most types available in toml
1. integers (decimal, binary, octal, and hexadecimal)
2. booleans
3. strings
4. floats (double-precision only)
<br>
practical use cases of this library are limited, as most platforms have a proper standard library which should support better toml implementations than this

## usage
the library exposes two functions for you to call, if these suck bad enough you can always just put more of them in the header and rewrite whatever you want<br>
using it looks something like this:
```c
#include <stdio.h>
#include <string.h>

#include "toml.h"
#include "keymap.h"

int main(void) {
    struct TOMLEntry entries[8];
    char *testFile =
    "[test]\n"
    "string = \"text\"\n"
    "int = 10\n"
    "bin = 0b1010\n"
    "oct = 0o12\n"
    "hex = 0xA\n"
    "float = 3.14\n"
    "bool = true\n"
    "literal = \'literal text\'\0"; // There is currently no difference between strings and literals 


    int result = TOMLParseFileBuf(testFile, strlen(testFile), entries, 8);
    if (result == TOML_PARSE_FAIL) {
        printf("Parse failed.\n");
    }

    char *text;
    int integer;
    double floating;
    int boolean;

    struct TOMLKeyMap map[] = {
        {"string", TOML_STRING, &text},
        {"int", TOML_INT, &integer},
        {"float", TOML_FLOAT, &floating},
        {"bool", TOML_BOOL, &boolean}
    };

    result = TOMLApplyEntriesToKeyMap(entries, 8, map, 4);

    if (result == TOML_SUCCESS) {
        printf("text: %s\n", text);
        printf("integer: %d\n", integer);
        printf("floating: %f\n", floating);
        printf("boolean: %s\n", (boolean ? "True" : "False"));
    }
}
```