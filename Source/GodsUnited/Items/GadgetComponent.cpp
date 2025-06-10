// Fill out your copyright notice in the Description page of Project Settings.


#include "GadgetComponent.h"

#include "ItemDefinitions.h"
#include "ItemSubsystem.h"


// Sets default values for this component's properties
UGadgetComponent::UGadgetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UGadgetComponent::BeginPlay()
{
	Super::BeginPlay();	
}

// Called every frame
void UGadgetComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


void UGadgetComponent::PrimaryUse_Implementation()
{
}

void UGadgetComponent::SecondaryUse_Implementation()
{
}

UItemSubsystem* UGadgetComponent::GetSubsystem_Implementation()
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