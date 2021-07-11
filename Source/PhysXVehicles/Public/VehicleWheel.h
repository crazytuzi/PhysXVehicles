// Copyright Epic Games, Inc. All Rights Reserved.

/*
 * Component to handle the vehicle simulation for an actor
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "UObject/ScriptMacros.h"
#include "EngineDefines.h"
#include "VehicleWheel.generated.h"

class UPhysicalMaterial;
class FPhysXVehicleManager;

#if WITH_PHYSX
namespace physx
{
	class PxShape;
}
#endif // WITH_PHYSX

UENUM()
enum EWheelSweepType
{
	/** Sweeps against both simple and complex geometry. */
	// 扫描简单和复杂的几何图形
	SimpleAndComplex UMETA(DisplayName="SimpleAndComplex"),	
	/** Sweeps against simple geometry only */
	// 仅扫描简单几何体
	Simple	UMETA(DisplayName="Simple"),		
	/** Sweeps against complex geometry only */
	// 仅扫描复杂几何体
	Complex	UMETA(DisplayName="Complex")	
};

class UE_DEPRECATED(4.26, "PhysX is deprecated. Use the UChaosVehicleWheel from the ChaosVehiclePhysics Plugin.") UVehicleWheel;
UCLASS(BlueprintType, Blueprintable)
class PHYSXVEHICLES_API UVehicleWheel : public UObject
{
	GENERATED_UCLASS_BODY()

	/** 
	 * Static mesh with collision setup for wheel, will be used to create wheel shape
	 * (if empty, sphere will be added as wheel shape, check bDontCreateShape flag)
	 */
	// 带有车轮碰撞设置的静态网格物体，将用于创建车轮形状
	// （如果为空，球体将被添加为车轮形状，检查 bDontCreateShape 标志
	UPROPERTY(EditDefaultsOnly, Category=Shape)
	class UStaticMesh*								CollisionMesh;

	/** If set, shape won't be created, but mapped from chassis mesh */
	// 如果设置，则不会创建形状，而是从底盘网格映射
	UPROPERTY(EditDefaultsOnly, Category=Shape, meta=(DisplayName="UsePhysAssetShape"))
	bool											bDontCreateShape;

	/** 
	 *	If true, ShapeRadius and ShapeWidth will be used to automatically scale collision taken from CollisionMesh to match wheel size.
	 *	If false, size of CollisionMesh won't be changed. Use if you want to scale wheels manually.
	 */
	// 如果为 true，ShapeRadius 和 ShapeWidth 将用于自动缩放从 CollisionMesh 获取的碰撞以匹配车轮大小
	// 如果为 false，则 CollisionMesh 的大小不会改变。 如果您想手动缩放车轮，请使用
	UPROPERTY(EditAnywhere, Category=Shape)
	bool											bAutoAdjustCollisionSize;

	/** 
	 * If BoneName is specified, offset the wheel from the bone's location.
	 * Otherwise this offsets the wheel from the vehicle's origin.
	 */
	// 如果指定了 BoneName，则从骨骼的位置偏移车轮
	// 否则这会使车轮偏离车辆的原点
	UPROPERTY(EditAnywhere, Category=Wheel)
	FVector											Offset;

	/** Radius of the wheel */
	// 车轮半径
	UPROPERTY(EditAnywhere, Category = Wheel, meta = (ClampMin = "0.01", UIMin = "0.01"))
	float											ShapeRadius;

	/** Width of the wheel */
	// 车轮宽度
	UPROPERTY(EditAnywhere, Category = Wheel, meta = (ClampMin = "0.01", UIMin = "0.01"))
	float											ShapeWidth;

	/** Mass of this wheel */
	// 车轮质量
	UPROPERTY(EditAnywhere, Category = Wheel, meta = (ClampMin = "0.01", UIMin = "0.01"))
	float											Mass;

	/** Damping rate for this wheel (Kgm^2/s) */
	// 车轮的阻尼率 (Kgm^2/s)
	UPROPERTY(EditAnywhere, Category = Wheel, meta = (ClampMin = "0.01", UIMin = "0.01"))
	float											DampingRate;

	// steer angle in degrees for this wheel
	// 车轮的转向角度（以度为单位）
	UPROPERTY(EditAnywhere, Category = WheelsSetup, meta = (ClampMin = "0", UIMin = "0"))
	float SteerAngle;

	/** Whether handbrake should affect this wheel */
	// 手刹是否应该影响车轮
	UPROPERTY(EditAnywhere, Category=Wheel)
	bool											bAffectedByHandbrake;

	/** Deprecated */
	// 已弃用
	UPROPERTY()
	class UTireType*								TireType;

	/** Tire type for the wheel. Determines friction */
	// 车轮的轮胎类型。 确定摩擦
	UPROPERTY(EditAnywhere, Category = Tire)
	class UTireConfig*								TireConfig;

	/** Max normalized tire load at which the tire can deliver no more lateral stiffness no matter how much extra load is applied to the tire. */
	// 最大归一化轮胎载荷，在该载荷下，无论对轮胎施加多少额外载荷，轮胎都无法提供更多横向刚度
	UPROPERTY(EditAnywhere, Category=Tire, meta=(ClampMin = "0.01", UIMin = "0.01"))
	float											LatStiffMaxLoad;

	/** How much lateral stiffness to have given lateral slip */
	// 产生横向滑移的横向刚度是多少
	UPROPERTY(EditAnywhere, Category=Tire, meta=(ClampMin = "0.01", UIMin = "0.01"))
	float											LatStiffValue;

	/** How much longitudinal stiffness to have given longitudinal slip */
	// 产生纵向滑移的纵向刚度是多少
	UPROPERTY(EditAnywhere, Category=Tire)
	float											LongStiffValue;

	/** Vertical offset from where suspension forces are applied (along Z-axis) */
	// 悬挂力施加位置的垂直偏移（沿 Z 轴）
	UPROPERTY(EditAnywhere, Category=Suspension)
	float											SuspensionForceOffset;

	/** How far the wheel can go above the resting position */
	// 车轮可以在静止位置上方走多远
	UPROPERTY(EditAnywhere, Category = Suspension)
	float											SuspensionMaxRaise;

	/** How far the wheel can drop below the resting position */
	// 车轮可以低于静止位置多远
	UPROPERTY(EditAnywhere, Category=Suspension)
	float											SuspensionMaxDrop;
	
	/** Oscillation frequency of suspension. Standard cars have values between 5 and 10 */
	// 悬架的振荡频率。 标准汽车的值在 5 到 10 之间
	UPROPERTY(EditAnywhere, Category=Suspension)
	float											SuspensionNaturalFrequency;

	/**
	 *	The rate at which energy is dissipated from the spring. Standard cars have values between 0.8 and 1.2.
	 *	values < 1 are more sluggish, values > 1 or more twitchy
	 */
	// 从弹簧中耗散能量的速率。 标准汽车的值介于 0.8 和 1.2 之间
	// 值 < 1 更迟钝，值 > 1 或更多抽搐
	UPROPERTY(EditAnywhere, Category=Suspension)
	float											SuspensionDampingRatio;

	/** Whether wheel suspension considers simple, complex, or both */
	// 车轮悬架是否考虑简单扫描、复杂扫描或两者兼而有之
	UPROPERTY(EditAnywhere, Category = Suspension)
	TEnumAsByte<EWheelSweepType> SweepType;

	/** max brake torque for this wheel (Nm) */
	// 车轮的最大制动扭矩 (Nm)
	UPROPERTY(EditAnywhere, Category=Brakes)
	float											MaxBrakeTorque;

	/** 
	 *	Max handbrake brake torque for this wheel (Nm). A handbrake should have a stronger brake torque
	 *	than the brake. This will be ignored for wheels that are not affected by the handbrake. 
	 */
	// 该车轮的最大手刹制动扭矩 (Nm)
	// 手刹应该比刹车有更强的制动力矩
	// 对于不受手刹影响的车轮，这将被忽略
	UPROPERTY(EditAnywhere, Category=Brakes)
	float											MaxHandBrakeTorque;


	/** The vehicle that owns us */
	// 拥有我们的车辆
	UPROPERTY(transient)
	class UWheeledVehicleMovementComponent*			VehicleSim;

	// Our index in the vehicle's (and setup's) wheels array
	// 我们在车辆（和设置）轮子数组中的索引
	UPROPERTY(transient)
	int32											WheelIndex;

	// Longitudinal slip experienced by the wheel
	// 车轮经历的纵向滑动
	UPROPERTY(transient)
	float											DebugLongSlip;

	// Lateral slip experienced by the wheel
	// 车轮经历的横向滑动
	UPROPERTY(transient)
	float											DebugLatSlip;

	// How much force the tire experiences at rest divided by how much force it is experiencing now
	// 轮胎静止时承受的力除以它现在承受的力
	UPROPERTY(transient)
	float											DebugNormalizedTireLoad;

	// How much force the tire is experiencing now
	// 轮胎现在承受多大的力
	float											DebugTireLoad;

	// Wheel torque
	// 车轮扭矩
	UPROPERTY(transient)
	float											DebugWheelTorque;

	// Longitudinal force the wheel is applying to the chassis
	// 车轮施加在底盘上的纵向力
	UPROPERTY(transient)
	float											DebugLongForce;

	// Lateral force the wheel is applying to the chassis
	// 车轮施加在底盘上的横向力
	UPROPERTY(transient)
	float											DebugLatForce;

	// Worldspace location of this wheel
	// 车轮的世界空间位置
	UPROPERTY(transient)
	FVector											Location;

	// Worldspace location of this wheel last frame
	// 上一帧车轮的世界空间位置
	UPROPERTY(transient)
	FVector											OldLocation;

	// Current velocity of the wheel center (change in location over time)
	// 车轮中心的当前速度（位置随时间的变化）
	UPROPERTY(transient)
	FVector											Velocity;

	UFUNCTION(BlueprintCallable, Category="Game|Components|WheeledVehicleMovement")
	// 获取转向角度
	float GetSteerAngle() const;

	UFUNCTION(BlueprintCallable, Category="Game|Components|WheeledVehicleMovement")
	// 获取旋转角度
	float GetRotationAngle() const;

	UFUNCTION(BlueprintCallable, Category="Game|Components|WheeledVehicleMovement")
	// 获取悬挂偏移
	float GetSuspensionOffset() const;

	UFUNCTION(BlueprintCallable, Category="Game|Components|WheeledVehicleMovement")
	// 是否在空中
	bool IsInAir() const;

#if WITH_PHYSX

	// Our wheelshape
	physx::PxShape*									WheelShape;
#endif // WITH_PHYSX

	/**
	 * Initialize this wheel instance
	 */
	// 初始化车轮实例
	virtual void Init( class UWheeledVehicleMovementComponent* InVehicleSim, int32 InWheelIndex );

	/**
	 * Notify this wheel it will be removed from the scene
	 */
	// 通知这个车轮它将从场景中移除
	virtual void Shutdown();

	/**
	 * Get the wheel setup we were created from
	 */
	// 获取我们创建的车轮设置
	struct FWheelSetup& GetWheelSetup();

	/**
	 * Tick this wheel when the vehicle ticks
	 */
	// 当车辆Tick时Tick这个车轮
	virtual void Tick( float DeltaTime );

#if WITH_EDITOR

	/**
	 * Respond to a property change in editor
	 */
	virtual void PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent ) override;

#endif //WITH_EDITOR

protected:

	/**
	 * Get the wheel's location in physics land
	 */
	// 获取车轮的物理位置
	FVector GetPhysicsLocation();

private:
#if WITH_PHYSX && PHYSICS_INTERFACE_PHYSX
	FPhysXVehicleManager* GetVehicleManager() const;
#endif // WITH_PHYSX

public:

	/** Get contact surface material */
	// 获取接触面材料
	UPhysicalMaterial* GetContactSurfaceMaterial();
};
