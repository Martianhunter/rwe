#pragma once

#include <rwe/SimAngle.h>
#include <rwe/SimScalar.h>
#include <rwe/SimVector.h>
#include <rwe/UnitFactory.h>
#include <rwe/UnitId.h>
#include <rwe/UnitOrder.h>
#include <rwe/cob/CobExecutionService.h>
#include <rwe/pathfinding/PathFindingService.h>

namespace rwe
{
    class GameScene;

    class UnitBehaviorService
    {
    private:
        GameScene* const scene;
        UnitFactory* const unitFactory;
        CobExecutionService* const cobExecutionService;

    public:
        UnitBehaviorService(
            GameScene* scene,
            UnitFactory* unitFactory,
            CobExecutionService* cobExecutionService);

        void onCreate(UnitId unitId);

        void update(UnitId unitId);

        // FIXME: shouldn't really be public
        SimVector getSweetSpot(UnitId id);
        std::optional<SimVector> tryGetSweetSpot(UnitId id);

    private:
        static std::pair<SimAngle, SimAngle> computeHeadingAndPitch(SimAngle rotation, const SimVector& from, const SimVector& to, SimScalar speed, SimScalar gravity, SimScalar zOffset, ProjectilePhysicsType projectileType);

        static std::pair<SimAngle, SimAngle> computeLineOfSightHeadingAndPitch(SimAngle rotation, const SimVector& from, const SimVector& to);

        static std::pair<SimAngle, SimAngle> computeBallisticHeadingAndPitch(SimAngle rotation, const SimVector& from, const SimVector& to, SimScalar speed, SimScalar gravity, SimScalar zOffset);

        /** Returns true if the order has been completed. */
        bool handleOrder(UnitId unitId, const UnitOrder& moveOrder);

        /** Returns true if the order has been completed. */
        bool handleMoveOrder(UnitId unitId, const MoveOrder& moveOrder);

        /** Returns true if the order has been completed. */
        bool handleAttackOrder(UnitId unitId, const AttackOrder& attackOrder);

        /** Returns true if the order has been completed. */
        bool handleBuildOrder(UnitId unitId, const BuildOrder& buildOrder);

        /** Returns true if the order has been completed. */
        bool handleBuggerOffOrder(UnitId unitId, const BuggerOffOrder& buggerOffOrder);

        /** Returns true if the order has been completed. */
        bool handleCompleteBuildOrder(UnitId unitId, const CompleteBuildOrder& buildOrder);

        bool handleBuild(UnitId unitId, const std::string& unitType);

        void clearBuild(UnitId unitId);

        bool followPath(Unit& unit, PathFollowingInfo& path);

        void updateWeapon(UnitId id, unsigned int weaponIndex);

        SimVector changeDirectionByRandomAngle(const SimVector& direction, SimAngle maxAngle);

        void tryFireWeapon(UnitId id, unsigned int weaponIndex, SimAngle heading, SimAngle pitch, const SimVector& targetPosition);

        void applyUnitSteering(UnitId id);
        void updateUnitRotation(UnitId id);
        void updateUnitSpeed(UnitId id);

        void updateUnitPosition(UnitId unitId);

        bool tryApplyMovementToPosition(UnitId id, const SimVector& newPosition);

        std::string getAimScriptName(unsigned int weaponIndex) const;
        std::string getAimFromScriptName(unsigned int weaponIndex) const;
        std::string getFireScriptName(unsigned int weaponIndex) const;
        std::string getQueryScriptName(unsigned int weaponIndex) const;

        std::optional<int> runCobQuery(UnitId id, const std::string& name);

        SimVector getAimingPoint(UnitId id, unsigned int weaponIndex);
        SimVector getLocalAimingPoint(UnitId id, unsigned int weaponIndex);

        SimVector getFiringPoint(UnitId id, unsigned int weaponIndex);
        SimVector getLocalFiringPoint(UnitId id, unsigned int weaponIndex);

        SimVector getNanoPoint(UnitId id);

        SimVector getPieceLocalPosition(UnitId id, unsigned int pieceId);
        SimVector getPiecePosition(UnitId id, unsigned int pieceId);

        SimAngle getPieceXZRotation(UnitId id, unsigned int pieceId);

        struct BuildPieceInfo
        {
            SimVector position;
            SimAngle rotation;
        };

        BuildPieceInfo getBuildPieceInfo(UnitId id);

        std::optional<SimVector> getTargetPosition(const UnitWeaponAttackTarget& target);

        MovingStateGoal attackTargetToMovingStateGoal(const AttackTarget& target);
    };
}
