
#include <stdlib.h>



int stringcmp(char* input, char* comparison, int compLength)
{
	int i;
	for(i = 0; i < compLength; i++)
	{
		//printf("input[i] %c\n", input[i]);
		if(input[i] == '\0' || input[i] != comparison[i])
		{
			return 0;
		}
	}
	return 1;
}