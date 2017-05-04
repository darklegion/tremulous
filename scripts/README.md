# Lua Scripts

Scripts packaged with Tremulous. This folder will likely fill up with
not so useful examples.

Even more likely this will be the only source of documentation in the
short term. Likely documentation will happen after the first large API
re-organization.


Available APIs 
==============

HTTP Client
-----------
   - `http.del()`
   - `http.get()`
   - `http.post()`
   - `http.put()`

   Returns a `HttpResponse` object.

```lua
   HttpResponse::code -- HTTP status code from server
   HttpResponse::body -- Raw response body
```

### Example 
```lua
    print(http.get('www.google.com/search?q=' .. 'Hello World').body)
```

JSON
----

  - `rapidjson.encode()`
  - `rapidjson.decode()`

Cvar
----

  - `cvar.new(a, b, c)`
  - `cvar.new(a, b)`
  - `cvar.new(a)`

Nettle
------
TBD

Client
------
TBD
