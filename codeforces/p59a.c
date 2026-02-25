#include <ctype.h>
#include <stdio.h>
#include <string.h>

int main() {
    char s[101];
    scanf("%s", s);

    int upper = 0, lower = 0;
    int n = strlen(s);

    for (int i = 0; i < n; i++) {
        if (isupper(s[i]))
            upper++;
        else
            lower++;
    }

    if (upper > lower) {
        for (int i = 0; i < n; i++) s[i] = toupper(s[i]);
    } else {
        for (int i = 0; i < n; i++) s[i] = tolower(s[i]);
    }

    printf("%s\n", s);
    return 0;
}