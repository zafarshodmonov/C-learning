#include <stdio.h>
#include <string.h>

void helper(char* word, int* len);

int main() {
    int n;
    scanf("%d", &n);

    char word[101];

    while (n--) {
        scanf("%s", word);

        int len = strlen(word);

        helper(word, &len);
    }

    return 0;
}

void helper(char* word, int* len) {
    if (*len > 10) {
        printf("%c%d%c\n", word[0], *len - 2, word[*len - 1]);
    } else {
        printf("%s\n", word);
    }
}