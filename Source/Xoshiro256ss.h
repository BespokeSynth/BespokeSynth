/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/

// originally from Quuxplusone/Xoshiro256ss.
// Upstream's public domain, so I assume all's ok.
// Original comment:
// The "xoshiro256** 1.0" generator.
// C++ port by Arthur O'Dwyer (2021).
// Based on the C version by David Blackman and Sebastiano Vigna (2018),
// https://prng.di.unimi.it/xoshiro256starstar.c

#ifndef BESPOKESYNTH_XOSHIRO256SS_H
#define BESPOKESYNTH_XOSHIRO256SS_H

#include <cstddef>
#include <cstdint>
#include <climits> // CHAR_BIT
#include <random>

namespace bespoke::core
{
   namespace detail
   {
      template <class T>
      constexpr size_t BitSizeOf()
      {
         return sizeof(T) * CHAR_BIT;
      }

      constexpr std::uint64_t splitmix64(std::uint64_t x)
      {
         std::uint64_t z = (x += 0x9e3779b97f4a7c15uLL);
         z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9uLL;
         z = (z ^ (z >> 27)) * 0x94d049bb133111ebuLL;
         return z ^ (z >> 31);
      }

      constexpr std::uint64_t rotl(std::uint64_t x, int k)
      {
         return (x << k) | (x >> (BitSizeOf<std::uint64_t>() - k));
      }

      /**
       * Xoshiro256** as a C++ RandomNumberEngine,
       * which can be used in C++ random algorithms.
       */
      struct Xoshiro256ss
      {
         using result_type = std::uint64_t;

         std::uint64_t s[4]{};

         constexpr explicit Xoshiro256ss()
         : Xoshiro256ss(0)
         {
         }

         constexpr explicit Xoshiro256ss(std::uint64_t seed)
         {
            s[0] = splitmix64(seed);
            s[1] = splitmix64(seed);
            s[2] = splitmix64(seed);
            s[3] = splitmix64(seed);
         }

         /**
          * Constructor for seeding from a `std::random_device`.
          *
          * \param[in] rd Random device to seed this engine with.
          */
         constexpr explicit Xoshiro256ss(std::random_device& rd)
         {
            // Get 64 bits out of the random device.
            //
            // This lambda is quite literal, as it fetches
            // 2 iterations of the random engine, and
            // shifts + OR's them into a 64bit value.
            auto get_u64 = [&rd]
            {
               std::uint64_t the_thing = rd();
               return (the_thing << 32) | rd();
            };

            // seed with 256 bits of entropy from the random device + splitmix64
            // to ensure we seed it well, as per recommendation.
            s[0] = splitmix64(get_u64());
            s[1] = splitmix64(get_u64());
            s[2] = splitmix64(get_u64());
            s[3] = splitmix64(get_u64());
         }

         static constexpr result_type min()
         {
            return 0;
         }

         static constexpr result_type max()
         {
            return std::uint64_t(-1);
         }

         constexpr result_type operator()()
         {
            result_type result = rotl(s[1] * 5, 7) * 9;
            result_type t = s[1] << 17;
            s[2] ^= s[0];
            s[3] ^= s[1];
            s[1] ^= s[2];
            s[0] ^= s[3];
            s[2] ^= t;
            s[3] = rotl(s[3], 45);
            return result;
         }
      };
   }

   using detail::Xoshiro256ss;
}

#endif //BESPOKESYNTH_XOSHIRO256SS_H
