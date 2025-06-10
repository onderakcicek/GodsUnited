// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemInterface.h"
#include "Components/ActorComponent.h"

#include "GadgetComponent.generated.h"

// Forward declarations
class UItemSubsystem;

// Class
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GODSUNITED_API UGadgetComponent : public UActorComponent, public IItemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGadgetComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	// IItemInterface implementation
	virtual void PrimaryUse_Implementation() override;
	virtual void SecondaryUse_Implementation() override;
	virtual UItemSubsystem* GetSubsystem_Implementation() override;
};
