// Copyright Epic Games, Inc. All Rights Reserved.

/*
 * Base VehicleSim for the 4W PhysX vehicle class
 */
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "WheeledVehicleMovementComponent.h"
#include "Curves/CurveFloat.h"
#include "WheeledVehicleMovementComponent4W.generated.h"

UENUM()
namespace EVehicleDifferential4W
{
	enum Type
	{
		LimitedSlip_4W,
		LimitedSlip_FrontDrive,
		LimitedSlip_RearDrive,
		Open_4W,
		Open_FrontDrive,
		Open_RearDrive,
	};
}

USTRUCT()
struct FVehicleDifferential4WData
{
	GENERATED_USTRUCT_BODY()

	/** Type of differential */
	// 差速器类型
	UPROPERTY(EditAnywhere, Category=Setup)
	TEnumAsByte<EVehicleDifferential4W::Type> DifferentialType;
	
	/** Ratio of torque split between front and rear (>0.5 means more to front, <0.5 means more to rear, works only with 4W type) */
	// 前后扭矩分配比（>0.5 表示前部更多，<0.5 表示后部更多，仅适用于 4W 类型）
	UPROPERTY(EditAnywhere, Category = Setup, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	float FrontRearSplit;

	/** Ratio of torque split between front-left and front-right (>0.5 means more to front-left, <0.5 means more to front-right, works only with 4W and LimitedSlip_FrontDrive) */
	// 左前和右前扭矩分配比（>0.5 表示左前更多，<0.5 表示右前更多，仅适用于 4W 和 LimitedSlip_FrontDrive）
	UPROPERTY(EditAnywhere, Category = Setup, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	float FrontLeftRightSplit;

	/** Ratio of torque split between rear-left and rear-right (>0.5 means more to rear-left, <0.5 means more to rear-right, works only with 4W and LimitedSlip_RearDrive) */
	// 左后和右后扭矩分配比（>0.5 表示左后更多，<0.5 表示右后更多，仅适用于 4W 和 LimitedSlip_RearDrive）
	UPROPERTY(EditAnywhere, Category = Setup, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	float RearLeftRightSplit;
	
	/** Maximum allowed ratio of average front wheel rotation speed and rear wheel rotation speeds (range: 1..inf, works only with LimitedSlip_4W) */
	// 平均前轮转速和后轮转速的最大允许比（范围：1..inf，仅适用于 LimitedSlip_4W）
	UPROPERTY(EditAnywhere, Category = Setup, meta = (ClampMin = "1.0", UIMin = "1.0"))
	float CentreBias;

	/** Maximum allowed ratio of front-left and front-right wheel rotation speeds (range: 1..inf, works only with LimitedSlip_4W, LimitedSlip_FrontDrive) */
	// 左前轮和右前轮转速的最大允许比（范围：1..inf，仅适用于 LimitedSlip_4W、LimitedSlip_FrontDrive
	UPROPERTY(EditAnywhere, Category = Setup, meta = (ClampMin = "1.0", UIMin = "1.0"))
	float FrontBias;

	/** Maximum allowed ratio of rear-left and rear-right wheel rotation speeds (range: 1..inf, works only with LimitedSlip_4W, LimitedSlip_FrontDrive) */
	// 左后轮和右后轮转速的最大允许比（范围：1..inf，仅适用于 LimitedSlip_4W、LimitedSlip_FrontDrive）
	UPROPERTY(EditAnywhere, Category = Setup, meta = (ClampMin = "1.0", UIMin = "1.0"))
	float RearBias;
};

USTRUCT()
struct FVehicleEngineData
{
	GENERATED_USTRUCT_BODY()

	/** Torque (Nm) at a given RPM*/
	// 给定 RPM 下的扭矩 (Nm)
	// 扭矩曲线
	UPROPERTY(EditAnywhere, Category = Setup)
	FRuntimeFloatCurve TorqueCurve;

	/** Maximum revolutions per minute of the engine */
	// 发动机每分钟最大转速
	UPROPERTY(EditAnywhere, Category = Setup, meta = (ClampMin = "0.01", UIMin = "0.01"))
	float MaxRPM;

	/** Moment of inertia of the engine around the axis of rotation (Kgm^2). */
	// 发动机绕旋转轴的转动惯量 (Kgm^2)
	UPROPERTY(EditAnywhere, Category = Setup, meta = (ClampMin = "0.01", UIMin = "0.01"))
	float MOI;

	/** Damping rate of engine when full throttle is applied (Kgm^2/s) */
	// 全油门时发动机的阻尼率（Kgm^2/s）
	UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DampingRateFullThrottle;

	/** Damping rate of engine in at zero throttle when the clutch is engaged (Kgm^2/s)*/
	// 离合器接合时发动机在零油门时的阻尼率（Kgm^2/s）
	UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DampingRateZeroThrottleClutchEngaged;

	/** Damping rate of engine in at zero throttle when the clutch is disengaged (in neutral gear) (Kgm^2/s)*/
	// 离合器脱开时（空档）发动机在零油门时的阻尼率（Kgm^2/s）
	UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DampingRateZeroThrottleClutchDisengaged;

	/** Find the peak torque produced by the TorqueCurve */
	// 找到 TorqueCurve 产生的峰值扭矩
	float FindPeakTorque() const;
};


USTRUCT()
struct FVehicleGearData
{
	GENERATED_USTRUCT_BODY()

	/** Determines the amount of torque multiplication*/
	// 确定扭矩倍增量
	UPROPERTY(EditAnywhere, Category = Setup)
	float Ratio;

	/** Value of engineRevs/maxEngineRevs that is low enough to gear down*/
	// engineRevs/maxEngineRevs 的值足够低以减挡
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"), Category = Setup)
	float DownRatio;

	/** Value of engineRevs/maxEngineRevs that is high enough to gear up*/
	// engineRevs/maxEngineRevs 的值足够高以加挡
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"), Category = Setup)
	float UpRatio;
};

USTRUCT()
struct FVehicleTransmissionData
{
	GENERATED_USTRUCT_BODY()
	/** Whether to use automatic transmission */
	// 是否使用自动变速器
	UPROPERTY(EditAnywhere, Category = VehicleSetup, meta=(DisplayName = "Automatic Transmission"))
	bool bUseGearAutoBox;

	/** Time it takes to switch gears (seconds) */
	// 换档时间（秒）
	UPROPERTY(EditAnywhere, Category = Setup, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float GearSwitchTime;
	
	/** Minimum time it takes the automatic transmission to initiate a gear change (seconds)*/
	// 自动变速器启动换档所需的最短时间（秒）
	UPROPERTY(EditAnywhere, Category = Setup, meta = (editcondition = "bUseGearAutoBox", ClampMin = "0.0", UIMin="0.0"))
	float GearAutoBoxLatency;

	/** The final gear ratio multiplies the transmission gear ratios.*/
	// 最终齿轮比乘以传动齿轮比
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Setup)
	float FinalRatio;

	/** Forward gear ratios (up to 30) */
	// 前进齿轮比（高达 30）
	UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay)
	TArray<FVehicleGearData> ForwardGears;

	/** Reverse gear ratio */
	// 倒档比
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Setup)
	float ReverseGearRatio;

	/** Value of engineRevs/maxEngineRevs that is high enough to increment gear*/
	// engineRevs/maxEngineRevs 的值足够高以加挡
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Setup, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	float NeutralGearUpRatio;

	/** Strength of clutch (Kgm^2/s)*/
	// 离合器强度（Kgm^2/s）
	UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float ClutchStrength;
};

PRAGMA_DISABLE_DEPRECATION_WARNINGS

class UE_DEPRECATED(4.26, "PhysX is deprecated. Use the UChaosWheeledVehicleMovementComponent from the ChaosVehiclePhysics Plugin.") UWheeledVehicleMovementComponent4W;
UCLASS(ClassGroup = (Physics), meta = (BlueprintSpawnableComponent), hidecategories = (PlanarMovement, "Components|Movement|Planar", Activation, "Components|Activation"))
class PHYSXVEHICLES_API UWheeledVehicleMovementComponent4W : public UWheeledVehicleMovementComponent
{
	GENERATED_UCLASS_BODY()

	/** Engine */
	// 引擎
	UPROPERTY(EditAnywhere, Category = MechanicalSetup)
	FVehicleEngineData EngineSetup;

	/** Differential */
	// 差速器
	UPROPERTY(EditAnywhere, Category = MechanicalSetup)
	FVehicleDifferential4WData DifferentialSetup;

	/** Accuracy of Ackermann steer calculation (range: 0..1) */
	// 阿克曼转向计算的精度（范围：0..1）
	UPROPERTY(EditAnywhere, Category = SteeringSetup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	float AckermannAccuracy;

	/** Transmission data */
	// 变速器
	UPROPERTY(EditAnywhere, Category = MechanicalSetup)
	FVehicleTransmissionData TransmissionSetup;

	/** Maximum steering versus forward speed (km/h) */
	// 最大转向与前进速度 (km/h)
	// 转向曲线
	UPROPERTY(EditAnywhere, Category = SteeringSetup)
	FRuntimeFloatCurve SteeringCurve;

	virtual void Serialize(FArchive & Ar) override;
	virtual void ComputeConstants() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:


#if WITH_PHYSX && PHYSICS_INTERFACE_PHYSX

	/** Allocate and setup the PhysX vehicle */
	// 分配和设置 PhysX 车辆
	virtual void SetupVehicleDrive(physx::PxVehicleWheelsSimData* PWheelsSimData) override;

	virtual void UpdateSimulation(float DeltaTime) override;

#endif // WITH_PHYSX

	/** update simulation data: engine */
	// 更新模拟数据：引擎
	void UpdateEngineSetup(const FVehicleEngineData& NewEngineSetup);

	/** update simulation data: differential */
	// 更新模拟数据：差速器
	void UpdateDifferentialSetup(const FVehicleDifferential4WData& NewDifferentialSetup);

	/** update simulation data: transmission */
	// 更新模拟数据：变速器
	void UpdateTransmissionSetup(const FVehicleTransmissionData& NewGearSetup);
};

PRAGMA_ENABLE_DEPRECATION_WARNINGS
