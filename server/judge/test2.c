#include <stdio.h>

int main()
{
	int n,x,input,i;

	scanf("%d %d", &n, &x);

	for(i =0; i<n ;i++)
	{
		scanf("%d",&input);
	
		if(input <x)
			printf("%d ",input);
	}
}	
