#include <stdio.h>

int main() {
	int sugar;

	scanf("%d", &sugar);

	if (sugar == 4 || sugar == 7)
		printf("-1");
	else {
		switch (sugar % 5) {
		case 0: 
			printf("%d", sugar / 5); 
			break;
		case 1:
		case 3:
			printf("%d", sugar / 5 + 1); 
			break;
		case 2:
		case 4:
			printf("%d", sugar / 5 + 2);
			break;
		}
	}
	return 0;
}
