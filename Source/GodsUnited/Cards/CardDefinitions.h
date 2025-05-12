// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Array.h"
#include "CoreGlobals.h"

#include "GodsUnited/Items/ItemDefinitions.h"
#include "CardDefinitions.generated.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
USTRUCT(BlueprintType)
struct FDTCard : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Id = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemType ItemType = EItemType::None;

	// Weapon specific
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "ItemType == EItemType::Weapon", EditConditionHides))
	EWeaponType WeaponType = EWeaponType::None;

	// Gadget specific
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "ItemType == EItemType::Gadget", EditConditionHides))
	EGadgetType GadgetType = EGadgetType::None;
};