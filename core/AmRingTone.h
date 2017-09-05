#ifndef _AMRINGTONE_H_
#define _AMRINGTONE_H_

#include "AmAudio.h"

/** \brief audio device that generates ring tones with uesr specified period, f
 * and f2 */
class AmRingTone : public AmAudio
{
  int on_period;  // ms
  int off_period; // ms
  int freq;       // Hz
  int freq2;      // Hz

  int length;

 public:
  AmRingTone(int length, int on, int off, int f, int f2 = 0);
  ~AmRingTone();

  int read(unsigned int user_ts, unsigned int size);
  int write(unsigned int user_ts, unsigned int size);
};

#endif
