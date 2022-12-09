#include <stdio.h>
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

int DP[18];
int t[18];
int p[18];

int main() {
    int n, prev;
    scanf("%d", &n);
    for (int i = 1; i <= n; i++)
        scanf("%d %d", &t[i], &p[i]);

    for (int i = n; i > 0; i--) {
        prev = i + t[i];
        if (prev > n + 1) DP[i] = DP[i + 1];
        else DP[i] = MAX(DP[i + 1], DP[prev] + p[i]);
    }
    printf("%d\n", DP[1]);

    return 0;
}
