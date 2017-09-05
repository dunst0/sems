/*
 * Copyright (C) 2002-2003 Fhg Fokus
 *
 * This file is part of SEMS, a free SIP media server.
 *
 * SEMS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * For a license to use the sems software under conditions
 * other than those described here, or to purchase support for this
 * software, please contact iptel.org by e-mail at the following addresses:
 *    info@iptel.org
 *
 * SEMS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _SAMPLEARRAY_H_
#define _SAMPLEARRAY_H_

/* MUST be a power of 2 */
#define SIZE_MIX_BUFFER (1 << 14)

/** \brief comparator for user timestamps */
struct ts_less
{
  inline bool operator()(const unsigned int& l, const unsigned int& r) const
  {
    return (l - r > (unsigned int) (1 << 31));
  }
};

/** \brief comparator for system timestamps
 * Note that system timestamps overflow at 48 bit boundaries.
 */
struct sys_ts_less
{
  inline bool operator()(const unsigned long long int& l,
                         const unsigned long long int& r) const
  {
    return (((l - r) & 0xFFFFFFFFFFFFLL) > (1LL << 47));
  }
};

/** \brief timed array of samples */
template <typename T> class SampleArray
{
 public:
  T            samples[SIZE_MIX_BUFFER];
  unsigned int last_ts;
  bool         init;

  SampleArray()
      : init(false)
  {
  }

  void clear_all() { memset(samples, 0, sizeof(samples)); }

  void clear(unsigned int start_ts, unsigned int end_ts)
  {
    if (end_ts - start_ts >= SIZE_MIX_BUFFER) {
      clear_all();
      return;
    }

    unsigned int start_off = start_ts & (SIZE_MIX_BUFFER - 1);
    unsigned int end_off   = end_ts & (SIZE_MIX_BUFFER - 1);

    T* sp = samples + start_off;
    if (start_off < end_off)
      memset(sp, 0, (end_off - start_off) * sizeof(T));
    else {
      memset(sp, 0, (SIZE_MIX_BUFFER - start_off) * sizeof(T));
      memset(samples, 0, end_off * sizeof(T));
    }
  }

  void write(unsigned int ts, T* buffer, unsigned int size)
  {
    unsigned int off = ts & (SIZE_MIX_BUFFER - 1);

    T* sp = samples + off;
    if (off + size <= SIZE_MIX_BUFFER)
      memcpy(sp, buffer, size * sizeof(T));
    else {
      unsigned int s = SIZE_MIX_BUFFER - off;
      memcpy(sp, buffer, s * sizeof(T));

      buffer += s;
      size -= s;
      memcpy(samples, buffer, size * sizeof(T));
    }
  }

  void read(unsigned int ts, T* buffer, unsigned int size)
  {
    unsigned int off = ts & (SIZE_MIX_BUFFER - 1);

    T* sp = samples + off;
    if (off + size <= SIZE_MIX_BUFFER) {
      memcpy(buffer, sp, size * sizeof(T));
    }
    else {
      unsigned int s = SIZE_MIX_BUFFER - off;
      memcpy(buffer, sp, s * sizeof(T));

      buffer += s;
      ;
      size -= s;
      memcpy(buffer, samples, size * sizeof(T));
    }
  }

  /**
   * @param size buffer size in [samples].
   */
  void put(unsigned int ts, T* buffer, unsigned int size)
  {
    // assert(size <= SIZE_MIX_BUFFER);

    if (!init) {
      clear_all();
      last_ts = ts;
      init    = true;
    }

    if (ts_less()(ts, last_ts - SIZE_MIX_BUFFER)) {
      DBG("throwing away too old packet (ts=%u; last_ts=%u).\n", ts, last_ts);
      return;
    }

    if (ts_less()(last_ts, ts)) clear(last_ts, ts);

    write(ts, buffer, size);
    if (ts_less()(last_ts, ts + size)) last_ts = ts + size;
  }

  /**
   * @param buf_size buffer size in [samples].
   */
  void get(unsigned int ts, T* buffer, unsigned int size)
  {
    // assert(size <= SIZE_MIX_BUFFER);

    if (!init || !ts_less()(ts, last_ts)
        || !ts_less()(last_ts - SIZE_MIX_BUFFER, ts + size)) {
      // !init ||
      // (ts+size <= last_ts-SIZE_MIX_BUFFER) ||
      // (ts >= last_ts)
      memset(buffer, 0, size * sizeof(T));
      return;
    }

    // init &&
    // (ts+size > last_ts-SIZE_MIX_BUFFER) &&
    // (ts < last_ts)

    if (ts_less()(ts, last_ts - SIZE_MIX_BUFFER)) {
      // ts < last_ts-SIZE_MIX_BUFFER
      unsigned int s = last_ts - SIZE_MIX_BUFFER - ts;
      memset(buffer, 0, s * sizeof(T));

      ts += s;
      buffer += s;
      size -= s;
      read(ts, buffer, size);
    }
    else if (ts_less()(last_ts, ts + size)) {
      // ts+size > last_ts
      unsigned int s = last_ts - ts;
      read(ts, buffer, s);

      buffer += s;
      size -= s;
      memset(buffer, 0, size * sizeof(T));
    }
    else {
      // ts+size <= last_ts
      read(ts, buffer, size);
    }
  }
};

// 32 bit sample
typedef int IntSample;

// 32 bit sample array
typedef SampleArray<IntSample> SampleArrayInt;

// 16 bit sample
typedef short int ShortSample;

// 16 bit sample array
typedef SampleArray<ShortSample> SampleArrayShort;

#endif
