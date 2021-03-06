#pragma once

#include <rwe/AudioService.h>
#include <rwe/GameTime.h>
#include <rwe/ProjectilePhysicsType.h>
#include <rwe/ProjectileRenderType.h>
#include <rwe/SpriteSeries.h>
#include <rwe/UnitId.h>
#include <rwe/cob/CobThread.h>
#include <rwe/math/Vector3f.h>
#include <variant>

namespace rwe
{
    struct UnitWeaponStateIdle
    {
    };

    using UnitWeaponAttackTarget = std::variant<UnitId, SimVector>;

    struct UnitWeaponStateAttacking
    {
        struct AimInfo
        {
            const CobThread* thread;
            SimAngle lastHeading;
            SimAngle lastPitch;
        };

        /** The target the weapon is currently trying to shoot at. */
        UnitWeaponAttackTarget target;

        std::optional<AimInfo> aimInfo;

        UnitWeaponStateAttacking(const UnitWeaponAttackTarget& target) : target(target)
        {
        }
    };

    using UnitWeaponState = std::variant<UnitWeaponStateIdle, UnitWeaponStateAttacking>;

    struct UnitWeapon
    {
        ProjectilePhysicsType physicsType;

        SimScalar maxRange;

        SimScalar reloadTime;

        bool startSmoke;
        bool endSmoke;

        std::optional<GameTime> smokeTrail;

        std::optional<AudioService::SoundHandle> soundStart;
        std::optional<AudioService::SoundHandle> soundHit;
        std::optional<AudioService::SoundHandle> soundWater;

        std::optional<std::shared_ptr<SpriteSeries>> explosion;
        std::optional<std::shared_ptr<SpriteSeries>> waterExplosion;

        /** The number of shots in a burst. */
        int burst;

        /** The amount of time between shots in the same burst, in seconds. */
        SimScalar burstInterval;

        /** Maximum angle deviation of projectiles shot in a burst. */
        SimAngle sprayAngle;

        /** Counts how many shots the weapon has fired so far in the current burst. */
        int burstNumber{0};

        /** The game time at which the weapon next becomes ready to fire. */
        GameTime readyTime{0};

        SimAngle tolerance;

        SimAngle pitchTolerance;

        ProjectileRenderType renderType;

        /** Projectile velocity in pixels/tick. */
        SimScalar velocity;

        /** If true, the weapon only fires on command and does not auto-target. */
        bool commandFire;

        std::unordered_map<std::string, unsigned int> damage;

        SimScalar damageRadius;

        /** Number of ticks projectiles fired from this weapon live for */
        std::optional<GameTime> weaponTimer;

        /** Offset from aim point to firing point, compensation for ballistics calculations. */
        SimScalar ballisticZOffset{0};

        /** The internal state of the weapon. */
        UnitWeaponState state{UnitWeaponStateIdle()};
    };
}
