#include <string.h>
#include <ctype.h>
#include "utils.h"

void trim(char *string)
{
	// Remover espaços do início movendo o conteúdo para o começo
	char *start = string;
	while (isspace((unsigned char)*start)) {
		start++;
	}
	if (start != string) {
		// move a parte sem espaços iniciais até o início do buffer, incluindo o '\0'
		memmove(string, start, strlen(start) + 1);
	}

	// Agora remover espaços do fim
	char *end = string + strlen(string) - 1;
	while (end >= string && isspace((unsigned char)*end)) {
		*end = '\0';
		end--;
	}
}
