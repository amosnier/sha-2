#include <stdint.h>
#include <stdio.h>

#include "sha-256.h"

int main()
{
	uint8_t hash[32];
	calc_sha_256(hash, "hello\n", 6);
	for (size_t i = 0; i < sizeof hash; i++) {
		printf("%02x", hash[i]);
	}
	printf("\n");
}
