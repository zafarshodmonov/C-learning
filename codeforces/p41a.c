#include <stdio.h>
#include <string.h>

int helper(char* s1, char* s2, int n);

int main() {
    char s1[101];
    char s2[101];
    scanf("%s", s1);
    scanf("%s", s2);
    int len_s = strlen(s1);
    int len_t = strlen(s2);
 
    if (len_s != len_t) {
        printf("NO");
        return 0;
    }
    if (helper(s1, s2, len_s)) {
        printf("YES");
    }
    else {
        printf("NO");
    }

    return 0;
}

int helper(char* s1, char* s2, int n) {
    for (int i = 0; i < n; i++) {
        if (s1[i] != s2[(n - 1) - i]) {
            return 0;
        }
    }
    return 1;
}