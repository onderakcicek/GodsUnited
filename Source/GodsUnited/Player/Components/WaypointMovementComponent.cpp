// Fill out your copyright notice in the Description page of Project Settings.

#include "WaypointMovementComponent.h"
#include "../BasePlayerDefinitions.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

#include "GodsUnited/GameLogic/GameModes/PvPGameMode/Definitions.h"
#include "GodsUnited/GameLogic/GameModes/PvPGameMode/PvPGameMode.h"
#include "GodsUnited/GameLogic/Managers/ActionManager/Waypoint.h"
#include "GodsUnited/Player/Character/BaseCharacter.h"

UWaypointMovementComponent::UWaypointMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	// Initialize default values
	CurrentWaypointIndex = 0;
	bIsFollowingPath = false;
	MovementTolerance = PlayerMovementConstants::DEFAULT_MOVEMENT_TOLERANCE;
	
	// Initialize cached references
	OwnerCharacter = nullptr;
	GameMode = nullptr;
}

void UWaypointMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	
	CacheReferences();
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
	if (GameMode && GameMode->GetCurrentPhase() == EPvPGamePhase::Preparation)
	{
		FVector HitLocation = HitResult.Location;
		if (AWaypoint* NewWaypoint = CreateWaypoint(HitLocation, ItemId))
		{
			AddWaypointToPath(NewWaypoint);
		}
	}
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

//-------------------------------------------
// Movement Execution
//-------------------------------------------

void UWaypointMovementComponent::StartFollowingPath()
{
	if (Path.Num() > 0)
	{
		CurrentWaypointIndex = 0;
		bIsFollowingPath = true;
		
		UE_LOG(LogTemp, Display, TEXT("WaypointMovementComponent: Started following path with %d waypoints"), Path.Num());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WaypointMovementComponent: Cannot start following - path is empty"));
	}
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

float UWaypointMovementComponent::CalculateMovementCost(const FVector& InputLocation) const
{
	if (!OwnerCharacter)
		return 0.0f;

	// Check if character has energy available
	if (OwnerCharacter->Energy > 0 || OwnerCharacter->BonusEnergy > 0)
	{
		FVector LastLocation = GetLastMoveLocation();
		return FVector::Dist(InputLocation, LastLocation);
	}
	
	return 0.0f;
}

void UWaypointMovementComponent::SpendEnergyToMove(float MoveCost)
{
	if (!OwnerCharacter)
		return;

	// Use bonus energy first with multiplier
	if (OwnerCharacter->BonusEnergy > 0)
	{
		float EnergyFactor = OwnerCharacter->BonusEnergy > MoveCost ? MoveCost : OwnerCharacter->BonusEnergy;
		OwnerCharacter->Energy += (EnergyFactor * PlayerMovementConstants::BONUS_ENERGY_MULTIPLIER);
		OwnerCharacter->BonusEnergy -= EnergyFactor;
	}
	
	// Spend regular energy
	OwnerCharacter->Energy -= MoveCost;
	
	UE_LOG(LogTemp, Display, TEXT("WaypointMovementComponent: Spent %.2f energy. Remaining: %.2f"), 
		MoveCost, OwnerCharacter->Energy);
}

//-------------------------------------------
// Internal Movement Logic
//-------------------------------------------

void UWaypointMovementComponent::ProcessMovement()
{
	if (!IsValidForMovement())
		return;

	if (bIsFollowingPath && GameMode && GameMode->GetCurrentPhase() == EPvPGamePhase::Action)
	{
		// Handle sequentially reached waypoints
		while (Path.IsValidIndex(CurrentWaypointIndex) && HasReachedCurrentWaypoint())
		{
			// Detect hard U-turn to apply instant brake
			if (auto* MoveComp = Cast<UCharacterMovementComponent>(OwnerCharacter->GetMovementComponent()))
			{
				const FVector OldDir = MoveComp->Velocity.SizeSquared() > KINDA_SMALL_NUMBER
					                       ? MoveComp->Velocity.GetSafeNormal()
					                       : FVector::ZeroVector;
				const int32 NextIdx = CurrentWaypointIndex + 1;
				if (Path.IsValidIndex(NextIdx))
				{
					FVector ToNext = Path[NextIdx]->GetActorLocation() - OwnerCharacter->GetActorLocation();
					ToNext.Z = 0;
					if (!ToNext.IsNearlyZero())
					{
						const FVector NewDir = ToNext.GetSafeNormal();
						const float Dot = FVector::DotProduct(OldDir, NewDir);
						const float Angle = FMath::Acos(FMath::Clamp(Dot, -1.0f, 1.0f));
						if (Angle > PlayerMovementConstants::U_TURN_THRESHOLD_RADIANS)
						{
							MoveComp->StopMovementImmediately();
						}
					}
				}
			}

			// Handle waypoint reached
			OnWaypointReached();
			
			if (!bIsFollowingPath) return;
		}
		
		// Move toward the current waypoint
		MoveToCurrentWaypoint();
	}

	// Render debug visualization
	RenderDebugVisualization();
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
	
	GameMode = Cast<APvPGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!GameMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("WaypointMovementComponent: GameMode is not PvPGameMode"));
	}
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