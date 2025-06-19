#include <string.h>
#include <ctype.h>
#include "utils.h"

void trim(char *string)
{
	// Remove espaços do início
	while (isspace((unsigned char)*string)) {
		string++;
	}

	// Se string está vazia, não há mais nada a fazer
	if (*string == '\0') {
		return;
	}

	// Posição do último caractere
	char *end = string + strlen(string) - 1;
	while (end > string && isspace((unsigned char)*end)) {
		*end = '\0';
		end--;
	}
}
