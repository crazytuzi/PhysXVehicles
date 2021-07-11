// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Templates/SubclassOf.h"
#include "AI/Navigation/NavigationAvoidanceTypes.h"
#include "AI/RVOAvoidanceInterface.h"
#include "GameFramework/PawnMovementComponent.h"
#include "VehicleWheel.h"
#include "WheeledVehicleMovementComponent.generated.h"

class UCanvas;

#if WITH_PHYSX
namespace physx
{
	class PxVehicleDrive;
	class PxVehicleWheels;
	class PxVehicleWheelsSimData;
	class PxRigidBody;
}
#endif // WITH_PHYSX

struct FBodyInstance;

/**
 * Values passed from PhysX to generate tire forces
 */
struct FTireShaderInput
{
	// 摩擦
	// Friction value of the tire contact.
	float TireFriction;

	// 纵向滑移
	// Longitudinal slip of the tire
	float LongSlip;

	// 横向滑移
	// Lateral slip of the tire.
	float LatSlip;

	// 旋转速度，以弧度为单位
	// Rotational speed of the wheel, in radians.
	float WheelOmega;

	// The distance from the tire surface to the center of the wheel.
	// 轮胎表面到车轮中心的距离
	float WheelRadius;

	// 1 / WheelRadius
	// WheelRadius的倒数
	float RecipWheelRadius;

	// How much force (weight) is pushing on the tire when the vehicle is at rest.
	// 当车辆静止时，有多少力（重量）作用在轮胎上
	float RestTireLoad;

	// How much force (weight) is pushing on the tire right now.
	// 现在有多少力（重量）作用在轮胎上
	float TireLoad;

	// RestTireLoad / TireLoad
	float NormalizedTireLoad;

	// Acceleration due to gravity
	// 重力加速度
	float Gravity;

	// 1 / Gravity
	// 重力加速度的倒数
	float RecipGravity;
};

/**
 * Generated tire forces to pass back to PhysX
 */
struct FTireShaderOutput
{
	// The torque to be applied to the wheel around the wheel axle. Opposes the engine torque on the wheel
	// 施加到围绕轮轴的车轮上的扭矩。抵抗车轮上的引擎扭矩
	float WheelTorque;

	// The magnitude of the longitudinal tire force to be applied to the vehicle's rigid body.
	// 施加在车辆刚体上的纵向轮胎力的大小
	float LongForce;

	// The magnitude of the lateral tire force to be applied to the vehicle's rigid body.
	// 施加到车辆刚体上的横向轮胎力的大小
	float LatForce;

	FTireShaderOutput() {}

	FTireShaderOutput(float f)
		: WheelTorque(f)
		, LongForce(f)
		, LatForce(f)
	{
	}
};

/**
 * Vehicle-specific wheel setup
 */
USTRUCT()
struct PHYSXVEHICLES_API FWheelSetup
{
	GENERATED_USTRUCT_BODY()

PRAGMA_DISABLE_DEPRECATION_WARNINGS
	// The wheel class to use
	// 车轮类
	UPROPERTY(EditAnywhere, Category=WheelSetup)
	TSubclassOf<UVehicleWheel> WheelClass;
PRAGMA_ENABLE_DEPRECATION_WARNINGS

	// Bone name on mesh to create wheel at
	// 在网格上创建轮子的骨骼名称
	UPROPERTY(EditAnywhere, Category=WheelSetup)
	FName BoneName;

	// Additional offset to give the wheels for this axle.
	// 为该轴提供车轮的额外偏移量
	UPROPERTY(EditAnywhere, Category=WheelSetup)
	FVector AdditionalOffset;

	// Disables steering regardless of the wheel data
	// 无论车轮数据如何，都禁用转向
	UPROPERTY(EditAnywhere, Category = WheelSetup)
	bool bDisableSteering;

	FWheelSetup();
};

USTRUCT()
struct PHYSXVEHICLES_API FReplicatedVehicleState
{
	GENERATED_USTRUCT_BODY()

	// input replication: steering
	// 转向
	UPROPERTY()
	float SteeringInput;

	// input replication: throttle
	// 油门
	UPROPERTY()
	float ThrottleInput;

	// input replication: brake
	// 刹车
	UPROPERTY()
	float BrakeInput;

	// input replication: handbrake
	// 手刹
	UPROPERTY()
	float HandbrakeInput;

	// state replication: current gear
	// 当前档位
	UPROPERTY()
	int32 CurrentGear;
};

USTRUCT()
struct PHYSXVEHICLES_API FVehicleInputRate
{
	GENERATED_USTRUCT_BODY()

	// Rate at which the input value rises
	// 输入值上升的速率
	UPROPERTY(EditAnywhere, Category=VehicleInputRate)
	float RiseRate;

	// Rate at which the input value falls
	// 输入值下降的速率
	UPROPERTY(EditAnywhere, Category=VehicleInputRate)
	float FallRate;

	FVehicleInputRate() : RiseRate(5.0f), FallRate(5.0f) { }

	/** Change an output value using max rise and fall rates */
	// 使用最大上升和下降率更改输出值
	float InterpInputValue( float DeltaTime, float CurrentValue, float NewValue ) const
	{
		const float DeltaValue = NewValue - CurrentValue;

		// We are "rising" when DeltaValue has the same sign as CurrentValue (i.e. delta causes an absolute magnitude gain)
		// OR we were at 0 before, and our delta is no longer 0.
		// 当 DeltaValue 与 CurrentValue 具有相同的符号时，我们正在“上升”（即 delta 导致绝对幅度增益）
		// 或者我们之前是 0，而我们的 delta 不再是 0
		const bool bRising = (( DeltaValue > 0.0f ) == ( CurrentValue > 0.0f )) ||
								(( DeltaValue != 0.f ) && ( CurrentValue == 0.f ));

		const float MaxDeltaValue = DeltaTime * ( bRising ? RiseRate : FallRate );
		const float ClampedDeltaValue = FMath::Clamp( DeltaValue, -MaxDeltaValue, MaxDeltaValue );
		return CurrentValue + ClampedDeltaValue;
	}
};

/**
 * Component to handle the vehicle simulation for an actor.
 */
class UE_DEPRECATED(4.26, "PhysX is deprecated. Use the UChaosWheeledVehicleMovementComponent fron the ChaosVehiclePhysics Plugin.") UWheeledVehicleMovementComponent;
UCLASS(Abstract, hidecategories=(PlanarMovement, "Components|Movement|Planar", Activation, "Components|Activation"))
class PHYSXVEHICLES_API UWheeledVehicleMovementComponent : public UPawnMovementComponent, public IRVOAvoidanceInterface
{
	GENERATED_UCLASS_BODY()

	// Supports the old (before 4.14) way of applying spring forces. We used to offset from the vehicle center of mass instead of the spring location center of mass. You should only use this for existing content that hasn't been re-tuned
	// 支持旧的（4.14 之前）施加弹簧力的方式
	// 我们曾经从车辆质量中心偏移而不是弹簧位置质量中心
	// 您应该只将它用于尚未重新调整的现有内容
	UPROPERTY(EditAnywhere, Category = VehicleSetup)
	uint8 bDeprecatedSpringOffsetMode : 1;

	/** If true, the brake and reverse controls will behave in a more arcade fashion where holding reverse also functions as brake. For a more realistic approach turn this off*/
	// 如果为 true，则刹车和倒车控制将以更街机方式运行，其中保持倒车也起到刹车的作用
	// 对于更现实的方法，请关闭此功能
	UPROPERTY(EditAnywhere, Category = VehicleSetup)
	uint8 bReverseAsBrake : 1;

	/** If set, component will use RVO avoidance */
	// 障碍物规避算法(Reciprocal Velocity obstacles)
	// 如果设置，组件将使用 RVO 规避
	UPROPERTY(Category = "Avoidance", EditAnywhere, BlueprintReadWrite)
	uint8 bUseRVOAvoidance : 1;

protected:
	// True if the player is holding the handbrake
	// 如果玩家握住手刹，则为真
	UPROPERTY(Transient)
	uint8 bRawHandbrakeInput : 1;

	// True if the player is holding gear up
	// 如果玩家正在加挡则为真
	UPROPERTY(Transient)
	uint8 bRawGearUpInput : 1;

	// True if the player is holding gear down
	UPROPERTY(Transient)
	// 如果玩家正在减挡则为真
	uint8 bRawGearDownInput : 1;

	/** Was avoidance updated in this frame? */
	// 是否在此帧中更新了回避
	UPROPERTY(Transient)
	uint32 bWasAvoidanceUpdated : 1;

public:
	/** Mass to set the vehicle chassis to. It's much easier to tweak vehicle settings when
	 * the mass doesn't change due to tweaks with the physics asset. [kg] */
	// 当质量由于物理资产的调整而没有改变时，调整车辆设置要容易得多
	UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (ClampMin = "0.01", UIMin = "0.01"))
	float Mass;

	/** Wheels to create */
	UPROPERTY(EditAnywhere, Category=VehicleSetup)
	TArray<FWheelSetup> WheelSetups;

	/** DragCoefficient of the vehicle chassis. */
	// 车辆底盘阻力系数
	UPROPERTY(EditAnywhere, Category = VehicleSetup)
	float DragCoefficient;

	/** Chassis width used for drag force computation (cm)*/
	// 用于阻力计算的底盘宽度
	UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (ClampMin = "0.01", UIMin = "0.01"))
	float ChassisWidth;

	/** Chassis height used for drag force computation (cm)*/
	// 用于阻力计算的底盘高度
	UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (ClampMin = "0.01", UIMin = "0.01"))
	float ChassisHeight;

	// Drag area in cm^2
	// 阻力面积
	UPROPERTY(transient)
	float DragArea;

	// Estimated mad speed for engine
	// 估计的最大引擎速度
	UPROPERTY(transient)
	float EstimatedMaxEngineSpeed;

	// Max RPM for engine
	// Revolutions per Minute 引擎每分钟的转速
	// 引擎每分钟的最大转速
	UPROPERTY(transient)
	float MaxEngineRPM;

	// Debug drag magnitude last applied
	// 上次应用调试阻力幅度
	UPROPERTY(transient)
	float DebugDragMagnitude;

	// Used for backwards compat to fixup incorrect COM of vehicles
	// 用于向后兼容以修复不正确的车辆Com

	/** When vehicle is created we want to compute some helper data like drag area, etc.... Derived classes should use this to properly compute things like engine RPM */
	// 创建车辆时，我们想要计算一些辅助数据，如阻力面积等......派生类应该使用它来正确计算诸如引擎 RPM 之类的东西
	virtual void ComputeConstants();

	/** Scales the vehicle's inertia in each direction (forward, right, up) */
	// 在每个方向（向前、向右、向上）缩放车辆的惯性
	UPROPERTY(EditAnywhere, Category=VehicleSetup, AdvancedDisplay)
	FVector InertiaTensorScale;

	/** Clamp normalized tire load to this value */
	// 将标准化轮胎负载限制到此值
	UPROPERTY(EditAnywhere, Category=VehicleSetup, AdvancedDisplay)
	float MinNormalizedTireLoad;

	/** Clamp normalized tire load to this value */
	// 将标准化过滤之后的轮胎负载限制到此值
	UPROPERTY(EditAnywhere, Category = VehicleSetup, AdvancedDisplay)
	float MinNormalizedTireLoadFiltered;

	/** Clamp normalized tire load to this value */
	// 将标准化轮胎负载限制到此值
	UPROPERTY(EditAnywhere, Category=VehicleSetup, AdvancedDisplay)
	float MaxNormalizedTireLoad;

	/** Clamp normalized tire load to this value */
	// 将标准化过滤之后的轮胎负载限制到此值
	UPROPERTY(EditAnywhere, Category = VehicleSetup, AdvancedDisplay)
	float MaxNormalizedTireLoadFiltered;

    /** PhysX sub-steps
     More sub-steps provides better stability but with greater computational cost.
     Typically, vehicles require more sub-steps at very low forward speeds.
     The threshold longitudinal speed has a default value of 5 metres per second. */
	// 更多的sub-steps提供更好的稳定性，但计算成本更高
	// 通常，车辆在非常低的前进速度下需要更多的sub-steps
	// 阈值纵向速度的默认值为 5 米每秒
    UPROPERTY(EditAnywhere, Category=VehicleSetup, AdvancedDisplay, meta = (ClampMin = "0.1", UIMin = "1.0", ClampMax = "1000.0", UIMax = "10.0"))
    float ThresholdLongitudinalSpeed;
    
    /** The sub-step count below the threshold longitudinal speed has a default of 3. */
    // 低于阈值纵向速度的sub-step数量默认为 3
    UPROPERTY(EditAnywhere, Category=VehicleSetup, AdvancedDisplay, meta = (ClampMin = "1", UIMin = "1", ClampMax = "10", UIMax = "5"))
    int32 LowForwardSpeedSubStepCount;
    
    /** The sub-step count above the threshold longitudinal speed has a default of 1. */
    // 超过阈值纵向速度的sub-step数量默认为 1
    UPROPERTY(EditAnywhere, Category=VehicleSetup, AdvancedDisplay, meta = (ClampMin = "1", UIMin = "1", ClampMax = "10", UIMax = "5"))
    int32 HighForwardSpeedSubStepCount;
    
	// Our instanced wheels
	UPROPERTY(transient, duplicatetransient, BlueprintReadOnly, Category=Vehicle)
	TArray<class UVehicleWheel*> Wheels;

	// The value of PhysXVehicleManager::VehicleSetupTag when this vehicle created its physics state.
	// Used to recreate the physics if the blueprint changes.
	// 当该车辆创建其物理状态时 PhysXVehicleManager::VehicleSetupTag 的值
	// 用于在蓝图更改时重新创建物理
	uint32 VehicleSetupTag;

	// 校验滑动阀值
	bool CheckSlipThreshold(float AbsLongSlipThreshold, float AbsLatSlipThreshold) const;
	// 获取最大弹簧力
	float GetMaxSpringForce() const;

	/** UObject interface */
	virtual void Serialize(FArchive& Ar) override;
	/** End UObject interface*/


#if WITH_PHYSX

	// The instanced PhysX vehicle
	// PhysX车辆实例
	physx::PxVehicleWheels* PVehicle;
	physx::PxVehicleDrive* PVehicleDrive;

#endif // WITH_PHYSX

	/** Overridden to allow registration with components NOT owned by a Pawn. */
	// 重写以允许注册不属于 Pawn 的组件
	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;
	
	/** Compute the forces generates from a spinning tire */
	// 计算旋转轮胎产生的力
	virtual void GenerateTireForces(class UVehicleWheel* Wheel, const FTireShaderInput& Input, FTireShaderOutput& Output);

#if WITH_PHYSX && PHYSICS_INTERFACE_PHYSX

	/** Return true if we are ready to create a vehicle */
	// 如果我们准备好创建车辆，则返回 true
	virtual bool CanCreateVehicle() const;

	/** Create and setup the physx vehicle */
	// 创建和设置 physx 车辆
	virtual void CreateVehicle();

	/** Tick this vehicle sim right before input is sent to the vehicle system  */
	// 在将输入发送到车辆系统之前Tick此车辆模拟
	virtual void TickVehicle( float DeltaTime );

	/** Updates the vehicle tuning and other state such as user input. */
	// 更新车辆调整和其他状态，例如用户输入
	virtual void PreTick(float DeltaTime);

	/** Updates the forces of drag acting on the vehicle */
	// 更新作用在车辆上的阻力
	virtual void UpdateDrag( float DeltaTime );

	/** Used to create any physics engine information for this component */
	// 用于为该组件创建任何物理引擎信息
	virtual void OnCreatePhysicsState() override;

	/** Used to shut down and pysics engine structure for this component */
	// 用于关闭此组件的物理引擎结构
	virtual void OnDestroyPhysicsState() override;

	// 是否应该创建此组件的物理引擎结构
	virtual bool ShouldCreatePhysicsState() const override;

	// 此组件的物理引擎结构是否有效
	virtual bool HasValidPhysicsState() const override;

	/** Draw debug text for the wheels and suspension */
	// 为车轮和悬架绘制调试文本
	virtual void DrawDebug(UCanvas* Canvas, float& YL, float& YPos);

	/** Draw debug lines for the wheels and suspension */
	// 绘制车轮和悬架的调试线
	virtual void DrawDebugLines();

	/** Skeletal mesh needs some special handling in the vehicle case */
	// 骨架网格在车辆情况下需要一些特殊处理
	virtual void FixupSkeletalMesh();

	/** Allow the player controller of a different pawn to control this vehicle*/
	// 允许不同 pawn 的玩家控制器控制这辆车
	virtual void SetOverrideController(AController* OverrideController);

#if WITH_EDITOR
	/** Respond to a property change in editor */
	// 响应编辑器中的属性更改
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif //WITH_EDITOR

	// 立刻停止移动
	virtual void StopMovementImmediately() override;

#endif // WITH_PHYSX

	/** Set the user input for the vehicle throttle */
	// 设置车辆油门的用户输入
	UFUNCTION(BlueprintCallable, Category="Game|Components|WheeledVehicleMovement")
	void SetThrottleInput(float Throttle);

	/** Set the user input for the vehicle Brake */
	// 设置车辆刹车的用户输入
	UFUNCTION(BlueprintCallable, Category = "Game|Components|WheeledVehicleMovement")
	void SetBrakeInput(float Brake);
	
	/** Set the user input for the vehicle steering */
	// 设置车辆转向的用户输入
	UFUNCTION(BlueprintCallable, Category="Game|Components|WheeledVehicleMovement")
	void SetSteeringInput(float Steering);

	/** Set the user input for handbrake */
	// 设置车辆手刹的用户输入
	UFUNCTION(BlueprintCallable, Category="Game|Components|WheeledVehicleMovement")
	void SetHandbrakeInput(bool bNewHandbrake);

	/** Set the user input for gear up */
	// 设置车辆加挡的用户输入
	UFUNCTION(BlueprintCallable, Category="Game|Components|WheeledVehicleMovement")
	void SetGearUp(bool bNewGearUp);

	/** Set the user input for gear down */
	// 设置车辆减挡的用户输入
	UFUNCTION(BlueprintCallable, Category="Game|Components|WheeledVehicleMovement")
	void SetGearDown(bool bNewGearDown);

	/** Set the user input for gear (-1 reverse, 0 neutral, 1+ forward)*/
	// 设置车辆档位的用户输入
	// -1 减挡，0 空档，1+ 加挡
	UFUNCTION(BlueprintCallable, Category="Game|Components|WheeledVehicleMovement")
	void SetTargetGear(int32 GearNum, bool bImmediate);

	/** Set the flag that will be used to select auto-gears */
	// 设置将用于选择自动档位的标志
	UFUNCTION(BlueprintCallable, Category="Game|Components|WheeledVehicleMovement")
	void SetUseAutoGears(bool bUseAuto);

	/** How fast the vehicle is moving forward */
	// 车辆前进的速度有多快
	UFUNCTION(BlueprintCallable, Category="Game|Components|WheeledVehicleMovement")
	float GetForwardSpeed() const;

	/** Get current engine's rotation speed */
	// 获取当前引擎的转速
	UFUNCTION(BlueprintCallable, Category="Game|Components|WheeledVehicleMovement")
	float GetEngineRotationSpeed() const;

	/** Get current engine's max rotation speed */
	// 获取当前引擎的最大转速
	UFUNCTION(BlueprintCallable, Category="Game|Components|WheeledVehicleMovement")
	float GetEngineMaxRotationSpeed() const;

	/** Get current gear */
	// 获取当前档位
	UFUNCTION(BlueprintCallable, Category="Game|Components|WheeledVehicleMovement")
	int32 GetCurrentGear() const;

	/** Get target gear */
	// 获取目标档位
	UFUNCTION(BlueprintCallable, Category="Game|Components|WheeledVehicleMovement")
	int32 GetTargetGear() const;

	/** Are gears being changed automatically? */
	// 是否自动换档
	UFUNCTION(BlueprintCallable, Category="Game|Components|WheeledVehicleMovement")
	bool GetUseAutoGears() const;

	// RVO Avoidance

	/** Vehicle Radius to use for RVO avoidance (usually half of vehicle width) */
	// 用于 RVO算法 的车辆半径（通常为车辆宽度的一半）
	UPROPERTY(Category = "Avoidance", EditAnywhere, BlueprintReadWrite)
	float RVOAvoidanceRadius;
	
	/** Vehicle Height to use for RVO avoidance (usually vehicle height) */
	// 用于 RVO算法 的车辆高度（通常为车辆高度）
	UPROPERTY(Category = "Avoidance", EditAnywhere, BlueprintReadWrite)
	float RVOAvoidanceHeight;
	
	/** Area Radius to consider for RVO avoidance */
	// 为 RVO算法 考虑的区域半径
	UPROPERTY(Category = "Avoidance", EditAnywhere, BlueprintReadWrite)
	float AvoidanceConsiderationRadius;

	/** Value by which to alter steering per frame based on calculated avoidance */
	// 基于计算出的规避改变每帧转向的值
	UPROPERTY(Category = "Avoidance", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	float RVOSteeringStep;

	/** Value by which to alter throttle per frame based on calculated avoidance */
	// 基于计算出的规避改变每帧油门的值
	UPROPERTY(Category = "Avoidance", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	float RVOThrottleStep;
	
	/** calculate RVO avoidance and apply it to current velocity */
	// 计算 RVO算法 并将其应用于当前速度
	virtual void CalculateAvoidanceVelocity(float DeltaTime);

	/** No default value, for now it's assumed to be valid if GetAvoidanceManager() returns non-NULL. */
	// 没有默认值，现在假设 GetAvoidanceManager() 返回非 NULL 时有效
	UPROPERTY(Category = "Avoidance", VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay)
	int32 AvoidanceUID;

	/** Moving actor's group mask */
	// 分组掩码
	UPROPERTY(Category = "Avoidance", EditAnywhere, BlueprintReadOnly, AdvancedDisplay)
	FNavAvoidanceMask AvoidanceGroup;

	// 设置分组
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|WheeledVehicleMovement", meta = (DeprecatedFunction, DeprecationMessage = "Please use SetAvoidanceGroupMask function instead."))
	void SetAvoidanceGroup(int32 GroupFlags);

	// 设置分组掩码
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|WheeledVehicleMovement")
	void SetAvoidanceGroupMask(const FNavAvoidanceMask& GroupMask);

	/** Will avoid other agents if they are in one of specified groups */
	// 如果他们在指定的分组之一中，将避免其他代理
	UPROPERTY(Category = "Avoidance", EditAnywhere, BlueprintReadOnly, AdvancedDisplay)
	FNavAvoidanceMask GroupsToAvoid;

	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|WheeledVehicleMovement", meta = (DeprecatedFunction, DeprecationMessage = "Please use SetGroupsToAvoidMask function instead."))
	void SetGroupsToAvoid(int32 GroupFlags);

	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|WheeledVehicleMovement")
	void SetGroupsToAvoidMask(const FNavAvoidanceMask& GroupMask);

	/** Will NOT avoid other agents if they are in one of specified groups, higher priority than GroupsToAvoid */
	// 如果其他代理在指定分组之一中，则不会规避其他代理，优先级高于 GroupsToAvoid
	UPROPERTY(Category = "Avoidance", EditAnywhere, BlueprintReadOnly, AdvancedDisplay)
	FNavAvoidanceMask GroupsToIgnore;

	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|WheeledVehicleMovement", meta = (DeprecatedFunction, DeprecationMessage = "Please use SetGroupsToIgnoreMask function instead."))
	void SetGroupsToIgnore(int32 GroupFlags);

	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|WheeledVehicleMovement")
	void SetGroupsToIgnoreMask(const FNavAvoidanceMask& GroupMask);

	/** De facto default value 0.5 (due to that being the default in the avoidance registration function), indicates RVO behavior. */
	// 默认值是 0.5（因为这是RVO注册函数中的默认值），表示 RVO 行为
	UPROPERTY(Category = "Avoidance", EditAnywhere, BlueprintReadOnly)
	float AvoidanceWeight;
	
	/** Temporarily holds launch velocity when pawn is to be launched so it happens at end of movement. */
	// 当 Pawn 被发动时暂时保持发动速度，所以它发生在移动结束时
	UPROPERTY()
	FVector PendingLaunchVelocity;
	
	/** Change avoidance state and register with RVO manager if necessary */
	// 如有必要，更改规避状态并注册到 RVO 管理器
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|WheeledVehicleMovement")
	void SetAvoidanceEnabled(bool bEnable);

protected:

	AController* GetController() const;

	// replicated state of vehicle 
	// 车辆同步状态
	UPROPERTY(Transient, Replicated)
	FReplicatedVehicleState ReplicatedState;

	// accumulator for RB replication errors 
	// 复制错误的累加器（没有用到）
	float AngErrorAccumulator;

	// What the player has the steering set to. Range -1...1
	// 玩家将转向设置为什么。 范围 -1...1
	UPROPERTY(Transient)
	float RawSteeringInput;

	// What the player has the accelerator set to. Range -1...1
	// 玩家将油门设置为什么。 范围 -1...1
	UPROPERTY(Transient)
	float RawThrottleInput;

	// What the player has the brake set to. Range -1...1
	// 玩家将刹车设置为什么。 范围 -1...1
	UPROPERTY(Transient)
	float RawBrakeInput;

	// Steering output to physics system. Range -1...1
	// 转向输出到物理系统。 范围 -1...1
	UPROPERTY(Transient)
	float SteeringInput;

	// Accelerator output to physics system. Range 0...1
	// 油门输出到物理系统。 范围 0...1
	UPROPERTY(Transient)
	float ThrottleInput;

	// Brake output to physics system. Range 0...1
	// 刹车输出到物理系统。 范围 0...1
	UPROPERTY(Transient)
	float BrakeInput;

	// Handbrake output to physics system. Range 0...1
	// 手刹输出到物理系统。 范围 0...1
	UPROPERTY(Transient)
	float HandbrakeInput;

	// How much to press the brake when the player has release throttle
	// 玩家松开油门时踩多少刹车
	UPROPERTY(EditAnywhere, Category=VehicleInput)
	float IdleBrakeInput;

	// Auto-brake when absolute vehicle forward speed is less than this (cm/s)
	// 车辆绝对前进速度小于此值（cm/s）时自动刹车
	UPROPERTY(EditAnywhere, Category=VehicleInput)
	float StopThreshold;

	// Auto-brake when vehicle forward speed is opposite of player input by at least this much (cm/s)
	// 当车辆前进速度与玩家输入至少相差这么多（cm/s）时自动刹车
	UPROPERTY(EditAnywhere, Category = VehicleInput)
	float WrongDirectionThreshold;

	// Rate at which input throttle can rise and fall
	// 油门输入上升和下降的速率
	UPROPERTY(EditAnywhere, Category=VehicleInput, AdvancedDisplay)
	FVehicleInputRate ThrottleInputRate;

	// Rate at which input brake can rise and fall
	// 刹车输入上升和下降的速率
	UPROPERTY(EditAnywhere, Category=VehicleInput, AdvancedDisplay)
	FVehicleInputRate BrakeInputRate;

	// Rate at which input handbrake can rise and fall
	// 手刹输入上升和下降的速率
	UPROPERTY(EditAnywhere, Category=VehicleInput, AdvancedDisplay)
	FVehicleInputRate HandbrakeInputRate;

	// Rate at which input steering can rise and fall
	// 转向输入上升和下降的速率
	UPROPERTY(EditAnywhere, Category=VehicleInput, AdvancedDisplay)
	FVehicleInputRate SteeringInputRate;

	/** Compute steering input */
	// 计算转向输入
	float CalcSteeringInput();

	/** Compute brake input */
	// 计算刹车输入
	float CalcBrakeInput();

	/** Compute handbrake input */
	// 计算手刹输入
	float CalcHandbrakeInput();

	/** Compute throttle input */
	// 计算油门输入
	virtual float CalcThrottleInput();

	/**
	* Clear all interpolated inputs to default values.
	* Raw input won't be cleared, the vehicle may resume input based movement next frame.
	*/
	// 将所有插值输入清除为默认值
	// 原始输入不会被清除，车辆可能会在下一帧恢复基于输入的运动
	virtual void ClearInput();
	
	/**
	* Clear all raw inputs to default values.
	* Interpolated input won't be cleared, the vehicle will begin interpolating to no input.
	*/
	// 将所有原始输入清除为默认值
	// 插值输入不会被清除，车辆将开始插值至无输入
	virtual void ClearRawInput();

	// 将所有输入清除为默认值
	void ClearAllInput()
	{
		ClearRawInput();
		ClearInput();
	}

	/** Updates the COMOffset on the actual body instance */
	// 在实例上更新Component偏移

	/** Read current state for simulation */
	// 读取当前状态以进行模拟
	virtual void UpdateState(float DeltaTime);

	/** Pass current state to server */
	// 将当前状态传递给服务器
	UFUNCTION(reliable, server, WithValidation)
	void ServerUpdateState(float InSteeringInput, float InThrottleInput, float InBrakeInput, float InHandbrakeInput, int32 CurrentGear);

	/** Update RVO Avoidance for simulation */
	// 更新规避以进行模拟
	void UpdateAvoidance(float DeltaTime);
		
	/** called in Tick to update data in RVO avoidance manager */
	// 在 Tick 中调用以更新 RVO 规避管理器中的数据
	void UpdateDefaultAvoidance();
	
	/** lock avoidance velocity */
	// 锁定规避速度
	void SetAvoidanceVelocityLock(class UAvoidanceManager* Avoidance, float Duration);
	
	/** Calculated avoidance velocity used to adjust steering and throttle */
	// 计算用于调整转向和油门的规避速度
	FVector AvoidanceVelocity;
	
	/** forced avoidance velocity, used when AvoidanceLockTimer is > 0 */
	// 强制规避速度，当AvoidanceLockTimer > 0 时使用
	FVector AvoidanceLockVelocity;
	
	/** remaining time of avoidance velocity lock */
	// 规避速度锁定剩余时间
	float AvoidanceLockTimer;

	/** Handle for delegate registered on mesh component */
	// 在网格组件上注册的委托的句柄
	FDelegateHandle MeshOnPhysicsStateChangeHandle;
	
	/** BEGIN IRVOAvoidanceInterface */
	virtual void SetRVOAvoidanceUID(int32 UID) override;
	virtual int32 GetRVOAvoidanceUID() override;
	virtual void SetRVOAvoidanceWeight(float Weight) override;
	virtual float GetRVOAvoidanceWeight() override;
	virtual FVector GetRVOAvoidanceOrigin() override;
	virtual float GetRVOAvoidanceRadius() override;
	virtual float GetRVOAvoidanceHeight() override;
	virtual float GetRVOAvoidanceConsiderationRadius() override;
	virtual FVector GetVelocityForRVOConsideration() override;
	virtual void SetAvoidanceGroupMask(int32 GroupFlags) override;
	virtual int32 GetAvoidanceGroupMask() override;
	virtual void SetGroupsToAvoidMask(int32 GroupFlags) override;
	virtual int32 GetGroupsToAvoidMask() override;
	virtual void SetGroupsToIgnoreMask(int32 GroupFlags) override;
	virtual int32 GetGroupsToIgnoreMask() override;
	/** END IRVOAvoidanceInterface */

#if WITH_PHYSX && PHYSICS_INTERFACE_PHYSX

	// 档位2PhysX档位
	int32 GearToPhysXGear(const int32 Gear) const;

	// PhysX档位2档位
	int32 PhysXGearToGear(const int32 PhysXGear) const;

	/** Pass input values to vehicle simulation */
	// 将输入值传递给车辆模拟
	virtual void UpdateSimulation( float DeltaTime );

	/** Allocate and setup the PhysX vehicle */
	// 分配和设置 PhysX 车辆
	virtual void SetupVehicle();

	/** Create the specific vehicle drive (4w drive vs tank etc...)*/
	// 创建特定的车辆驱动器（四轮载具 vs 坦克等...）
	virtual void SetupVehicleDrive(physx::PxVehicleWheelsSimData* PWheelsSimData);

	/** Do some final setup after the physx vehicle gets created */
	// 在创建 physx 车辆后进行一些最终设置
	virtual void PostSetupVehicle();

	/** Set up the chassis and wheel shapes */
	// 设置底盘和车轮形状
	virtual void SetupVehicleShapes();

	/** Adjust the PhysX actor's mass */
	// 调整 PhysX actor 的质量
	virtual void SetupVehicleMass();

	// 设置车轮质量属性（假设已经锁定）
	virtual void SetupWheelMassProperties_AssumesLocked(const uint32 NumWheels, physx::PxVehicleWheelsSimData* PWheelsSimData, physx::PxRigidBody* PVehicleActor);

	/** Set up the wheel data */
	// 设置车轮数据
	virtual void SetupWheels(physx::PxVehicleWheelsSimData* PWheelsSimData);

	/** Instantiate and setup our wheel objects */
	// 实例化和设置车轮对象
	virtual void CreateWheels();

	/** Release our wheel objects */
	// 释放轮子对象
	virtual void DestroyWheels();

	/** Get the local position of the wheel at rest */
	// 获取车轮静止时的局部坐标
	virtual FVector GetWheelRestingPosition(const FWheelSetup& WheelSetup);

	/** Get the local COM */
	// center of mass
	// 重心
	virtual FVector GetLocalCOM() const;

	/** Get the mesh this vehicle is tied to */
	// 获取车辆绑定的网格
	class USkinnedMeshComponent* GetMesh();

	void UpdateMassProperties(FBodyInstance* BI);

	void ShowDebugInfo(class AHUD* HUD, class UCanvas* Canvas, const class FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos);
#endif // WITH_PHYSX && PHYSICS_INTERFACE_PHYSX

private:
	UPROPERTY(transient, Replicated)
	AController* OverrideController;

};

//some helper functions for converting units

//rev per minute to rad/s
inline float RPMToOmega(float RPM)
{
	return RPM * PI / 30.f;
}

//rad/s to rev per minute
inline float OmegaToRPM(float Omega)
{
	return Omega * 30.f / PI;
}

//km/h to cm/s
inline float KmHToCmS(float KmH)
{
	return KmH * 100000.f / 3600.f;
}

inline float CmSToKmH(float CmS)
{
	return CmS * 3600.f / 100000.f;
}

inline float M2ToCm2(float M2)
{
	return M2 * 100.f * 100.f;
}

inline float Cm2ToM2(float Cm2)
{
	return Cm2 / (100.f * 100.f);
}
