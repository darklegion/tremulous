#include "g_lua.h"

#include "sys/sys_shared.h"

//#define LUA_BUILD_AS_DLL
#include "lua.hpp"
#include "sol.hpp"

#include "g_local.h"
#include "g_spawn.h"

#include "script/mathlib.h"
using namespace script;

#include <iostream>

sol::state *lua = nullptr;

class GEntity {
public:
    GEntity() { ent = G_Spawn(); }
    GEntity(gentity_t* _ent) : ent(_ent) {}

    static GEntity* find(GEntity* _from, std::string match)
    {
        gentity_t* from = _from ? _from->ent : nullptr;
        gentity_t* ent = G_Find(from, FOFS(classname), match.c_str());
        if (ent)
        {
            return new GEntity(ent);
        }

        return nullptr;
    }

    static void call_spawn(GEntity* me)
    {
        if (me)
            G_CallSpawn(me->ent);
    }

    // Member functions
    void die(GEntity* _inflictor, GEntity* _attacker, int damage, int mod)
    {
        gentity_t* in = _inflictor ? _inflictor->ent : nullptr;
        gentity_t* at = _attacker ? _attacker->ent : nullptr;
        ent->die(ent, ent, ent, damage, mod);
    }

    void link_entity()
    {
        G_LinkEntity(ent);
    }

    void unlink_entity()
    {
        G_UnlinkEntity(ent);
    }

    void free_entity()
    {
        G_FreeEntity(ent);
        ent = nullptr;
    }

    //
    // Properties
    //

    // acceleration
    void set_acceleration(Vec3 vec)
    {
        ent->acceleration[0] = vec.vec[0];
        ent->acceleration[1] = vec.vec[1];
        ent->acceleration[2] = vec.vec[2];
    }
    Vec3 get_acceleration()
    {
        Vec3 r(ent->acceleration);
        return r;
    }

    // alpha
    void set_alpha(Vec3 vec)
    {
        ent->pos1[0] = vec.vec[0];
        ent->pos1[1] = vec.vec[1];
        ent->pos1[2] = vec.vec[2];
    }
    Vec3 get_alpha()
    {
        Vec3 r(ent->pos1);
        return r;
    }

    // angle
    void set_angle(float f)
    {
        ent->s.pos.trBase[0] = 0;
        ent->s.pos.trBase[1] = f;
        ent->s.pos.trBase[2] = 0;
    }
    float get_angle() { return ent->s.pos.trBase[1]; }

    // angles
    void set_angles(Vec3 vec)
    {
        ent->s.pos.trBase[0] = vec.vec[0];
        ent->s.pos.trBase[1] = vec.vec[1];
        ent->s.pos.trBase[2] = vec.vec[2];
    }
    Vec3 get_angles()
    {
        Vec3 r(ent->s.pos.trBase);
        return r;
    }

    // animation
    void set_animation(Vec4 vec)
    {
        ent->animation[0] = vec.vec[0];
        ent->animation[1] = vec.vec[1];
        ent->animation[2] = vec.vec[2];
        ent->animation[3] = vec.vec[3];
    }
    Vec4 get_animation()
    {
        Vec4 r(ent->animation);
        return r;
    }

    // bounce
    void set_bounce(float f) { ent->physicsBounce = f; }
    float get_bounce() { return ent->physicsBounce; }

    // classname
    void set_classname(std::string name)
    {
        ent->classname = strdup(name.c_str());
        ent->_classname_alloced = true;
    }
    std::string get_classname() { return ent->classname; }

    // count
    void set_count(int count) { ent->count = count; }
    int get_count() { return ent->count; }

    // damage
    void set_damage(int damage) { ent->damage = damage; }
    int get_damage() { return ent->damage; }

    // health
    void set_health(int health) { ent->health = health; }
    int get_health() { return ent->health; }


    // message
    void set_message(std::string name)
    {
        ent->message = strdup(name.c_str());
        ent->_message_alloced = true;
    }
    std::string get_message() { return ent->message; }

    // model
    void set_model(std::string name)
    {
        ent->model = strdup(name.c_str());
        ent->_model_alloced = true;
    }
    std::string get_model()
    {
        if (ent->model)
            return ent->model;
        return "";
    }

    // model2
    void set_model2(std::string name)
    {
        ent->model2 = strdup(name.c_str());
        ent->_model2_alloced = true;
    }
    std::string get_model2()
    {
        if (ent->model2)
            return ent->model2;
        return "";
    }
    // origin
    void set_origin(Vec3 vec)
    {
        ent->s.pos.trBase[0] = vec.vec[0];
        ent->s.pos.trBase[1] = vec.vec[1];
        ent->s.pos.trBase[2] = vec.vec[2];

        ent->s.pos.trType = TR_STATIONARY;
        ent->s.pos.trTime = 0;
        ent->s.pos.trDuration = 0;
        VectorClear(ent->s.pos.trDelta);

        ent->r.currentOrigin[0] = vec.vec[0];
        ent->r.currentOrigin[1] = vec.vec[1];
        ent->r.currentOrigin[2] = vec.vec[2];
    }
    Vec3 get_origin()
    {
        Vec3 r(ent->s.pos.trBase);
        return r;
    }

    // radius
    void set_radius(Vec3 vec)
    {
        ent->pos2[0] = vec.vec[0];
        ent->pos2[0] = vec.vec[1];
        ent->pos2[0] = vec.vec[2];
    }
    Vec3 get_radius()
    {
        Vec3 r(ent->pos2);
        return r;
    }

    // random
    void set_random(float f) { ent->random = f; }
    float get_random() { return ent->random; }

    // rotatorAngle
    void set_rotatorAngle(float f) { ent->rotatorAngle = f; }
    float get_rotatorAngle() { return ent->rotatorAngle; }

    // set rotation
    void set_rotation(float a, float b, float c)
    {
        BG_EvaluateTrajectory( &ent->s.apos, level.time, ent->r.currentAngles );
        VectorCopy( ent->r.currentAngles, ent->s.apos.trBase );

        ent->s.apos.trType = TR_LINEAR;
        ent->s.apos.trTime = level.time;

        ent->s.apos.trDelta[0] = a;
        ent->s.apos.trDelta[1] = b;
        ent->s.apos.trDelta[2] = c;
        G_LinkEntity(ent);
    }

    // spawned
    void set_spawned(bool spawn) { ent->spawned = spawn; }
    bool get_spawned() { return ent->spawned; }
 
    // spawnflags
    void set_spawnflags(int spawnflags) { ent->spawnflags = spawnflags; }
    int get_spawnflags() { return ent->spawnflags; }

    // speed
    void set_speed(float f) { ent->speed = f; }
    float get_speed() { return ent->speed; }

    // target
    void set_target(std::string name)
    {
        ent->target = strdup(name.c_str());
        ent->_target_alloced = true;
    }
    std::string get_target() { return ent->target; }

    // targetname
    void set_targetname(std::string name)
    {
        ent->targetname = strdup(name.c_str());
        ent->_targetname_alloced = true;
    }
    std::string get_targetname() { return ent->targetname; }

    // targetShaderName
    void set_targetShaderName(std::string name)
    {
        ent->targetShaderName = strdup(name.c_str());
        ent->_targetShaderName_alloced = true;
    }
    std::string get_targetShaderName() { return ent->targetShaderName; }

    // targetShaderNewName
    void set_targetShaderNewName(std::string name)
    {
        ent->targetShaderNewName = strdup(name.c_str());
        ent->_targetShaderNewName_alloced = true;
    }
    std::string get_targetShaderNewName() { return ent->targetShaderNewName; }

    // wait
    void set_wait(float f) { ent->wait = f; }
    float get_wait() { return ent->wait; }

private:
    gentity_t* ent = nullptr;
};

void Api_Init()
{
    lua = static_cast<sol::state*>(Sys_GetLua());

    sol::table game = lua->create_named_table("game");

    game.new_usertype<GEntity>("gentity_t",
            sol::constructors<sol::types<>>(),
            // Static routine
            "find", &GEntity::find,
            "call_spawn", &GEntity::call_spawn,

            // Member function
            // think
            // reached
            // blocked
            // touch
            // use
            // pain
            "die", &GEntity::die,
            "link_entity", &GEntity::link_entity,
            "unlink_entity", &GEntity::unlink_entity,
            //"set_rotation", &GEntity::set_rotation,

            // Properties available in g_spawn.c
            "acceleration", sol::property(&GEntity::get_acceleration, &GEntity::set_acceleration),
            "alpha", sol::property(&GEntity::get_alpha, &GEntity::set_alpha),
            "angle", sol::property(&GEntity::get_angle, &GEntity::set_angle),
            "angles", sol::property(&GEntity::get_angles, &GEntity::set_angles),
            "animation", sol::property(&GEntity::get_animation, &GEntity::set_animation),
            "bounce", sol::property(&GEntity::get_bounce, &GEntity::set_bounce),
            "classname", sol::property(&GEntity::get_classname, &GEntity::set_classname),
            "count", sol::property(&GEntity::get_count, &GEntity::set_count),
            "damage", sol::property(&GEntity::get_damage, &GEntity::set_damage),
            "health", sol::property(&GEntity::get_health, &GEntity::set_health),
            "model", sol::property(&GEntity::get_model, &GEntity::set_model),
            "model2", sol::property(&GEntity::get_model2, &GEntity::set_model2),
            "origin", sol::property(&GEntity::get_origin, &GEntity::set_origin),
            "radius", sol::property(&GEntity::get_radius, &GEntity::set_radius),
            "random", sol::property(&GEntity::get_random, &GEntity::set_random),
            "rotatorAngle", sol::property(&GEntity::get_rotatorAngle, &GEntity::set_rotatorAngle),
            "spawned", sol::property(&GEntity::get_spawned, &GEntity::set_spawned),
            "spawnflags", sol::property(&GEntity::get_spawnflags, &GEntity::set_spawnflags),
            "speed", sol::property(&GEntity::get_speed, &GEntity::set_speed),
            "target", sol::property(&GEntity::get_target, &GEntity::set_target),
            "targetShaderName", sol::property(&GEntity::get_targetShaderName, &GEntity::set_targetShaderName),
            "targetShaderNewName", sol::property(&GEntity::get_targetShaderNewName, &GEntity::set_targetShaderNewName),
            "targetname", sol::property(&GEntity::get_targetname, &GEntity::set_targetname),
            "wait", sol::property(&GEntity::get_wait, &GEntity::set_wait));
}

void LuaHook_ClientSpawn(GEntity gEnt)
{
}

void Cmd_LuaLoad_f(gentity_t*)
{
    if (Cmd_Argc() < 2)
    {
        G_Printf("usage: lua <name>\n");
        return;
    }

    try
    {
        std::string s = ConcatArgs(1);
        lua->script_file(s);
    }
    catch (sol::error& e)
    {
        Com_Printf(S_COLOR_YELLOW "%s\n", e.what());
    }
}
