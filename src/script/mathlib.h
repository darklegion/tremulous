// This file is part of Tremulous.
// Copyright © 2017 Victor Roemer (blowfish) <victor@badsec.org>
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.

// It's quite possible this is _way over the top_ but I've been toying
// with several ideas. -Victor

#ifndef SCRIPT_MATHLIB_H
#define SCRIPT_MATHLIB_H

#include <iostream>

#include "qcommon/mathlib.h"

namespace script
{
    class Vec3 {
        public:
            Vec3() {}
            Vec3(float x, float y, float z = 0)
            {
                vec[0] = x;
                vec[1] = y;
                vec[2] = z;
            }

            Vec3(vec3_t v)
            {

                vec[0] = v[0];
                vec[1] = v[1];
                vec[2] = v[2];
            }

            float get_x() { return vec[0]; }
            float get_y() { return vec[1]; }
            float get_z() { return vec[2]; }
            void set_x(float x) { vec[0] = x; }
            void set_y(float y) { vec[1] = y; }
            void set_z(float z) { vec[2] = z; }
            vec3_t vec;
    };

    class Vec4 {
        public:
            Vec4() {}

            Vec4(float a, float b, float c, float d)
            {
                vec[0] = a;
                vec[1] = b;
                vec[2] = c;
                vec[3] = d;
            }

            Vec4(vec4_t v)
            {
                vec[0] = v[0];
                vec[1] = v[1];
                vec[2] = v[2];
                vec[3] = v[3];
            }

            float get_a() { return vec[0]; }
            float get_b() { return vec[1]; }
            float get_c() { return vec[2]; }
            float get_d() { return vec[3]; }
            void set_a(float a) { vec[0] = a; }
            void set_b(float b) { vec[1] = b; }
            void set_c(float c) { vec[2] = c; }
            void set_d(float d) { vec[3] = d; }
            vec4_t vec;
    };


    namespace mathlib
    {
        static inline void init(sol::state&& lua)
        {

            lua.new_usertype<Vec3>("vec3_t",
                    sol::constructors<sol::types<>, sol::types<float, float, float>>(),
                    "x", sol::property(&Vec3::get_x, &Vec3::set_x),
                    "y", sol::property(&Vec3::get_y, &Vec3::set_y),
                    "z", sol::property(&Vec3::get_z, &Vec3::set_z));

            lua.new_usertype<Vec4>("vec4_t",
                    sol::constructors<sol::types<>, sol::types<float, float, float, float>>(),
                    "a", sol::property(&Vec4::get_a, &Vec4::set_a),
                    "b", sol::property(&Vec4::get_b, &Vec4::set_b),
                    "c", sol::property(&Vec4::get_c, &Vec4::set_c),
                    "d", sol::property(&Vec4::get_d, &Vec4::set_d));
        }
    }
}

#endif
