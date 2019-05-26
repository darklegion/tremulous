//
// This file is part of Tremulous.
// Copyright © 2017 Victor Roemer (blowfish) <victor@badsec.org>
// Copyright (C) 2015-2019 GrangerHub
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#pragma once

#include <iostream>

using std::string;

typedef char fingerprint_t[64];
typedef char guid_t[33];
typedef char name_t[MAX_NAME_LENGTH];
typedef char err_t[MAX_STRING_CHARS];

struct AdminFlag {
    unsigned id;
    name_t name;
};

struct AdminLevel {
    name_t name;
    admin_flags_t flags;
    unsigned level;
};

struct Admin {
    bool flag(const name_t flagname)
    { }

    bool deny(const name_t flagname)
    { }

    guid_t guid;
    name_t name;

private:
    admin_flags_t flags;
    admin_flags_t denied;
    unsigned level;
};

class AdminMgr {
    bool add(Admin&);
    bool remove(Admin&);
}
