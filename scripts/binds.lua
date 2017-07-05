--[[ 
-- Key binds specified in Lua
--]]


player = {
    team    = cvar.new('team_teamname'),
    stage   = cvar.new('team_stage'),
    hp      = cvar.new('player_hp'),
    kns     = cvar.new('team_kns'),
    spawns  = cvar.new('team_spawns'),
    bp      = cvar.new('team_bp'),
    maxbp   = cvar.new('player_maxbp'),
    credits = cvar.new('player_credits'),
    score   = cvar.new('player_score'),
    deaths  = cvar.new('player_deaths')
}

alien = {
    kns     = cvar.new('alien_kns'),
    score   = cvar.new('alien_score')
}

human = {
    kns     = cvar.new('human_kns'),
    score   = cvar.new('human_score')
}

function TeamSay(text)
    client.addReliableCommand('say_team ' .. text)
end

function Screenshot()
    draw2D = cvar.new('cg_draw2d')
    drawGun = cvar.new('cg_drawgun')
    _a = draw2D.value
    _b = drawGun.value

    draw2D.value = 0
    drawGun.value = 0

    client.addReliableCommand('wait 2; screenshotJPEG silent')

    draw2D.value = _a
    drawGun.value = _b
end

binds = { 
    a = bind.new('a', "teamstatus"),
    b = bind.new('b', "script TeamSay('^5Humans have ^1' .. human.score.value .. ' ^2Aliens have ^1' .. alien.score.value)"),
    c = bind.new('c', "script TeamSay('^5MOVE!!! I only have ^1' .. player.hp.value .. '^5HP Available')"),
    d = bind.new('d'),
    e = bind.new('e'),
    f = bind.new('f'),
    g = bind.new('g'),
    h = bind.new('h', "buy ammo"),
    i = bind.new('i', "+forward"),
    j = bind.new('j', "+moveleft"),
    k = bind.new('k', "+back"),
    l = bind.new('l', "+moveright"),
    m = bind.new('m', "itemact medkit"),
    n = bind.new('n', "+button8"),
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
    z = bind.new('z', "screenshotJPEG silent"),

    f12 = bind.new('f12', "cg_draw2d ")
} 
