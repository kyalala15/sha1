#include "ap_axi_sdata.h"
#include "hls_stream.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define INPUT_SIZE 5

// SHA-1 functions
static uint32_t sha1_ch(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (~x & z);
}

static uint32_t sha1_maj(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (x & z) ^ (y & z);
}

static uint32_t sha1_parity(uint32_t x, uint32_t y, uint32_t z) {
    return x ^ y ^ z;
}

static uint32_t left_rotate(uint32_t x, uint32_t n){
    return (((x) << (n)) | ((x) >> (32 - (n))));
}

// Compute the SHA-1 hash of the given message
void sha1_hash(const uint8_t *message, uint32_t hash[5]) {
    static const uint32_t sha1_initial_hash[5] = {
            0x67452301,
            0xEFCDAB89,
            0x98BADCFE,
            0x10325476,
            0xC3D2E1F0
    };

    // Initialize hash values
    memcpy(hash, sha1_initial_hash, sizeof(uint32_t) * 5);

    // Process message in 512-bit blocks
    size_t padded_len = ((INPUT_SIZE + 8) / 64 + 1) * 64;
    uint8_t padded_message[padded_len];
    memset(padded_message, 0, padded_len);  // Initialize to all zeros
    memcpy(padded_message, message, INPUT_SIZE);
    padded_message[INPUT_SIZE] = 0x80;  // Append '1' bit

    // Append length (in bits) to the message
    uint64_t bit_len = INPUT_SIZE * 8;
    for (int i = 0; i < 8; ++i) {
        padded_message[padded_len - 8 + i] = (bit_len >> ((7 - i) * 8)) & 0xFF;
    }

    // Process each block
    sha1_hash_label0:for (size_t i = 0; i < padded_len; i += 64) {
        // Prepare message schedule (big-endian)
        uint32_t w[80];
#pragma HLS ARRAY_PARTITION dim=1 type=complete variable=w
        uint32_t a, b, c, d, e, f, k, temp;

        sha1_hash_label3:for (int j = 0; j < 16; ++j) {
            w[j] = (padded_message[i + j * 4] << 24) | (padded_message[i + j * 4 + 1] << 16) | (padded_message[i + j * 4 + 2] << 8) | (padded_message[i + j * 4 + 3]);
        }

        sha1_hash_label4:for (int j = 16; j < 80; ++j) {
            w[j] = left_rotate((w[j - 3] ^ w[j - 8] ^ w[j - 14] ^ w[j - 16]), 1);
        }

        // Initialize hash value for this chunk
        a = hash[0];
        b = hash[1];
        c = hash[2];
        d = hash[3];
        e = hash[4];

        // Main loop
 ShaLoop:
        for (int j = 0; j < 80; ++j) {
            if (j < 20) {
                f = sha1_ch(b, c, d);
                k = 0x5A827999;
            } else if (j < 40) {
                f = sha1_parity(b, c, d);
                k = 0x6ED9EBA1;
            } else if (j < 60) {
                f = sha1_maj(b, c, d);
                k = 0x8F1BBCDC;
            } else {
                f = sha1_parity(b, c, d);
                k = 0xCA62C1D6;
            }

            temp = left_rotate(a, 5) + f + e + k + w[j];
            e = d;
            d = c;
            c = left_rotate(b, 30);
            b = a;
            a = temp;
        }

        // Add this chunk's hash to the result so far
        hash[0] += a;
        hash[1] += b;
        hash[2] += c;
        hash[3] += d;
        hash[4] += e;
    }
}


void sha1(hls::stream< ap_axis<32,2,5,6> > &A,hls::stream< ap_axis<32,2,5,6> > &B)
{
	#pragma HLS INTERFACE axis register both port=A
	#pragma HLS INTERFACE axis register both port=B
	#pragma HLS INTERFACE ap_ctrl_none port=return

    ap_axis<32, 2, 5, 6> input;
    ap_axis<32, 2, 5, 6> output;
    uint32_t hash[5];
    uint8_t ascii_array[INPUT_SIZE];
    while(1){
    	for (int i = 0; i < INPUT_SIZE; i++){
    		A.read(input);
    		ascii_array[i] = input.data;
    	}
        const uint8_t *message = ascii_array;
        sha1_hash(message, hash);
        for (int i = 0; i < 5; i++){
        	output.data = hash[i];
        	output.keep = input.keep;
        	output.strb = input.strb;
        	output.dest = input.dest;
        	output.id = input.id;
        	output.user = input.user;
        	B.write(output);
        }
        if(input.last){
            break;
        }
    }
    output.last = 1;
}



