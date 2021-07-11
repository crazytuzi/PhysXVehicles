// Copyright Epic Games, Inc. All Rights Reserved.



#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/DataAsset.h"
#include "TireConfig.generated.h"

class UPhysicalMaterial;

/** Allows overriding of friction of this tire config on a specific material */
// 允许覆盖此轮胎配置在特定材料上的摩擦
USTRUCT()
struct FTireConfigMaterialFriction
{
	GENERATED_USTRUCT_BODY()

	/** Physical material for friction scale */
	// 摩擦缩放的物理材料
	UPROPERTY(EditAnywhere, Category = FTireConfigMaterialFriction)
	UPhysicalMaterial*		PhysicalMaterial;

	/** Friction scale for this type of material */
	// 此类材料的摩擦缩放
	UPROPERTY(EditAnywhere, Category = FTireConfigMaterialFriction)
	float					FrictionScale;

	FTireConfigMaterialFriction()
		: PhysicalMaterial(nullptr)
		, FrictionScale(1.0f)
	{
	}
};

/** Represents a type of tire surface used to specify friction values against physical materials. */
// 代表一种轮胎表面，用于指定与物理材料的摩擦系数
class UE_DEPRECATED(4.26, "PhysX is deprecated. Use the Chaos physics and the ChaosVehiclePhysics Plugin.") UTireConfig;
UCLASS()
class PHYSXVEHICLES_API UTireConfig : public UDataAsset
{
	GENERATED_BODY()

private:

	// Scale the tire friction for this tire type
	// 缩放此轮胎类型的轮胎摩擦力
	UPROPERTY(EditAnywhere, Category = TireConfig)
	float								FrictionScale;

	/** Tire friction scales for specific physical materials. */
	// 特定物理材料的轮胎摩擦缩放
	UPROPERTY(EditAnywhere, Category = TireConfig)
	TArray<FTireConfigMaterialFriction> TireFrictionScales;

private:

	// Tire config ID to pass to PhysX
	// 要传递给 PhysX 的轮胎配置 ID
	uint32								TireConfigID;

public:

	UTireConfig();

	// All loaded tire types - used to assign each tire type a unique TireConfigID
	// 所有加载的轮胎类型 - 用于为每个轮胎类型分配唯一的 TireConfigID
	static TArray<TWeakObjectPtr<UTireConfig>>			AllTireConfigs;

	/**
	* Getter for FrictionScale
	*/
	// 获取摩擦缩放
	float GetFrictionScale() { return FrictionScale; }

	/**
	* Setter for FrictionScale
	*/
	// 设置摩擦缩放
	void SetFrictionScale(float NewFrictionScale);

	/** Set friction scaling for a particular material */
	// 为特定材料设置摩擦缩放
	void SetPerMaterialFrictionScale(UPhysicalMaterial*	PhysicalMaterial, float	NewFrictionScale);

	/**
	* Getter for TireConfigID
	*/
	// 获取TireConfigID
	int32 GetTireConfigID() { return TireConfigID; }

	/**
	* Called after the C++ constructor and after the properties have been initialized, but before the config has been loaded, etc.
	* mainly this is to emulate some behavior of when the constructor was called after the properties were initialized.
	*/
	virtual void PostInitProperties() override;

	/**
	* Called before destroying the object.  This is called immediately upon deciding to destroy the object, to allow the object to begin an
	* asynchronous cleanup process.
	*/
	virtual void BeginDestroy() override;

	/** Get the friction for this tire config on a particular physical material */
	// 获取此轮胎配置在特定物理材料上的摩擦系数
	float GetTireFriction(UPhysicalMaterial* PhysicalMaterial);

#if WITH_EDITOR

	/**
	* Respond to a property change in editor
	*/
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

#endif //WITH_EDITOR

protected:

	/**
	* Add this tire type to the TireConfigs array
	*/
	void NotifyTireFrictionUpdated();


};
