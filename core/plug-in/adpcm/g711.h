#ifndef _G711_H_
#define _G711_H_

// The linear PCM codes are signed 16 bit values

// G.711 A-law
unsigned char linear2alaw(short int pcm_val);
short int alaw2linear(unsigned char a_val);

// G.711 u-law
unsigned char linear2ulaw(short int pcm_val);
short int ulaw2linear(unsigned char u_val);

// A-law <-> u-law conversions
unsigned char alaw2ulaw(unsigned char aval);
unsigned char ulaw2alaw(unsigned char uval);

#endif
