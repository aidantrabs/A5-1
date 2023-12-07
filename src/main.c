#include <stdio.h>
#include <stdint.h>

#define R1MASK 0x07FFFF /* 19 bit register */
#define R2MASK 0x3FFFFF /* 22 bit register */
#define R3MASK 0x7FFFFF /* 23 bit register */

#define R1MID  0x000100 /* bit 8 */
#define R2MID  0x000400 /* bit 10 */
#define R3MID  0x000400 /* bit 10 */

#define R1TAPS 0x072000 /* bits 18,17,16,13 */
#define R2TAPS 0x300000 /* bits 21,20 */
#define R3TAPS 0x700080 /* bits 22,21,20,7 */

#define R1OUT  0x040000 /* bit 18 (the high bit) */
#define R2OUT  0x200000 /* bit 21 (the high bit) */
#define R3OUT  0x400000 /* bit 22 (the high bit) */

uint32_t R1, R2, R3;

/*
 * 
 * @brief Initialize the shift registers with the key and frame number. 
 * @param key The 64-bit key.
 * @param frame The 22-bit frame number.
 * @return void
 * 
 */
void init(uint8_t *key, uint32_t frame) {
    uint8_t keybit, framebit;

    // Clear the shift registers.
    R1 = R2 = R3 = 0;

    // Load the key into the shift registers.
    for (int i = 0; i < 64; i++) {
        keybit = (key[i/8] >> (i & 7)) & 1;

        R1 ^= keybit; R1 <<= 1;
        R2 ^= keybit; R2 <<= 1;
        R3 ^= keybit; R3 <<= 1;
    }

    // Load the frame number into the shift registers.
    for (int i = 0; i < 22; i++) {
        framebit = (frame >> i) & 1;

        R1 ^= framebit; R1 <<= 1;
        R2 ^= framebit; R2 <<= 1;
        R3 ^= framebit; R3 <<= 1;
    }

    // Complete the initialization by running the shift registers for 100 clocks.
    for (int i = 0; i < 100; i++) {
        uint32_t t;
        uint32_t m = ((R1 >> 8) & 1) + ((R2 >> 10) & 1) + ((R3 >> 10) & 1);

        if (m >= 2) {
            if (R1 & 1) {
                R1 = ((R1 ^ R1TAPS) >> 1) | (1 << 18);
            } else  {
                R1 >>= 1;
            }
        
            if (R2 & 1) {
                R2 = ((R2 ^ R2TAPS) >> 1) | (1 << 21);
            } else {
                R2 >>= 1;
            }
        
            if (R3 & 1) {
                R3 = ((R3 ^ R3TAPS) >> 1) | (1 << 22);
            } else {
                R3 >>= 1;
            }
        }
        else {
            if (!(R1 & 1)) {
                R1 >>= 1;
            }
            if (!(R2 & 1)) {
                R2 >>= 1;
            }
            if (!(R3 & 1)) {
                R3 >>= 1;
            }
        }
    }
}

/*
 *
 * @brief Generate 114 bits of output.
 * @param void
 * @return The 114-bit output.
 * 
 */
uint32_t display() {
    uint32_t output = 0;

    for (int i = 0; i < 114; i++) {
        // XOR the output bits with the high bits of the shift registers.
        uint32_t m = ((R1 >> 8) & 1) + ((R2 >> 10) & 1) + ((R3 >> 10) & 1);

        // Shift the shift registers.
        if (m >= 2) {
            if (R1 & 1) {
                R1 = ((R1 ^ R1TAPS) >> 1) | (1 << 18);
            } else  {
                R1 >>= 1;
            } 

            if (R2 & 1) {
                R2 = ((R2 ^ R2TAPS) >> 1) | (1 << 21);
            } else {
                R2 >>= 1;
            } 

            if (R3 & 1) {
                R3 = ((R3 ^ R3TAPS) >> 1) | (1 << 22);
            } else {
                R3 >>= 1;
            }
        } else {
            if (!(R1 & 1)) {
                R1 >>= 1;
            }

            if (!(R2 & 1)) {
                R2 >>= 1;
            } 

            if (!(R3 & 1)){
                R3 >>= 1;
            } 
        }

        // XOR the output bits with the high bits of the shift registers.
        output = (output << 1) | ((R1 & R1OUT) ^ (R2 & R2OUT) ^ (R3 & R3OUT));
    }

    // Return the 114-bit output.
    return output;
}

/*
 * 
 * @brief Main function.
 * @param void
 * @return 0
 * 
 */
int main() {
    uint8_t key[8] = {  0x12, 
                        0x34,
                        0x56, 
                        0x78,
                        0x9A, 
                        0xBC, 
                        0xDE, 
                        0xF0 
                    };
    uint32_t frame = 0x134;
    uint32_t output;

    // Initialize the shift registers.
    init(key, frame);

    // Generate 114 bits of output.
    output = display();

    // Print the output.
    fprintf(stdout, "Output: %08X\n", output);

    return 0;
}
