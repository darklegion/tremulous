# Tremulous Voice Commands

Files matching the pattern `voice/*.voice` are loaded automatically as "voices". 
Each `.voice` file can contain an arbitrary number of voice commands.

The most important voice is the `default` or `default.voice`.
Every player by default is assigned the `default` voice. 

A player can change they're voice by setting the `voice` userinfo cvar. 

```
/setu voice bender
```

## Voice File Format 

```
hailking
{
  "sound/player/ash/voice/aod-hailking.wav"
  {
    team 2
    class 1
    weapon 1
    text "Hail to the king"
    enthusiasm 0
  }
}
```

- "hailking" is the voice command. (i.e., `/vsay "hailking"`). Note that the maximum command length is 16 characters.
- "sound/player/ash/voice/aod-hailking.wav" is the 1st "track".
- "team" corresponds to the `team_t` enum in `bg_public.h`.
- "class" corresponds to the `class_t` enum in `bg_public.h`.
- "weapon" corresponds to the `weapon_t` enum in `bg_public.h`.
- "text" is the text that will appear in chat when the voice command is issued.
- "enthusiasm" needs to be tested


I think "text" is the only *required* field.
