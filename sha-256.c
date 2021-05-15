#include <stdint.h>
#include <string.h>

#include "sha-256.h"

#define CHUNK_SIZE 64
#define INT64_SIZE 8

/*
 * Comments from pseudo-code at https://en.wikipedia.org/wiki/SHA-2 are reproduced here.
 * When useful for clarification, portions of the pseudo-code are reproduced here too.
 */

/*
 * Initialize array of round constants:
 * (first 32 bits of the fractional parts of the cube roots of the first 64 primes 2..311):
 */
static const uint32_t k[] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static inline uint32_t right_rot(uint32_t value, unsigned int count)
{
	/*
	 * Defined behaviour in standard C for all count where 0 < count < 32,
	 * which is what we need here.
	 */
	return value >> count | value << (32 - count);
}

/*
 * Limitations:
 * - Since input is a pointer in RAM, the data to hash should be in RAM, which could be a problem
 *   for large data sizes.
 * - SHA algorithms theoretically operate on bit strings. However, this implementation has no support
 *   for bit string lengths that are not multiples of eight, and it really operates on arrays of bytes.
 *   In particular, the len parameter is a number of bytes.
 */
void calc_sha_256(uint8_t hash[32], const void * input, size_t len)
{
	/*
	 * Note 1: All integers (expect indexes) are 32-bit unsigned integers and addition is calculated modulo 2^32.
	 * Note 2: For each round, there is one round constant k[i] and one entry in the message schedule array w[i], 0 = i = 63
	 * Note 3: The compression function uses 8 working variables, a through h
	 * Note 4: Big-endian convention is used when expressing the constants in this pseudocode,
	 *     and when parsing message block data from bytes to words, for example,
	 *     the first word of the input message "abc" after padding is 0x61626380
	 */

	unsigned i, j;

	/* The length of the original message in bits as a 64-bit big-endian integer */
	uint8_t total_len[INT64_SIZE];
	for (i = 0; i < INT64_SIZE; i++)
		total_len[i] = (uint8_t) ((len << 3) >> ((INT64_SIZE - i - 1) * 8));

	/*
	 * Initialize hash values:
	 * (first 32 bits of the fractional parts of the square roots of the first 8 primes 2..19):
	 */
	uint32_t h[] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 };

	/* Reserve a chunk for Pre-processing (appending the single '1' bit, optional padding and the message length) */
	uint8_t processed_chunk[CHUNK_SIZE];

	const uint8_t *chunk = input;
	const uint8_t *input_end = (uint8_t *) input + len;
	size_t appended = 0;

	/* Process the message and additional appended bytes in successive 512-bit chunks */
	while (chunk < input_end || appended < 1 + INT64_SIZE) {
		const uint8_t *p;

		if (chunk + CHUNK_SIZE <= input_end) {
			/* Normal processing */
			p = chunk;
			chunk += CHUNK_SIZE;
		} else {
			/* We are near or past the end of the input, do we have any input bytes remaining? */
			const size_t remaining = input_end - chunk;
			if (remaining > 0)
				memcpy(processed_chunk, chunk, remaining);
			/* Add padding bytes (some may be overwritten) */
			memset(&processed_chunk[remaining], 0, CHUNK_SIZE - remaining);
			/* Did we not yet add the single '1' bit in previous chunk? */
			if (appended < 1)
				processed_chunk[remaining] = 0x80;
			/* Do we have the room left for the 64-bit message length? */
			if (remaining + 1 + INT64_SIZE <= CHUNK_SIZE)
				memcpy(&processed_chunk[CHUNK_SIZE - INT64_SIZE], total_len, INT64_SIZE);

			appended += CHUNK_SIZE - remaining;
			p = processed_chunk;
			chunk += remaining;
		}

		uint32_t ah[8];

		/* Initialize working variables to current hash value: */
		for (i = 0; i < 8; i++)
			ah[i] = h[i];

		/* Compression function main loop: */
		for (i = 0; i < 4; i++) {
			/*
			 * The w-array is really w[64], but since we only need
			 * 16 of them at a time, we save stack by calculating
			 * 16 at a time.
			 *
			 * This optimization was not there initially and the
			 * rest of the comments about w[64] are kept in their
			 * initial state.
			 */

			/*
			 * create a 64-entry message schedule array w[0..63] of 32-bit words
			 * (The initial values in w[0..63] don't matter, so many implementations zero them here)
			 * copy chunk into first 16 words w[0..15] of the message schedule array
			 */
			uint32_t w[16];

			for (j = 0; j < 16; j++) {
				if (i == 0) {
					w[j] = (uint32_t) p[0] << 24 | (uint32_t) p[1] << 16 |
						(uint32_t) p[2] << 8 | (uint32_t) p[3];
					p += 4;
				} else {
					/* Extend the first 16 words into the remaining 48 words w[16..63] of the message schedule array: */
					const uint32_t s0 = right_rot(w[(j + 1) & 0xf], 7) ^ right_rot(w[(j + 1) & 0xf], 18) ^ (w[(j + 1) & 0xf] >> 3);
					const uint32_t s1 = right_rot(w[(j + 14) & 0xf], 17) ^ right_rot(w[(j + 14) & 0xf], 19) ^ (w[(j + 14) & 0xf] >> 10);
					w[j] = w[j] + s0 + w[(j + 9) & 0xf] + s1;
				}
				const uint32_t s1 = right_rot(ah[4], 6) ^ right_rot(ah[4], 11) ^ right_rot(ah[4], 25);
				const uint32_t ch = (ah[4] & ah[5]) ^ (~ah[4] & ah[6]);
				const uint32_t temp1 = ah[7] + s1 + ch + k[i << 4 | j] + w[j];
				const uint32_t s0 = right_rot(ah[0], 2) ^ right_rot(ah[0], 13) ^ right_rot(ah[0], 22);
				const uint32_t maj = (ah[0] & ah[1]) ^ (ah[0] & ah[2]) ^ (ah[1] & ah[2]);
				const uint32_t temp2 = s0 + maj;

				ah[7] = ah[6];
				ah[6] = ah[5];
				ah[5] = ah[4];
				ah[4] = ah[3] + temp1;
				ah[3] = ah[2];
				ah[2] = ah[1];
				ah[1] = ah[0];
				ah[0] = temp1 + temp2;
			}
		}

		/* Add the compressed chunk to the current hash value: */
		for (i = 0; i < 8; i++)
			h[i] += ah[i];
	}

	/* Produce the final hash value (big-endian): */
	for (i = 0, j = 0; i < 8; i++)
	{
		hash[j++] = (uint8_t) (h[i] >> 24);
		hash[j++] = (uint8_t) (h[i] >> 16);
		hash[j++] = (uint8_t) (h[i] >> 8);
		hash[j++] = (uint8_t) h[i];
	}
}
