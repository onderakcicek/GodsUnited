// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ItemSubsystem.generated.h"

// forward declarations
class UItemData;

/** Global inventory */
UCLASS(Abstract, Blueprintable, BlueprintType)
class GODSUNITED_API UItemSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/* Lifecycle */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/* API */
	/*UFUNCTION(BlueprintCallable)
	bool AddItem(UItemData* Item);

	UFUNCTION(BlueprintCallable)
	bool RemoveItem(UItemData* Item);

	UPROPERTY(BlueprintReadOnly)
	TArray<UItemData*> Items;*/

	/** Triggers for UI */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInventoryChanged);
	UPROPERTY(BlueprintAssignable)
	FInventoryChanged OnInventoryChanged;
};
