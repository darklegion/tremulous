// This file is part of Tremulous.
// Copyright © 2016 Victor Roemer (blowfish) <victor@badsec.org>
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

// It's quite possible this is _way over the top_ but I've been toying
// with several ideas. -Victor

#ifndef __cplusplus
#error __file__ " is only available to C++"
#endif

#ifndef SCRIPT_RESTCLIENT_H
#define SCRIPT_RESTCLIENT_H

#include <iostream>
#include "restclient/restclient.h"

namespace sol
{ 
    class state;
};

namespace script
{
    // No Exceptions at this time

    struct HttpClient
    { };

    namespace http_client
    {
        //using namespace RestClient;
        static inline void init(sol::state&& lua)
        {

            lua.new_usertype<RestClient::Response>( "HttpResponse",
                    "code", &RestClient::Response::code,
                    "body", &RestClient::Response::body
                    //"headers", &RestClient::Response::headers
                );

            lua.new_usertype<HttpClient>( "http",
                    "get", &RestClient::get,
                    "post", &RestClient::post,
                    "put", &RestClient::put,
                    "delete", &RestClient::del
                );
        }
    };
};

#endif
