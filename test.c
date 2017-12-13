#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "sha-256.h"

struct vector {
	const char *input;
	const char *output;
};

static const struct vector VECTORS[] = {
	{
		"",
		"e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"
	},
	{
		"abc",
		"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"
	},
	{
		"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
		"a8ae6e6ee929abea3afcfc5258c8ccd6f85273e0d4626d26c7279f3250f77c8e"
	},
	{
		"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcde",
		"057ee79ece0b9a849552ab8d3c335fe9a5f1c46ef5f1d9b190c295728628299c"
	},
	{
		"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0",
		"2a6ad82f3620d3ebe9d678c812ae12312699d673240d5be8fac0910a70000d93"
	},
	{
		"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
		"248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1"
	},
	{
		"abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoi"
		"jklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
		"cf5b16a778af8380036ce59e7b0492370b249b11e8f07a51afac45037afee9d1"
	}
};

static void hash_to_string(char string[65], const uint8_t hash[32])
{
	for (size_t i = 0; i < 32; i++) {
		string += sprintf(string, "%02x", hash[i]);
	}
}	
	
static void test(const char input[], const char output[])
{
	uint8_t hash[32];
	char hash_string[65];
	calc_sha_256(hash, input, strlen(input));
	hash_to_string(hash_string, hash);
	printf("input: %s\n", input);
	printf("hash : %s\n", hash_string);
	if (strcmp(output, hash_string)) {
		printf("FAILURE!\n\n");
	} else {
		printf("SUCCESS!\n\n");
	}		
}
	

int main(void)
{
	for (size_t i = 0; i < (sizeof VECTORS / sizeof (struct vector)); i++) {
		const struct vector *vector = &VECTORS[i];
		test(vector->input, vector->output);
	}
	return 0;
}
