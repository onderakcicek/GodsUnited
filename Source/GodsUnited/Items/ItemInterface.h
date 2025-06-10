// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ItemInterface.generated.h"

// forward declarations
class UItemSubsystem;

// This class does not need to be modified.
UINTERFACE()
class UItemInterface : public UInterface
{
	GENERATED_BODY()
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class GODSUNITED_API IItemInterface
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void PrimaryUse();
	virtual void PrimaryUse_Implementation() = 0;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SecondaryUse();
	virtual void SecondaryUse_Implementation() = 0;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UItemSubsystem* GetSubsystem();
	virtual UItemSubsystem* GetSubsystem_Implementation() = 0;
};
