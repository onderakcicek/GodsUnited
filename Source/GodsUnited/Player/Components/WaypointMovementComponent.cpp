// Fill out your copyright notice in the Description page of Project Settings.

#include "WaypointMovementComponent.h"
#include "../BasePlayerDefinitions.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"

#include "GodsUnited/GameLogic/Managers/ActionManager/Waypoint.h"
#include "GodsUnited/Player/Character/BaseCharacter.h"
#include "GodsUnited/Player/Controller/BasePlayerController.h"

UWaypointMovementComponent::UWaypointMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	// Initialize default values
	CurrentWaypointIndex = 0;
	bIsFollowingPath = false;
	bAcceptingWaypoints = true;
	MovementTolerance = PlayerMovementConstants::DEFAULT_MOVEMENT_TOLERANCE;
	
	// Initialize cached references
	OwnerCharacter = nullptr;
}

void UWaypointMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	
	CacheReferences();
	
	// Create starting waypoint immediately when component begins play
	if (OwnerCharacter)
	{
		InitializeStartingWaypoint();
		UE_LOG(LogTemp, Display, TEXT("WaypointMovementComponent: Starting waypoint created in BeginPlay"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WaypointMovementComponent: OwnerCharacter not available in BeginPlay"));
	}
}

void UWaypointMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	ProcessMovement();
}

//-------------------------------------------
// Path Management
//-------------------------------------------

void UWaypointMovementComponent::HandleMouseClick(FHitResult HitResult, const FString& ItemId)
{
	// Input validation
	if (!bAcceptingWaypoints)
	{
		UE_LOG(LogTemp, Display, TEXT("WaypointMovementComponent: Not accepting waypoints currently"));
		return;
	}

	if (!HitResult.bBlockingHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("WaypointMovementComponent: Invalid hit result"));
		return;
	}

	if (HitResult.Location.ContainsNaN())
	{
		UE_LOG(LogTemp, Error, TEXT("WaypointMovementComponent: Hit location contains NaN values"));
		return;
	}

	FVector HitLocation = HitResult.Location;

	// Determine placement type based on ItemId
	bool bIsCardPlacement = !ItemId.IsEmpty();
	bool bCanPlace;
	int32 EnergyCost = 0;
	FString PlacementType;

	if (bIsCardPlacement)
	{
		// Card placement validation
		bCanPlace = CanPlaceCardWaypoint(HitLocation, EnergyCost);
		PlacementType = TEXT("Card");
		
		if (!bCanPlace)
		{
			UE_LOG(LogTemp, Warning, TEXT("WaypointMovementComponent: Cannot place card waypoint. Required energy: %d, Available energy: %d"), 
				EnergyCost, GetAvailableEnergy());
			
			// Provide visual feedback for insufficient energy
			ProvideInvalidPlacementFeedback(HitLocation, TEXT("Insufficient Energy"));
			return;
		}
	}
	else
	{
		// Movement waypoint validation
		bCanPlace = CanPlaceMovementWaypoint(HitLocation);
		PlacementType = TEXT("Movement");
		EnergyCost = CalculateWaypointEnergyCost(HitLocation);
		
		if (!bCanPlace)
		{
			float Distance = GetDistanceToLastWaypoint(HitLocation);
			UE_LOG(LogTemp, Display, TEXT("WaypointMovementComponent: Cannot place movement waypoint. Distance to last waypoint: %.2f (minimum: %.2f)"), 
				Distance, CardSystemConstants::PLACEMENT_RADIUS);
			
			// Provide visual feedback for too close placement
			ProvideInvalidPlacementFeedback(HitLocation, TEXT("Too Close to Last Waypoint"));
			return;
		}
	}

	// Validation passed - create and place waypoint
	AWaypoint* NewWaypoint = CreateWaypoint(HitLocation, ItemId);
	if (NewWaypoint)
	{
		AddWaypointToPath(NewWaypoint);
		
		// Spend energy for placement
		if (EnergyCost > 0)
		{
			SpendEnergyToMove(EnergyCost);
		}
		
		UE_LOG(LogTemp, Display, TEXT("WaypointMovementComponent: Successfully placed %s waypoint. Energy cost: %d"), 
			*PlacementType, EnergyCost);
		
		// Provide visual feedback for successful placement
		ProvideValidPlacementFeedback(HitLocation, PlacementType, EnergyCost);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("WaypointMovementComponent: Failed to create waypoint"));
	}
}

void UWaypointMovementComponent::ProvideInvalidPlacementFeedback(const FVector& Location, const FString& Reason)
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	// Visual feedback for invalid placement
	DrawDebugSphere(World, Location, CardSystemConstants::DEBUG_SPHERE_SIZE * 2.0f, 
		PlayerDebugConstants::SPHERE_SEGMENTS, FColor::Red, false, 2.0f);
	
	// Draw error message
	DrawDebugString(World, Location + FVector(0, 0, 100), *Reason, 
		nullptr, FColor::Red, 2.0f);
	
	UE_LOG(LogTemp, Warning, TEXT("WaypointMovementComponent: Invalid placement - %s"), *Reason);
}

void UWaypointMovementComponent::ProvideValidPlacementFeedback(const FVector& Location, const FString& PlacementType, int32 EnergyCost)
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	// Visual feedback for successful placement
	FColor FeedbackColor = PlacementType == TEXT("Card") ? FColor::Green : FColor::Blue;
	
	DrawDebugSphere(World, Location, CardSystemConstants::DEBUG_SPHERE_SIZE, 
		PlayerDebugConstants::SPHERE_SEGMENTS, FeedbackColor, false, 1.5f);
	
	// Draw success message with energy cost
	FString Message = FString::Printf(TEXT("%s Placed (Cost: %d)"), *PlacementType, EnergyCost);
	DrawDebugString(World, Location + FVector(0, 0, 80), *Message, 
		nullptr, FeedbackColor, 1.5f);
}

AWaypoint* UWaypointMovementComponent::CreateWaypoint(FVector Location, const FString& Item)
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("WaypointMovementComponent: OwnerCharacter is null"));
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("WaypointMovementComponent: World is null"));
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerCharacter;
	AWaypoint* Waypoint = World->SpawnActor<AWaypoint>(Location, FRotator::ZeroRotator, SpawnParams);
	
	if (Waypoint)
	{
		Waypoint->OwningPlayer = OwnerCharacter;
		if (!Item.IsEmpty())
		{
			Waypoint->SetItem(Item);
		}
		Waypoint->PathIndex = Path.Num();
		
		UE_LOG(LogTemp, Display, TEXT("WaypointMovementComponent: Created waypoint at %s"), *Location.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("WaypointMovementComponent: Failed to spawn waypoint"));
	}
	
	return Waypoint;
}

void UWaypointMovementComponent::AddWaypointToPath(AWaypoint* Waypoint)
{
	if (!Waypoint)
	{
		UE_LOG(LogTemp, Warning, TEXT("WaypointMovementComponent: Attempted to add null waypoint"));
		return;
	}
	
	Waypoint->PathIndex = Path.Num();
	Path.Add(Waypoint);
	
	UE_LOG(LogTemp, Display, TEXT("WaypointMovementComponent: Added waypoint to path. Path length: %d"), Path.Num());
}

void UWaypointMovementComponent::ResetPath()
{
	Path.Empty();
	CurrentWaypointIndex = 0;
	bIsFollowingPath = false;
	
	UE_LOG(LogTemp, Display, TEXT("WaypointMovementComponent: Path reset"));
}

void UWaypointMovementComponent::InitializeStartingWaypoint()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("WaypointMovementComponent: Cannot initialize starting waypoint - OwnerCharacter is null"));
		return;
	}

	// Clear any existing path first
	ResetPath();

	// Get character's current position
	FVector CharacterLocation = OwnerCharacter->GetActorLocation();
	
	// Create starting waypoint at character position
	AWaypoint* StartingWaypoint = CreateWaypoint(CharacterLocation, TEXT(""));
	
	if (StartingWaypoint)
	{
		AddWaypointToPath(StartingWaypoint);
		
		UE_LOG(LogTemp, Display, TEXT("WaypointMovementComponent: Starting waypoint created at %s"), *CharacterLocation.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("WaypointMovementComponent: Failed to create starting waypoint"));
	}
}

AWaypoint* UWaypointMovementComponent::GetLastWaypoint() const
{
	if (Path.IsValidIndex(Path.Num() - 1))
	{
		return Path[Path.Num() - 1];
	}
	
	return nullptr;
}

FVector UWaypointMovementComponent::GetLastMoveLocation() const
{
	AWaypoint* LastWaypoint = GetLastWaypoint();
	
	if (!LastWaypoint)
	{
		return OwnerCharacter ? OwnerCharacter->GetActorLocation() : FVector::ZeroVector;
	}
	
	return LastWaypoint->GetActorLocation();
}

bool UWaypointMovementComponent::CancelLastWaypoint()
{
	// Validate that cancel operation is possible
	if (!CanCancelLastWaypoint())
	{
		UE_LOG(LogTemp, Warning, TEXT("WaypointMovementComponent: Cannot cancel last waypoint"));
		return false;
	}

	// Get the last waypoint for energy refund calculation
	AWaypoint* LastWaypoint = GetLastWaypoint();
	if (!LastWaypoint)
	{
		UE_LOG(LogTemp, Error, TEXT("WaypointMovementComponent: Last waypoint is null during cancel"));
		return false;
	}

	// Calculate energy refund before removing waypoint
	int32 EnergyRefund = 0;
	if (Path.Num() >= 2)
	{
		// Calculate energy cost from second-to-last waypoint to last waypoint
		FVector SecondToLastLocation = Path[Path.Num() - 2]->GetActorLocation();
		FVector LastLocation = LastWaypoint->GetActorLocation();
		EnergyRefund = EnergyCalculations::CalculateEnergyRings(
			SecondToLastLocation, 
			LastLocation, 
			CardSystemConstants::PLACEMENT_RADIUS
		);
	}
	else
	{
		// If only 2 waypoints (starting + one more), calculate from character position
		FVector CharacterLocation = OwnerCharacter ? OwnerCharacter->GetActorLocation() : FVector::ZeroVector;
		FVector LastLocation = LastWaypoint->GetActorLocation();
		EnergyRefund = EnergyCalculations::CalculateEnergyRings(
			CharacterLocation, 
			LastLocation, 
			CardSystemConstants::PLACEMENT_RADIUS
		);
	}

	// Store waypoint info for logging
	FVector WaypointLocation = LastWaypoint->GetActorLocation();
	bool bHadItem = LastWaypoint->HasItem();
	FString ItemId = LastWaypoint->GetItem();

	// Remove waypoint from path
	Path.RemoveAt(Path.Num() - 1);

	// Destroy waypoint actor
	if (IsValid(LastWaypoint))
	{
		LastWaypoint->Destroy();
	}

	// Refund energy if there was a cost
	if (EnergyRefund > 0)
	{
		RefundEnergy(EnergyRefund);
	}

	// Update path indices for remaining waypoints
	UpdateWaypointIndices();

	// Provide visual feedback
	ProvideCancelFeedback(WaypointLocation, EnergyRefund, bHadItem, ItemId);

	UE_LOG(LogTemp, Display, TEXT("WaypointMovementComponent: Cancelled waypoint. Energy refunded: %d, Had item: %s"), 
		EnergyRefund, bHadItem ? TEXT("Yes") : TEXT("No"));

	return true;
}

bool UWaypointMovementComponent::CanCancelLastWaypoint() const
{
	// Must have at least 2 waypoints (starting waypoint + 1 more)
	if (Path.Num() < 2)
	{
		return false;
	}

	// Cannot cancel during movement execution
	if (bIsFollowingPath)
	{
		return false;
	}

	// Must be accepting waypoints (preparation phase)
	if (!bAcceptingWaypoints)
	{
		return false;
	}

	return true;
}

//-------------------------------------------
// Movement Execution
//-------------------------------------------

void UWaypointMovementComponent::StartFollowingPath()
{
	// Validate component state
	if (!IsValidForMovement())
	{
		UE_LOG(LogTemp, Error, TEXT("WaypointMovementComponent: Component not valid for movement"));
		return;
	}

	// Validate path exists and is not empty
	if (Path.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("WaypointMovementComponent: Cannot start following - path is empty"));
		return;
	}

	// Validate all waypoints in path
	bool bValidPath = true;
	for (int32 i = 0; i < Path.Num(); ++i)
	{
		if (!Path[i] || !IsValid(Path[i]))
		{
			UE_LOG(LogTemp, Error, TEXT("WaypointMovementComponent: Invalid waypoint at index %d"), i);
			bValidPath = false;
		}
	}

	if (!bValidPath)
	{
		UE_LOG(LogTemp, Error, TEXT("WaypointMovementComponent: Path contains invalid waypoints"));
		return;
	}

	CurrentWaypointIndex = 0;
	bIsFollowingPath = true;
	
	UE_LOG(LogTemp, Display, TEXT("WaypointMovementComponent: Started following path with %d waypoints"), Path.Num());
}

void UWaypointMovementComponent::StopFollowingPath()
{
	bIsFollowingPath = false;
	
	if (OwnerCharacter)
	{
		if (auto* MoveComp = Cast<UCharacterMovementComponent>(OwnerCharacter->GetMovementComponent()))
		{
			MoveComp->StopMovementImmediately();
		}
	}
	
	UE_LOG(LogTemp, Display, TEXT("WaypointMovementComponent: Stopped following path"));
}

//-------------------------------------------
// Energy System
//-------------------------------------------

bool UWaypointMovementComponent::HasSufficientEnergy(int32 RequiredEnergy) const
{
	return GetAvailableEnergy() >= static_cast<float>(RequiredEnergy);
}

void UWaypointMovementComponent::SpendEnergyToMove(int32 EnergyCost)
{
	if (!OwnerCharacter || EnergyCost <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("WaypointMovementComponent: Invalid energy spending attempt"));
        return;
    }

    // Validate that player has sufficient energy before spending
    int32 AvailableEnergy = GetAvailableEnergy();
    if (AvailableEnergy < EnergyCost)
    {
        UE_LOG(LogTemp, Error, TEXT("WaypointMovementComponent: Attempting to spend more energy than available. Required: %d, Available: %d"), 
            EnergyCost, AvailableEnergy);
        return;
    }

    int32 RemainingCost = EnergyCost;
    
    // Use buffer energy first with multiplier
    if (OwnerCharacter->BufferEnergy > 0 && RemainingCost > 0)
    {
        // Calculate how much buffer energy we need (considering multiplier)
        int32 EffectiveBufferEnergy = OwnerCharacter->BufferEnergy * PlayerMovementConstants::BONUS_ENERGY_MULTIPLIER;
        int32 BufferEnergyToUse = FMath::Min(EffectiveBufferEnergy, RemainingCost);
        
        // Calculate actual buffer units to spend (reverse the multiplier)
        int32 BufferUnitsToSpend = FMath::CeilToInt(static_cast<float>(BufferEnergyToUse) / PlayerMovementConstants::BONUS_ENERGY_MULTIPLIER);
        BufferUnitsToSpend = FMath::Min(BufferUnitsToSpend, OwnerCharacter->BufferEnergy);
        
        OwnerCharacter->BufferEnergy -= BufferUnitsToSpend;
        RemainingCost -= (BufferUnitsToSpend * PlayerMovementConstants::BONUS_ENERGY_MULTIPLIER);
        
        UE_LOG(LogTemp, Display, TEXT("WaypointMovementComponent: Used %d buffer energy units (%d effective energy)"), 
            BufferUnitsToSpend, BufferUnitsToSpend * PlayerMovementConstants::BONUS_ENERGY_MULTIPLIER);
    }
    
    // Spend remaining cost from regular energy
    if (RemainingCost > 0)
    {
        OwnerCharacter->Energy = FMath::Max(0, OwnerCharacter->Energy - RemainingCost);
    }
    
    UE_LOG(LogTemp, Display, TEXT("WaypointMovementComponent: Spent %d energy rings. Remaining: %d regular, %d buffer"), 
        EnergyCost, OwnerCharacter->Energy, OwnerCharacter->BufferEnergy);
}

bool UWaypointMovementComponent::CanPlaceMovementWaypoint(const FVector& TargetLocation) const
{
	// Validate component state
	if (!IsValidForMovement())
	{
		return false;
	}

	// Movement waypoints cannot be placed within placement radius of last waypoint
	// This prevents redundant movement commands in the same area
	float DistanceToLast = GetDistanceToLastWaypoint(TargetLocation);
	
	return DistanceToLast > CardSystemConstants::PLACEMENT_RADIUS;
}

bool UWaypointMovementComponent::CanPlaceCardWaypoint(const FVector& TargetLocation, int32& OutEnergyCost) const
{
	// Validate component state
	if (!IsValidForMovement())
	{
		OutEnergyCost = 0;
		return false;
	}

	// Calculate energy cost for card placement
	OutEnergyCost = CalculateWaypointEnergyCost(TargetLocation);
	
	// Check if player has sufficient energy
	float AvailableEnergy = GetAvailableEnergy();
	
	return AvailableEnergy >= static_cast<float>(OutEnergyCost);
}

int32 UWaypointMovementComponent::GetAvailableEnergy() const
{
	if (!OwnerCharacter)
	{
		return 0;
	}

	// Calculate total available energy as integer
	int32 TotalEnergy = OwnerCharacter->Energy;
	TotalEnergy += (OwnerCharacter->BufferEnergy * PlayerMovementConstants::BONUS_ENERGY_MULTIPLIER);
    
	return TotalEnergy;
}

float UWaypointMovementComponent::GetDistanceToLastWaypoint(const FVector& TargetLocation) const
{
	FVector LastLocation = GetLastMoveLocation();
	
	// Use 2D distance calculation ignoring Z axis
	FVector TargetLocation2D = TargetLocation;
	FVector LastLocation2D = LastLocation;
	TargetLocation2D.Z = 0.0f;
	LastLocation2D.Z = 0.0f;
	
	return FVector::Dist(LastLocation2D, TargetLocation2D);
}

int32 UWaypointMovementComponent::CalculateWaypointEnergyCost(const FVector& TargetLocation) const
{
	FVector LastLocation = GetLastMoveLocation();
	
	// Use standardized ring-based calculation
	return EnergyCalculations::CalculateEnergyRings(
		LastLocation, 
		TargetLocation, 
		CardSystemConstants::PLACEMENT_RADIUS
	);
}

//-------------------------------------------
// Internal Movement Logic
//-------------------------------------------

void UWaypointMovementComponent::ProcessMovement()
{
	if (!IsValidForMovement())
		return;

	if (bIsFollowingPath)
	{
		// Process waypoint sequence with U-turn handling
		ProcessWaypointSequence();
		
		// Move toward current waypoint if still following path
		if (bIsFollowingPath)
		{
			MoveToCurrentWaypoint();
		}
	}

	// Render debug visualization
	RenderDebugVisualization();
}

void UWaypointMovementComponent::ProcessWaypointSequence()
{
	// Validate basic state before processing
	if (!IsValidForMovement() || !bIsFollowingPath)
	{
		return;
	}

	// Handle sequentially reached waypoints
	int32 SafetyCounter = 0;
	const int32 MaxIterations = 10; // Prevent infinite loops
	
	while (Path.IsValidIndex(CurrentWaypointIndex) && HasReachedCurrentWaypoint() && SafetyCounter < MaxIterations)
	{
		// Apply U-turn braking if needed
		HandleUTurnDetection();

		// Handle waypoint reached
		OnWaypointReached();
		
		if (!bIsFollowingPath) 
			return;
			
		++SafetyCounter;
	}

	// Log if we hit the safety limit
	if (SafetyCounter >= MaxIterations)
	{
		UE_LOG(LogTemp, Warning, TEXT("WaypointMovementComponent: Hit safety counter limit in ProcessWaypointSequence"));
	}
}

void UWaypointMovementComponent::HandleUTurnDetection()
{
	auto* MoveComp = Cast<UCharacterMovementComponent>(OwnerCharacter->GetMovementComponent());
	if (!MoveComp)
		return;

	// Get current movement direction
	const FVector OldDir = MoveComp->Velocity.SizeSquared() > KINDA_SMALL_NUMBER
		                       ? MoveComp->Velocity.GetSafeNormal()
		                       : FVector::ZeroVector;
	
	// Check if there's a next waypoint
	const int32 NextIdx = CurrentWaypointIndex + 1;
	if (!Path.IsValidIndex(NextIdx))
		return;

	// Calculate direction to next waypoint
	FVector ToNext = Path[NextIdx]->GetActorLocation() - OwnerCharacter->GetActorLocation();
	ToNext.Z = 0;
	
	if (ToNext.IsNearlyZero())
		return;

	// Calculate angle between current and desired direction
	const FVector NewDir = ToNext.GetSafeNormal();
	const float Dot = FVector::DotProduct(OldDir, NewDir);
	const float Angle = FMath::Acos(FMath::Clamp(Dot, -1.0f, 1.0f));
	
	// Apply instant brake for sharp U-turns
	if (Angle > PlayerMovementConstants::U_TURN_THRESHOLD_RADIANS)
	{
		MoveComp->StopMovementImmediately();
		UE_LOG(LogTemp, Display, TEXT("WaypointMovementComponent: U-turn detected, applying brake. Angle: %.2f degrees"), 
			FMath::RadiansToDegrees(Angle));
	}
}

void UWaypointMovementComponent::MoveToCurrentWaypoint()
{
	if (!IsValidForMovement() || !Path.IsValidIndex(CurrentWaypointIndex))
		return;

	const FVector Curr = OwnerCharacter->GetActorLocation();
	const FVector Target = Path[CurrentWaypointIndex]->GetActorLocation();

	// 1) Compute full 3D vector for checking proximity, but use horizontal for movement
	const FVector ToTarget3D = Target - Curr;
	const float Dist3D = ToTarget3D.Size();
	if (Dist3D <= MovementTolerance)
		return;

	// 2) Compute horizontal (XY) vector for movement and reach check
	FVector ToTarget = ToTarget3D;
	ToTarget.Z = 0;
	const float Dist2D = ToTarget.Size();
	if (Dist2D <= MovementTolerance)
		return;

	// 3) Determine slowdown for sharp turns using 2D directions
	float InputScale = PlayerMovementConstants::MAX_INPUT_SCALE;
	if (auto* MoveComp = Cast<UCharacterMovementComponent>(OwnerCharacter->GetMovementComponent()))
	{
		if (MoveComp->Velocity.SizeSquared() > KINDA_SMALL_NUMBER)
		{
			const FVector OldDir = FVector(MoveComp->Velocity.X, MoveComp->Velocity.Y, 0.0f).GetSafeNormal();
			const FVector DesiredDir = ToTarget.GetSafeNormal();
			const float Dot = FVector::DotProduct(OldDir, DesiredDir);
			const float Angle = FMath::Acos(FMath::Clamp(Dot, -1.0f, 1.0f));
			const float SlowDist = MovementTolerance * PlayerMovementConstants::SLOW_DISTANCE_MULTIPLIER;
			if (Angle > PlayerMovementConstants::SLOW_TURN_THRESHOLD_RADIANS && Dist2D < SlowDist)
			{
				InputScale = FMath::Clamp(Dist2D / SlowDist, PlayerMovementConstants::MIN_INPUT_SCALE, PlayerMovementConstants::MAX_INPUT_SCALE);
			}
		}
	}

	// 4) Move using horizontal direction; vertical movement handled by CharacterMovementComponent
	const FVector Direction = ToTarget.GetSafeNormal();
	OwnerCharacter->AddMovementInput(Direction, InputScale);

	// 5) Debug arrow showing movement direction
	const FVector ArrowStart = Curr + FVector(0, 0, PlayerDebugConstants::ARROW_START_HEIGHT_OFFSET);
	const FVector ArrowEnd = ArrowStart + Direction * PlayerDebugConstants::ARROW_LENGTH;
	DrawDebugDirectionalArrow(
		GetWorld(), 
		ArrowStart, 
		ArrowEnd,
		PlayerDebugConstants::ARROW_SIZE, 
		FColor::Orange, 
		false, 
		PlayerDebugConstants::DEBUG_DURATION_PERSISTENT, 
		0, 
		PlayerDebugConstants::ARROW_THICKNESS);
}

bool UWaypointMovementComponent::HasReachedCurrentWaypoint() const
{
	if (!OwnerCharacter || !Path.IsValidIndex(CurrentWaypointIndex))
		return false;

	FVector CharLoc = OwnerCharacter->GetActorLocation();
	FVector WayLoc = Path[CurrentWaypointIndex]->GetActorLocation();
	CharLoc.Z = 0;
	WayLoc.Z = 0;
	const float Dist2D = FVector::Dist(CharLoc, WayLoc);
	return Dist2D <= MovementTolerance;
}

void UWaypointMovementComponent::MoveToNextWaypoint()
{
	++CurrentWaypointIndex;
	if (CurrentWaypointIndex >= Path.Num())
	{
		bIsFollowingPath = false;
		if (OwnerCharacter)
		{
			if (auto* MoveComp = Cast<UCharacterMovementComponent>(OwnerCharacter->GetMovementComponent()))
			{
				MoveComp->StopMovementImmediately();
			}
		}

		// Broadcast completion event
		OnMovementCompleted.Broadcast(OwnerCharacter);
		
		UE_LOG(LogTemp, Display, TEXT("WaypointMovementComponent: Path following completed"));
	}
}

void UWaypointMovementComponent::OnWaypointReached()
{
	// Trigger item action if needed
	if (Path.IsValidIndex(CurrentWaypointIndex) && Path[CurrentWaypointIndex]->HasItem())
	{
		TriggerItemAction();
	}
	
	// Advance to next waypoint
	MoveToNextWaypoint();
	
	// Redirect velocity to new waypoint path
	if (!bIsFollowingPath || !OwnerCharacter) 
		return;
	
	if (auto* MoveComp = Cast<UCharacterMovementComponent>(OwnerCharacter->GetMovementComponent()))
	{
		float Speed = MoveComp->Velocity.Size();
		if (Speed > KINDA_SMALL_NUMBER && Path.IsValidIndex(CurrentWaypointIndex))
		{
			FVector Dir = (Path[CurrentWaypointIndex]->GetActorLocation() - OwnerCharacter->GetActorLocation());
			Dir.Z = 0;
			Dir.Normalize();
			MoveComp->Velocity = Dir * Speed;
		}
	}
}

void UWaypointMovementComponent::TriggerItemAction()
{
	if (!OwnerCharacter || !Path.IsValidIndex(CurrentWaypointIndex))
		return;

	// Delegate to character's action system
	OwnerCharacter->TriggerAction(Path[CurrentWaypointIndex]->GetItem());
}

void UWaypointMovementComponent::RenderDebugVisualization()
{
	UWorld* World = GetWorld();
	if (!World || Path.Num() <= 1)
		return;

	// Draw waypoint path
	for (int32 i = 0; i < Path.Num() - 1; ++i)
	{
		if (Path[i] && Path[i + 1])
		{
			DrawDebugLine(
				World,
				Path[i]->GetActorLocation(),
				Path[i + 1]->GetActorLocation(),
				FColor::Yellow, 
				false, 
				PlayerDebugConstants::DEBUG_DURATION_PERSISTENT, 
				0, 
				PlayerDebugConstants::LINE_THICKNESS);
		}
	}
	
	// Draw current target waypoint
	if (bIsFollowingPath && Path.IsValidIndex(CurrentWaypointIndex))
	{
		DrawDebugSphere(
			World,
			Path[CurrentWaypointIndex]->GetActorLocation(), 
			PlayerDebugConstants::SPHERE_RADIUS, 
			PlayerDebugConstants::SPHERE_SEGMENTS,
			FColor::Green, 
			false, 
			PlayerDebugConstants::DEBUG_DURATION_PERSISTENT, 
			0, 
			PlayerDebugConstants::LINE_THICKNESS);
	}
}

//-------------------------------------------
// Private Helpers
//-------------------------------------------

void UWaypointMovementComponent::CacheReferences()
{
	OwnerCharacter = Cast<ABaseCharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("WaypointMovementComponent: Owner is not a BaseCharacter"));
		return;
	}
	
	UE_LOG(LogTemp, Display, TEXT("WaypointMovementComponent: Successfully cached OwnerCharacter reference"));
}

bool UWaypointMovementComponent::IsValidForMovement() const
{
	// Check basic component validity
	if (!OwnerCharacter || !IsValid(OwnerCharacter))
	{
		return false;
	}

	// Check world context
	UWorld* World = GetWorld();
	if (!World || !IsValid(World))
	{
		return false;
	}

	// Check character movement component
	UCharacterMovementComponent* MoveComp = Cast<UCharacterMovementComponent>(OwnerCharacter->GetMovementComponent());
	if (!MoveComp || !IsValid(MoveComp))
	{
		return false;
	}

	return true;
}

/**
 * Refund energy to player, prioritizing regular energy first
 * @param EnergyAmount Amount of energy to refund
 */
void UWaypointMovementComponent::RefundEnergy(int32 EnergyAmount)
{
	if (!OwnerCharacter || EnergyAmount <= 0)
		return;

	// Add energy back to regular energy pool
	OwnerCharacter->Energy += EnergyAmount;

	UE_LOG(LogTemp, Display, TEXT("WaypointMovementComponent: Refunded %d energy. Current energy: %d"), 
		EnergyAmount, OwnerCharacter->Energy);
}

/**
 * Update path indices for all waypoints after removal
 */
void UWaypointMovementComponent::UpdateWaypointIndices()
{
	for (int32 i = 0; i < Path.Num(); ++i)
	{
		if (Path[i] && IsValid(Path[i]))
		{
			Path[i]->PathIndex = i;
		}
	}
}

/**
 * Provide visual feedback for waypoint cancellation
 * @param CancelledLocation Location where waypoint was cancelled
 * @param EnergyRefunded Amount of energy that was refunded
 * @param bHadItem Whether cancelled waypoint had an item
 * @param ItemId ID of the cancelled item
 */
void UWaypointMovementComponent::ProvideCancelFeedback(const FVector& CancelledLocation, int32 EnergyRefunded, bool bHadItem, const FString& ItemId)
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	// Visual feedback for cancellation
	FColor FeedbackColor = FColor::Orange;
	
	// Draw cancellation indicator
	DrawDebugSphere(World, CancelledLocation, CardSystemConstants::DEBUG_SPHERE_SIZE * 1.5f, 
		PlayerDebugConstants::SPHERE_SEGMENTS, FeedbackColor, false, 2.0f);
	
	// Draw X mark to indicate cancellation
	FVector XStart1 = CancelledLocation + FVector(-20, -20, 50);
	FVector XEnd1 = CancelledLocation + FVector(20, 20, 50);
	FVector XStart2 = CancelledLocation + FVector(-20, 20, 50);
	FVector XEnd2 = CancelledLocation + FVector(20, -20, 50);
	
	DrawDebugLine(World, XStart1, XEnd1, FeedbackColor, false, 2.0f, 0, 5.0f);
	DrawDebugLine(World, XStart2, XEnd2, FeedbackColor, false, 2.0f, 0, 5.0f);
	
	// Draw feedback message
	FString Message = FString::Printf(TEXT("Cancelled (+%d Energy)"), EnergyRefunded);
	if (bHadItem)
	{
		Message += FString::Printf(TEXT(" [%s]"), *ItemId);
	}
	
	DrawDebugString(World, CancelledLocation + FVector(0, 0, 100), *Message, 
		nullptr, FeedbackColor, 2.0f);
}