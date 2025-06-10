// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponComponent.h"

#include "ItemDefinitions.h"
#include "ItemSubsystem.h"

// Sets default values for this component's properties
UWeaponComponent::UWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();	
}


// Called every frame
void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}



void UWeaponComponent::PrimaryUse_Implementation()
{
}

void UWeaponComponent::SecondaryUse_Implementation()
{
}

UItemSubsystem* UWeaponComponent::GetSubsystem_Implementation()
{
	UWorld* World = GetWorld();
	if(!World)
	{
		ITEM_LOG_ERR(TEXT("World is null"));
		return nullptr;	
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if(!GameInstance)
	{
		ITEM_LOG_ERR(TEXT("GameInstance is null"));
		return nullptr;	
	}

	UItemSubsystem* Subsystem = GameInstance->GetSubsystem<UItemSubsystem>();
	if(!Subsystem)
	{
		ITEM_LOG_ERR(TEXT("Subsystem is null"));
		return nullptr;	
	}
	
	return Subsystem;
}