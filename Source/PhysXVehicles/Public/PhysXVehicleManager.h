// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehicleMovementComponent.h"
#include "PhysicsPublic.h"
#include "PhysXIncludes.h"

class UTireConfig;
class UWheeledVehicleMovementComponent;
class FPhysScene_PhysX;

DECLARE_LOG_CATEGORY_EXTERN(LogVehicles, Log, All);

PRAGMA_DISABLE_DEPRECATION_WARNINGS

#if WITH_PHYSX_VEHICLES

/**
 * Manages vehicles and tire surface data for all scenes
 */

class UE_DEPRECATED(4.26, "PhysX is deprecated. Use the FChaosVehicleManager from the ChaosVehiclePhysics Plugin.") FPhysXVehicleManager;
class PHYSXVEHICLES_API FPhysXVehicleManager
{
public:

	// Updated when vehicles need to recreate their physics state.
	// Used when designer tweaks values while the game is running.
	// 当车辆需要重新创建其物理状态时更新
	// 当设计者在游戏运行时调整值时使用
	static uint32											VehicleSetupTag;

	FPhysXVehicleManager(FPhysScene* PhysScene);
	~FPhysXVehicleManager();

	/**
	 * Refresh the tire friction pairs
	 */
	// 刷新轮胎摩擦对
	static void UpdateTireFrictionTable();

	/**
	 * Register a PhysX vehicle for processing
	 */
	// 注册 PhysX 车辆进行处理
	void AddVehicle( TWeakObjectPtr<UWheeledVehicleMovementComponent> Vehicle );

	/**
	 * Unregister a PhysX vehicle from processing
	 */
	// 从处理中取消注册 PhysX 车辆
	void RemoveVehicle( TWeakObjectPtr<UWheeledVehicleMovementComponent> Vehicle );

	/**
	 * Set the vehicle that we want to record telemetry data for
	 */
	// 设置我们要为其记录遥测数据的车辆
	void SetRecordTelemetry( TWeakObjectPtr<UWheeledVehicleMovementComponent> Vehicle, bool bRecord );

	/**
	 * Get the updated telemetry data
	 */
	// 获取更新的遥测数据
	PxVehicleTelemetryData* GetTelemetryData_AssumesLocked();
	
	/**
	 * Get a vehicle's wheels states, such as isInAir, suspJounce, contactPoints, etc
	 */
	// 获取车辆的车轮状态，例如 isInAir、suspJounce、contactPoints 等
	PxWheelQueryResult* GetWheelsStates_AssumesLocked(TWeakObjectPtr<const UWheeledVehicleMovementComponent> Vehicle);

	/**
	 * Update vehicle data before the scene simulates
	 */
	// 在场景模拟之前更新车辆数据
	void Update(FPhysScene* PhysScene, float DeltaTime);
	
	/**
	 * Update vehicle tuning and other state such as input */
	// 更新车辆调整和其他状态，例如输入
	void PreTick(FPhysScene* PhysScene, float DeltaTime);

	/** Detach this vehicle manager from a FPhysScene (remove delegates, remove from map etc) */
	// 从 FPhysScene 中分离该车辆管理器（移除委托、从地图中移除等）
	void DetachFromPhysScene(FPhysScene* PhysScene);

	PxScene* GetScene() const { return Scene; }



	/** Find a vehicle manager from an FPhysScene */
	// 从 FPhysScene 中查找车辆管理器
	static FPhysXVehicleManager* GetVehicleManagerFromScene(FPhysScene* PhysScene);

	/** Gets a transient default TireConfig object */
	// 获取一个暂时的默认 TireConfig 对象
	static UTireConfig* GetDefaultTireConfig();

private:

	// True if the tire friction table needs to be updated
	// 如果轮胎摩擦表需要更新，则为真
	static bool													bUpdateTireFrictionTable;

	// Friction from combinations of tire and surface types.
	// 来自轮胎和表面类型组合的摩擦
	static PxVehicleDrivableSurfaceToTireFrictionPairs*			SurfaceTirePairs;

	/** Map of physics scenes to corresponding vehicle manager */
	// 物理场景映射到相应的车辆管理器
	static TMap<FPhysScene*, FPhysXVehicleManager*>		        SceneToVehicleManagerMap;


	// The scene we belong to
	// 所属的场景
	PxScene*													Scene;

	// All instanced vehicles
	// 所有实例化车辆
	TArray<TWeakObjectPtr<UWheeledVehicleMovementComponent>>	Vehicles;

	// All instanced PhysX vehicles
	// 所有实例化的 PhysX 车辆
	TArray<PxVehicleWheels*>									PVehicles;

	// Store each vehicle's wheels' states like isInAir, suspJounce, contactPoints, etc
	// 存储每辆车的车轮状态，如 isInAir、suspJounce、contactPoints 等
	TArray<PxVehicleWheelQueryResult>							PVehiclesWheelsStates;

	// Scene query results for each wheel for each vehicle
	// 每辆车每个车轮的场景查询结果
	TArray<PxRaycastQueryResult>								WheelQueryResults;

	// Scene raycast hits for each wheel for each vehicle
	// 每辆车的每个车轮的场景光线投射命中
	TArray<PxRaycastHit>										WheelHitResults;

	// Batch query for the wheel suspension raycasts
	// 车轮悬架光线投射的批量查询
	PxBatchQuery*												WheelRaycastBatchQuery;

	FDelegateHandle OnPhysScenePreTickHandle;
	FDelegateHandle OnPhysSceneStepHandle;


	/**
	 * Refresh the tire friction pairs
	 */
	// 刷新轮胎摩擦对
	void UpdateTireFrictionTableInternal();

	/**
	 * Reallocate the WheelRaycastBatchQuery if our number of wheels has increased
	 */
	// 如果我们的车轮数量增加，则重新分配 WheelRaycastBatchQuery
	void SetUpBatchedSceneQuery();

	/**
	 * Update all vehicles without telemetry
	 */
	// 在没有遥测的情况下更新所有车辆
	void UpdateVehicles( float DeltaTime );

	/**
	 * Get the gravity for our phys scene
	 */
	// 获取我们物理场景的重力
	PxVec3 GetSceneGravity_AssumesLocked();

#if PX_DEBUG_VEHICLE_ON

	PxVehicleTelemetryData*				TelemetryData4W;

	PxVehicleWheels*					TelemetryVehicle;

	/**
	 * Init telemetry data
	 */
	// 初始化遥测数据
	void SetupTelemetryData();

	/**
	 * Update the telemetry vehicle and then all other vehicles
	 */
	// 更新遥测车辆，然后更新所有其他车辆
	void UpdateVehiclesWithTelemetry( float DeltaTime );

#endif //PX_DEBUG_VEHICLE_ON
};

#endif // WITH_PHYSX

PRAGMA_ENABLE_DEPRECATION_WARNINGS