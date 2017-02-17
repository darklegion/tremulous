# Trail System
 
Note that as of Tremulous 1.1.0 there is no way to trigger a trail system with map entities.
However, a trail system can be attached to a Particle System with the `childTrailSystem` key.
 
Files matching the pattern `scripts/*.trail` are loaded as trail system description files.
Each `.trail` file can contain an arbitrary number of discrete trail systems, much like a `.shader` file can house many shaders.
A trail system is declared by a name followed by curly braces within which the functionality of the trail system is defined. For example:
 
```
   aShinyNewTrailSystem { } 
```
 
Inside the particle system declaration are placed up to four trail beams. Beams are identified by the keyword beam and curly braces:
 
```
   aShinyNewTrailSystem
   {
 
     beam { }
 
     beam { }
 
     thirdPersonOnly
   } 
```

The `thirdPersonOnly` keyword may be used to specify that the trail system is not visible from the first person if it relates to that client.

A trail beam describes the appearance of one element of the trail system:
 
- `shader <shader>` - the shader to use to texture this beam.
- `segments <number>` - the number of quads that make up the beam.
- `width <frontWidth> <backWidth>` - the width of the beam at the front and back.
- `alpha <frontAlpha> <backAlpha>` - the alpha of the beam at the front and back.
- `color { <fr> <fg> <fb> } { <br> <bg> <bb> }` - the color of the beam at the front and back.
- `segmentTime <time>` - how long a single segment lasts when the trail is only attached at one end.
- `fadeOutTime <time>` - how long this beam takes to fade away.
- `textureType [stretch <frontTC> <backTC>][repeat [front|back] <repeatLength>]` - how to texture the beam. `stretch` causes the texture to be stretched from the front to the back using the specified texture coordinates. `repeat` causes the texture to be repeated over a specified length either from the front or the back.
- `model <model1> <model2> ... <modelN>` - use one of the specified models as the particle. This cannot be used in conjunction with the shader keyword.
- `modelAnmation <firstFrame> <numFrames> <loopFrames> <fps>sync` - animation parameters to use when model particles are employed.
- `realLight` - light particles using the lightgrid instead of fullbright.
- `jitter <magnitude> <period>` - this specifies a random jitter of the position of each beam node by magnitude every period.
- `jitterAttachments` - if this is specified the end points of the beam are jittered as well as the intervening nodes.
