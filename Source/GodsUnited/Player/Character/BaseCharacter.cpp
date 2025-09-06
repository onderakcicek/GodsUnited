// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseCharacter.h"
#include "../BasePlayerDefinitions.h"
#include "../Components/WaypointMovementComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

#include "GodsUnited/GameLogic/GameModes/PvPGameMode/PvPGameMode.h"

// Constructor: enable ticking and initialize defaults
ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// Create waypoint movement component
	WaypointMovementComponent = CreateDefaultSubobject<UWaypointMovementComponent>(TEXT("WaypointMovementComponent"));
	
	// Initialize energy values
	Energy = 0.0f;
	BonusEnergy = 0.0f;
}

// Called at game start or actor spawn
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Cache game mode
	GameMode = Cast<APvPGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	
	// Lock Z position
	FVector Loc = GetActorLocation();
	SetActorLocation(FVector(Loc.X, Loc.Y, PlayerGroundOffset));

	// Configure movement component for natural braking
	if (auto* MoveComp = Cast<UCharacterMovementComponent>(GetMovementComponent()))
	{
		MoveComp->BrakingDecelerationWalking = PlayerMovementConstants::DEFAULT_BRAKING_DECELERATION;
		MoveComp->GroundFriction = PlayerMovementConstants::DEFAULT_GROUND_FRICTION;
	}
	
	// Bind to movement completion event
	if (WaypointMovementComponent)
	{
		WaypointMovementComponent->OnMovementCompleted.AddDynamic(this, &ABaseCharacter::OnMovementCompleted);
	}
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// Movement processing is now handled by WaypointMovementComponent
}

//-------------------------------------------
// Waypoint System Delegation
//-------------------------------------------

AWaypoint* ABaseCharacter::GetLastWaypoint()
{
	if (WaypointMovementComponent)
	{
		return WaypointMovementComponent->GetLastWaypoint();
	}
	
	return nullptr;
}

FVector ABaseCharacter::GetLastMoveLocation()
{
	if (WaypointMovementComponent)
	{
		return WaypointMovementComponent->GetLastMoveLocation();
	}
	
	return GetActorLocation();
}

void ABaseCharacter::StartFollowingPath()
{
	if (WaypointMovementComponent)
	{
		WaypointMovementComponent->StartFollowingPath();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BaseCharacter: WaypointMovementComponent is null"));
	}
}

void ABaseCharacter::ResetPath()
{
	if (WaypointMovementComponent)
	{
		WaypointMovementComponent->ResetPath();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BaseCharacter: WaypointMovementComponent is null"));
	}
}

AWaypoint* ABaseCharacter::CreateWaypoint(FVector Location, FString Item)
{
	if (WaypointMovementComponent)
	{
		return WaypointMovementComponent->CreateWaypoint(Location, Item);
	}
	
	UE_LOG(LogTemp, Error, TEXT("BaseCharacter: WaypointMovementComponent is null"));
	return nullptr;
}

void ABaseCharacter::AddWaypointToPath(AWaypoint* Waypoint)
{
	if (WaypointMovementComponent)
	{
		WaypointMovementComponent->AddWaypointToPath(Waypoint);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BaseCharacter: WaypointMovementComponent is null"));
	}
}

//-------------------------------------------
// Input & Interaction
//-------------------------------------------

void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Example binding: PlayerInputComponent->BindAction("LeftClick", IE_Pressed, this, &ABaseCharacter::OnMouseClick);
}

void ABaseCharacter::OnMouseClick(FHitResult HitResult, FString ItemId)
{
	// Validate component exists
	if (!WaypointMovementComponent || !IsValid(WaypointMovementComponent))
	{
		UE_LOG(LogTemp, Error, TEXT("BaseCharacter: WaypointMovementComponent is invalid"));
		return;
	}

	// Validate hit result
	if (!HitResult.bBlockingHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("BaseCharacter: Invalid hit result provided to OnMouseClick"));
		return;
	}

	// Validate hit location
	if (HitResult.Location.ContainsNaN())
	{
		UE_LOG(LogTemp, Error, TEXT("BaseCharacter: Hit location is invalid"));
		return;
	}

	WaypointMovementComponent->HandleMouseClick(HitResult, ItemId);
}

//-------------------------------------------
// Action System
//-------------------------------------------

void ABaseCharacter::TriggerAction_Implementation(const FString& ItemId)
{
	UE_LOG(LogTemp, Warning, TEXT("TriggerAction not overridden. ItemId: %s"), *ItemId);
}

//-------------------------------------------
// Energy System
//-------------------------------------------

float ABaseCharacter::CalculateMovementCost(const FVector& InputLocation)
{
	if (WaypointMovementComponent)
	{
		return WaypointMovementComponent->CalculateMovementCost(InputLocation);
	}
	
	return 0.0f;
}

void ABaseCharacter::SpendEnergyToMove(const float MoveCost)
{
	if (WaypointMovementComponent)
	{
		WaypointMovementComponent->SpendEnergyToMove(MoveCost);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BaseCharacter: WaypointMovementComponent is null"));
	}
}

//-------------------------------------------
// Component Access
//-------------------------------------------

UWaypointMovementComponent* ABaseCharacter::GetWaypointMovementComponent() const
{
	return WaypointMovementComponent;
}

bool ABaseCharacter::IsFollowingPath() const
{
	if (WaypointMovementComponent)
	{
		return WaypointMovementComponent->IsFollowingPath();
	}
	
	return false;
}

int32 ABaseCharacter::GetCurrentWaypointIndex() const
{
	if (WaypointMovementComponent)
	{
		return WaypointMovementComponent->GetCurrentWaypointIndex();
	}
	
	return -1;
}

//-------------------------------------------
// Event Handlers
//-------------------------------------------

void ABaseCharacter::OnMovementCompleted(ABaseCharacter* Character)
{
	// Handle movement completion
	UE_LOG(LogTemp, Display, TEXT("BaseCharacter: Movement completed for %s"), 
		Character ? *Character->GetName() : TEXT("Unknown"));
	
	// Custom logic for when character completes movement
	// This can be overridden in subclasses
	OnMovementCompletedEvent(Character);
}

void ABaseCharacter::OnMovementCompletedEvent_Implementation(ABaseCharacter* Character)
{
	// Default implementation - can be overridden in Blueprint or subclasses
	// Empty by default to allow custom implementations
}