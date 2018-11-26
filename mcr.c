#include "mcr.h"
#include "exint.h"

// Enough bits to read any of the three tracks...
#define BIT_BUFFER_LEN 800 

// Variables used by the interrupt handlers...
static volatile bool next_bit = 0;                       // next bit to read
static volatile unsigned char bits[BIT_BUFFER_LEN / 8];  // buffer for bits being read
static volatile short num_bits = 0;                      // number of bits already read

// Manipulate the bit buffer...
static void bits_set(short index, bool bit);
static bool bits_get(short index);

// The interrupt handlers...
static void handle_data(void);
static void handle_clock(void);

static short decode_bits(char *data, unsigned char size);
static void reverse_bits();
static bool verify_parity(unsigned char c);
static bool verify_lrc(short start, short length);
static short find_sentinel(unsigned char pattern);

unsigned char track;

void mcr_begin(unsigned char _track)
{
    track = _track;
    // Reading is more reliable when using interrupts...
    exint_attach(0, handle_data, CHANGE);    // data on digital pin 2 (INT0)
    exint_attach(1, handle_clock, FALLING);  // clock on digital pin 3 (INT1)
    DDR_CLS &= ~_BV(BIT_CLS);                          // cls on digital pin 4 (D4)
    PORT_CLS |= _BV(BIT_CLS);                          // internal pullup
}
void mcr_restart()
{
	exint_attach(0, handle_data, CHANGE);    // data on digital pin 2 (INT0)
	exint_attach(1, handle_clock, FALLING);  // clock on digital pin 3 (INT1)
}
void mcr_stop()
{
    exint_detach(0);
    exint_detach(1);
}
bool mcr_available()
{
    return cls_is_low();
}
short mcr_get_dataKartu(){
	return mcr_read(mcr_dataKartu, DATA_BUFFER_LEN);
}
short mcr_read(char *data, unsigned char size)
{
    // Fail if no card present...
    if (!mcr_available()) {
        return -1;
    }

    // Empty the bit buffer...
    num_bits = 0;
    next_bit = 0;

    // Wait while the data is being read by the interrupt routines...
    while (mcr_available()) {}

    // Decode the raw bits...
    short chars = decode_bits(data, size);

    // If the data looks bad, reverse and try again...
    if (chars < 0) {
        reverse_bits();
        chars = decode_bits(data, size);
    }

    return chars;
}
/*end of public functions*/

static void reverse_bits()
{
    for (short i = 0; i < num_bits / 2; i++) {
        bool b = bits_get(i);

        bits_set(i, bits_get(num_bits - i - 1));
        bits_set(num_bits - i - 1, b);
    }
}
static bool verify_parity(unsigned char c)
{
    unsigned char parity = 0;

    for (unsigned char i = 0; i < 8; i++) {
        parity += (c >> i) & 1;
    }

    // The parity must be odd...
    return parity % 2 != 0;
}
static bool verify_lrc(short start, short length)
{
    unsigned char parity_bit = (track == 1 ? 7 : 5);

    // Count the number of ones per column (ignoring parity bits)...
    for (short i = 0; i < (parity_bit - 1); i++) {
        short parity = 0;

        for (short j = i; j < length; j += parity_bit) {
            parity += bits_get(start + j);
        }

        // Even parity is what we want...
        if (parity % 2 != 0) {
            return false;
        }
    }

    return true;
}
static short find_sentinel(unsigned char pattern)
{
    unsigned char bit_accum = 0;
    unsigned char bit_length = (track == 1 ? 7 : 5);

    for (short i = 0; i < num_bits; i++) {
        bit_accum >>= 1;                               // rotate the bits to the right...
        bit_accum |= bits_get(i) << (bit_length - 1);  // ...and add the current bit
    
        // Stop when the start sentinel pattern is found...
        if (bit_accum == pattern) {
            return i - (bit_length - 1);
        }
    }

    // No start sentinel was found...
    return -1;
}
static short decode_bits(char *data, unsigned char size) {
    short bit_count = 0;
    unsigned char chars = 0;
    unsigned char bit_accum = 0;
    unsigned char bit_length = (track == 1 ? 7 : 5);
    
    short bit_start = find_sentinel(track == 1 ? 0x45 : 0x0b);
    if (bit_start < 0) {  // error, start sentinel not found
        return -1;
    }

    for (short i = bit_start; i < num_bits; i++) {
        bit_accum >>= 1;                                 // rotate the bits to the right...
        bit_accum |= (bits_get(i) << (bit_length - 1));  // ...and add the current bit
    
        bit_count++;
    
        if (bit_count % bit_length == 0) {
            if (chars >= size) {  // error, the buffer is too small
                return -1;
            }

            // A null means we reached the end of the data...
            if (bit_accum == 0) {
                break;
            }
           
            // The parity must be odd... 
            if (!verify_parity(bit_accum)) {
                return -1;
            }

            // Remove the parity bit...
            bit_accum &= ~(1 << (bit_length - 1));

            // Convert the character to ASCII...
            data[chars] = bit_accum + (track == 1 ? 0x20 : 0x30);
            chars++;
     
            // Reset...
            bit_accum = 0;
        }
    }
  
    // Turn the data into a null-terminated string...
    data[chars] = '\0';
  
    if (data[chars - 2] != '?') {  // error, the end sentinel is not in the right place
        return -1;
    }

    // Verify the LRC (even parity across columns)...
    if (!verify_lrc(bit_start, chars * bit_length)) {
        return -1;
    }

    return chars;
}

static void bits_set(short index, bool bit)
{
    volatile unsigned char *b = &bits[index / 8];
    unsigned char m = 1 << (index % 8);

    *b = bit ? (*b | m) : (*b & ~m);
}
static bool bits_get(short index)
{
    return bits[index / 8] & (1 << (index % 8));
}
static void handle_data()
{
    next_bit = !next_bit;
}
static void handle_clock()
{
    // Avoid a crash in case there are too many bits (garbage)...
    if (num_bits >= BIT_BUFFER_LEN) {
        return;
    }

    bits_set(num_bits, next_bit);
    num_bits++;
}


/* vim: set expandtab ts=4 sw=4: */
