
# Tremulous Particle System

Files matching the pattern `scripts/*.particle` are loaded as particle system description files.
Each `.particle` file can contain an arbitrary number of discrete particle systems, much like a `.shader` file can house many shaders.
A particle system is declared by a name followed by curly braces within which the functionality of the particle system is defined.

For example:

```
aShinyNewParticleSystem { }
```

Inside the particle system declaration are placed UP TO *four* particle ejectors.
Ejectors are identified by the keyword ejector and curly braces:

```
aShinyNewParticleSystem
{
     ejector { }

     ejector { }

     thirdPersonOnly
}
```

The `thirdPersonOnly` keyword may be used to specify that the particle system is not visible from the first person if it relates to that client.

The role of the particle ejector is to create some number of new particles at a defined rate.
 These attributes are controlled by the following parameters:

- count `<number>|infinite` - the number of particles this ejector will spawn.
- delay `<msec>` - the delay in msec before the ejector starts spawning.
- period `<initial> <final> <variance>` - the period between particle ejections.

It is perfectly acceptable to have an initial period of zero.
In this case the number of particles specified by the count keyword will be ejected at once.
It is not permissible to have count infinite and a period of zero for obvious reasons.

At ejection time each ejector creates up to four new particles based on templates.
These are specified in the ejector section using the particle keyword:

```
aShinyNewParticleSystem
{
  ejector
  {
    particle { }

    particle { }

    count 50
    delay 0
    period 0 - 0
  }
}
```

Each particle template has a number of attributes:

- `shader <fps>|sync <shader1 <shader2> ... <shaderN>` - this specifies the shaders to use for the particle. The frame rate can be set to a static rate or the sync parameter can be used in which case the frame rate will be synchronised to the lifetime of the particle such that the first frame is displayed on birth and the last frame is displayed immediately before death.
- `model <model1> <model2> ... <modelN>` - use one of the specified models as the particle. This cannot be used in conjunction with the shader keyword.
- `modelAnmation <firstFrame> <numFrames> <loopFrames> <fps>|sync` - animation parameters to use when model particles are employed.
- `displacement <x> <y> <z> <variance>` - a static displacement about the attachment point. The variance parameter specifies a random displacement in all axes.
- `normalDisplacement <displacement>` - for particle systems that have their normal set (impact particle systems for example) this specifies the magnitude of a displacement along the normal.
- `velocityType static|static_transform|tag|cent|normal` - this specifies how the particle will compute its initial velocity. `static` means it is specified statically in the `.particle` file, `static_transform` means the same, except that it is transformed by the orientation matrix of what it is attached to, `tag` means the velocity is in the direction of the tag it is attached to, `cent` means the velocity is in the direction of the cent it is attached to and `normal` means the velocity is in the direction of the particle system normal.
- `velocityDir linear|point` - this specifies whether the initial velocity is computed as a simple direction or as the direction towards a secondary point (defined by `velocityPoint` or dynamically through `velocityType cent`).
- `velocity <x> <y> <z> <variance>` - for when `velocityType static` is present this specifies the direction. The variance here is specified in degrees e.g. `~5` - up to 5 degrees deviation.
- `velocityMagnitude <magnitude>` - the magnitude of the velocity.
- `velocityPoint <x> <y> <z> <variance>` - for when `velocityType static` and `velocityDir point` are present this specifies the point to move towards.
- `parentVelocityFraction <fraction>` - for when the particle system is attached to a cent this specifies the fraction of the cent’s velocity that is added to the particle’s velocity.
- `accelerationType static|static_transform|tag|cent|normal` - this specifies how the particle will compute its acceleration. `static` means it is specified statically in the `.particle` file, `static_transform` means the same, except that it is transformed by the orientation matrix of what it is attached to, `tag` means the acceleration is in the direction of the tag it is attached to, `cent` means the acceleration is in the direction of the cent it is attached to and `normal` means the acceleration is in the direction of the particle system normal.
- `accelerationDir linear|point` - this specifies whether the acceleration is computed as a simple direction or as the direction towards a secondary point (defined by `accelerationPoint` or dynamically through `accelerationType cent`).
- `acceleration <x> <y> <z> <variance>` - for when accelerationType static is present this specifies the direction. The variance here is specified in degrees e.g. `~5` - up to 5 degrees deviation.
- `accelerationMagnitude <magnitude>` - the magnitude of the acceleration.
- `accelerationPoint <x> <y> <z> <variance>` - for when accelerationType static and accelerationDir point are present this specifies the point to move towards.
- `bounce <fraction>|cull` - the fraction of velocity that is refiected when a particle collides. If this is set to `0.0` the particle won't collide. When cull is used particles are culled as soon as they collide with objects.
- `bounceMark <count> <radius> <shader>` - make a mark at each bounce point for up to `<count>` bounces.
- `bounceSound <count> <sound>` - make a sound at each bounce point for up to `<count>` bounces.
- `dynamicLight <delayRadius> <initialRadius> <finalRadius> { <r> <g> <b> }` - attach a dynamic light to this particle.
- `color <delay> { <ir> <ig> <ib> } { <fr> <fg> <fb> }` - color the particle where `<i.>` refers to the initial color component and `<f.>` refers to the final color component.
- `overdrawProtection` - cull particles that occupy a large amount of screen space.
- `realLight` - light particles using the lightgrid instead of fullbright.
- `cullOnStartSolid` - cull particles that are spawned inside brushes.
- `radius <delay> <initial> <final>` - the radius of the particle throughout its lifetime. The delay parameter specifies the time in msec before radius scaling begins. The initial and final parameters specify the radii of the particle in quake units.
- `alpha <delay> <initial> <final>` - the alpha of the particle throughout its lifetime. The delay parameter specifies the time in msec before alpha scaling begins. The initial and final parameters specify the alpha of the particle where `1.0` is totally opaque and `0.0` is totally transparent.
- `rotation <delay> <initial> <final>` - the rotation of the particle throughout its lifetime. The delay parameter specifies the time in msec before the rotation begins. The initial and final parameters specify the rotation of the particle in degrees.
- `lifeTime <time>` - the lifetime of the particle.
- `childSystem <particle system>` - specifies a particle system to attach to this particle.
- `childTrailSystem <trail system>` - specifies a trail system to attach to this particle.
- `onDeathSystem <particle system>` - specifies a particle system to spawn at the point where this particle died.
- `physicsRadius` - ???
- `scaleWithCharge` - ???

Except for vector components, shader fps ... and `period <initial><final> <variance>`, every value can be specified with a random variance.
The syntax for this is as follows:

```
 [value][variance[%]]
```
So the following forms are possible, where random is a random number between `0.0` and `1.0` inclusive:

```
   5.0      // 5.0

   5.0~8.0  // 5.0 + ( random * 8.0 )

   5.0~200% // 5.0 + ( random * 5.0 * 200% )

   ~7.0     // random * 7.0
```

This allows for relatively fiexible randomisation of most of the particle’s parameters. For parameters taking an initial and final value, specifying the final value as '`-`' will result in a final value the same as the initial value.

For the purposes of map based particle systems using `misc_particle_system` it is safe to ignore `velocityType` and `accelerationType tag|cent|normal`, `normalDisplacement` and `parentVelocityFraction` altogether.

Of course, it is not necessary to specify every parameter documented here for every particle system.
If a parameter is not included it will usually default to zero.
C/C++ style comments can be used throughout.
There are an enormous number of possible combinations of particle systems parameters and as such it is impractical to test them all.
For this reason it is possible that certain permutations do not behave as expected or wrongly.
In this case you may have discovered a bug - let us know.
Having said this when you’re having problems with a particle system make sure you scroll up the console and check that it compiled OK, I’ve written the parser to be very intolerant of error.

Here is an example particle system:

```
aShinyNewParticleSystem
{
  ejector
  {
    particle
    {
      shader sync shader1 shader2

      velocityType static
      velocityDir linear
      velocityMagnitude 200
      velocity 0 0 1 ~30

      accelerationType static
      accelerationDir linear
      accelerationMagnitude 50
      acceleration 0 0 1 ~0

      radius 0 10.0 50.0
      alpha 0 1.0 1.0
      rotation 0 ~360 -
      bounce 0.4

      lifeTime 1500
    }

    count 50
    delay 0
    period 0 - 0
  }
}
```
