
--[[ 
-- Key binds specified in Lua
--]]


player_hp = cvar.new('player_hp')

function TeamSay(text)
    client.addReliableCommand('say_team ' .. text)
end

k_C = bind.new('c', "script TeamSay('^5MOVE!!! I only have ^1' .. player_hp.value .. '^5HP Available')")
k_A = bind.new('a', "teamstatus")


binds = { 
    a = bind.new('a'),
    b = bind.new('b'),
    c = bind.new('c'),
    d = bind.new('d'),
    e = bind.new('e'),
    f = bind.new('f'),
    g = bind.new('g'),
    h = bind.new('h'),
    i = bind.new('i'),
    j = bind.new('j'),
    k = bind.new('k'),
    l = bind.new('l'),
    m = bind.new('m'),
    n = bind.new('n'),
    o = bind.new('o'),
    p = bind.new('p'),
    q = bind.new('q'),
    r = bind.new('r'),
    s = bind.new('s'),
    t = bind.new('t'),
    u = bind.new('u'),
    v = bind.new('v'),
    w = bind.new('w'),
    x = bind.new('x'),
    y = bind.new('y'),
    z = bind.new('z')
} 
