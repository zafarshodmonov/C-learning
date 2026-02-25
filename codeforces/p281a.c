#include <ctype.h>
#include <stdio.h>

int main() {
    char s[1005];

    scanf("%s", s);

    s[0] = toupper(s[0]);

    printf("%s\n", s);

    return 0;
}