

cl_latestRelease = cvar.new("cl_latestRelease") 

dlurl = ""

Releases = {
  url='https://api.github.com/repos/GrangerHub/tremulous/releases'

  refresh = function()
    r = http.get(url)
    if r.code != 200 then
      cl_latestRelease = "ERROR:\n Server did not return OK status code"
      return false
    end
    releases = rapidjson.decode(r.body)
    most_recent = releases[1]
    cl_latestRelease = most_recent.tag;
    for i,asset in ipairs(most_recent.assets) do
      dlurl = cvar.new("download_url", "", 256)
      dlurl = asset.browser_download_url
    end
    return true
  end
  
  download = function()
    r = http.get(download_url)
    if  r.code != 200 then
      cvar.new("com_error") = "Download failed"
      return false
    end
  
    io.open(path, "w+")
    io.write(r.body)
    io.close()
  
    args = "path-to-tremulous-binary"
    os.execute(path .. args)
  end
}
