#include <stdio.h>
#include <string.h>
#include <ctype.h>

int two(char *s1, char *s2, int n);

int main() {
    char s1[101], s2[101];

    scanf("%s", s1);
    scanf("%s", s2);

    int n = strlen(s1);

    int ans = two(s1, s2, n);

    printf("%d\n", ans);
    return 0;
}

int two(char *s1, char *s2, int n) {
    for (int i = 0; i < n; i++) {
        char c1 = tolower(s1[i]);
        char c2 = tolower(s2[i]);

        if (c1 < c2) {
            return -1;
        } else if (c1 > c2) {
            return 1;
        }
    }
    return 0;
}