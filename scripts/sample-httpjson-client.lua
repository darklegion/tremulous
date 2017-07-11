--[[
 _____                         _                       _
|_   _| __ ___ _ __ ___  _   _| | ___  _   _ ___      | |   _   _  __ _
  | || '__/ _ \ '_ ` _ \| | | | |/ _ \| | | / __|_____| |  | | | |/ _` |
  | || | |  __/ | | | | | |_| | | (_) | |_| \__ \_____| |__| |_| | (_| |
  |_||_|  \___|_| |_| |_|\__,_|_|\___/ \__,_|___/     |_____\__,_|\__,_|
  Victor Roemer [WTFBBQHAX]
  Nov. 03, 2016 10:53:27AM EST
  A sample Restful JSON Client API.
  This sample demonstrates how to comunicate with an HTTP JSON 3rdparty
  API in Lua script.
  APIs demonstrated:
    * HTTP RestClient
    * JSON
--]]

-- GitHub API URL
url='https://api.github.com/repos/GrangerHub/tremulous/releases'

-- HTTP Get request- retrieve the raw JSON response
txt = http.get(url)
assert(txt.code == 200)

-- Decode raw JSON response into a Lua table
releases = rapidjson.decode(txt.body)

-- GitHub returned an array of releases- The "most_recent" is item 1
-- NOTE: Lua array indexing starts at `1` not `0`!!
most_recent = releases[1]

-- FIXME: Remove hardcoded tag_name in this test
assert(most_recent.tag_name == "Oct-22-2016")


for i,asset in ipairs(most_recent.assets) do
    print(asset.browser_download_url)
end
