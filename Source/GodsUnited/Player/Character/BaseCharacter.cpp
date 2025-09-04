// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"

#include "GodsUnited/GameLogic/GameModes/PvPGameMode/Definitions.h"
#include "GodsUnited/GameLogic/GameModes/PvPGameMode/PvPGameMode.h"
#include "GodsUnited/GameLogic/Managers/ActionManager/Waypoint.h"

// Constructor: enable ticking and initialize defaults
ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	CurrentWaypointIndex = 0;
	bIsFollowingPath = false;
	MovementTolerance = 10.0f;
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
		MoveComp->BrakingDecelerationWalking = 2048.0f;
		MoveComp->GroundFriction = 8.0f;
	}
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ProcessMovement();
}

AWaypoint* ABaseCharacter::GetLastWaypoint()
{
	if (Path.IsValidIndex(Path.Num() - 1))
	{
		return Path[Path.Num() - 1];
	}

	return nullptr;
}

FVector ABaseCharacter::GetLastMoveLocation()
{
	AWaypoint* LastWaypoint = GetLastWaypoint();
	
	if (!LastWaypoint)
	{
		return GetActorLocation();	
	}

	return LastWaypoint->GetActorLocation();
}

// Main movement logic separated from Tick
void ABaseCharacter::ProcessMovement()
{
	if (bIsFollowingPath && GameMode && GameMode->GetCurrentPhase() == EPvPGamePhase::Action)
	{
		// Handle sequentially reached waypoints
		while (Path.IsValidIndex(CurrentWaypointIndex) && HasReachedCurrentWaypoint())
		{
			// Detect hard U-turn to apply instant brake
			if (auto* MoveComp = Cast<UCharacterMovementComponent>(GetMovementComponent()))
			{
				const FVector OldDir = MoveComp->Velocity.SizeSquared() > KINDA_SMALL_NUMBER
					                       ? MoveComp->Velocity.GetSafeNormal()
					                       : FVector::ZeroVector;
				const int32 NextIdx = CurrentWaypointIndex + 1;
				if (Path.IsValidIndex(NextIdx))
				{
					FVector ToNext = Path[NextIdx]->GetActorLocation() - GetActorLocation();
					ToNext.Z = 0;
					if (!ToNext.IsNearlyZero())
					{
						const FVector NewDir = ToNext.GetSafeNormal();
						const float Dot = FVector::DotProduct(OldDir, NewDir);
						const float Angle = FMath::Acos(FMath::Clamp(Dot, -1.0f, 1.0f));
						constexpr float UTurnThreshold = PI * 0.8f; // ~144°
						if (Angle > UTurnThreshold)
						{
							MoveComp->StopMovementImmediately();
						}
					}
				}
			}

			// Trigger item action if needed
			if (Path[CurrentWaypointIndex]->HasItem())
			{
				TriggerItemAction();
			}
			
			// Advance to next waypoint
			MoveToNextWaypoint();
			
			if (!bIsFollowingPath) return;
		}
		
		// Move toward the current waypoint
		MoveToCurrentWaypoint();
	}

	// Debug: draw waypoint path and current target
	if (Path.Num() > 1)
	{
		for (int32 i = 0; i < Path.Num() - 1; ++i)
		{
			if (Path[i] && Path[i + 1])
			{
				DrawDebugLine(
					GetWorld(),
					Path[i]->GetActorLocation(),
					Path[i + 1]->GetActorLocation(),
					FColor::Yellow, false, -1.0f, 0, 2.0f);
			}
		}
		if (bIsFollowingPath && Path.IsValidIndex(CurrentWaypointIndex))
		{
			DrawDebugSphere(
				GetWorld(),
				Path[CurrentWaypointIndex]->GetActorLocation(), 60.0f, 12,
				FColor::Green, false, -1.0f, 0, 2.0f);
		}
	}
}

// Move the character toward the current waypoint, slowing into sharp turns on horizontal plane
void ABaseCharacter::MoveToCurrentWaypoint()
{
	if (!Path.IsValidIndex(CurrentWaypointIndex))
		return;

	const FVector Curr = GetActorLocation();
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
	float InputScale = 1.0f;
	if (auto* MoveComp = Cast<UCharacterMovementComponent>(GetMovementComponent()))
	{
		if (MoveComp->Velocity.SizeSquared() > KINDA_SMALL_NUMBER)
		{
			const FVector OldDir = FVector(MoveComp->Velocity.X, MoveComp->Velocity.Y, 0.0f).GetSafeNormal();
			const FVector DesiredDir = ToTarget.GetSafeNormal();
			const float Dot = FVector::DotProduct(OldDir, DesiredDir);
			const float Angle = FMath::Acos(FMath::Clamp(Dot, -1.0f, 1.0f));
			const float SlowAngle = PI * 0.7f; // ~126°
			const float SlowDist = MovementTolerance * 10.0f;
			if (Angle > SlowAngle && Dist2D < SlowDist)
			{
				InputScale = FMath::Clamp(Dist2D / SlowDist, 0.1f, 1.0f);
			}
		}
	}

	// 4) Move using horizontal direction; vertical movement handled by CharacterMovementComponent
	const FVector Direction = ToTarget.GetSafeNormal();
	AddMovementInput(Direction, InputScale);

	// 5) Debug arrow showing movement direction
	const FVector ArrowStart = Curr + FVector(0, 0, 50.0f);
	const FVector ArrowEnd = ArrowStart + Direction * 200.0f;
	DrawDebugDirectionalArrow(
		GetWorld(), ArrowStart, ArrowEnd,
		100.0f, FColor::Orange, false, -1.0f, 0, 10.0f);
}

// Check if the character is within tolerance of the waypoint (horizontal distance)
bool ABaseCharacter::HasReachedCurrentWaypoint() const
{
	if (!Path.IsValidIndex(CurrentWaypointIndex))
		return false;

	FVector CharLoc = GetActorLocation();
	FVector WayLoc = Path[CurrentWaypointIndex]->GetActorLocation();
	CharLoc.Z = 0;
	WayLoc.Z = 0;
	const float Dist2D = FVector::Dist(CharLoc, WayLoc);
	return Dist2D <= MovementTolerance;
}

// Bind gameplay inputs
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Example binding: PlayerInputComponent->BindAction("LeftClick", IE_Pressed, this, &ABaseCharacter::OnMouseClick);
}

// Handle mouse clicks to place waypoints in Preparation phase
void ABaseCharacter::OnMouseClick(FHitResult HitResult, FString ItemId)
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

// Spawn a waypoint actor at the specified location with optional item
AWaypoint* ABaseCharacter::CreateWaypoint(FVector Location, FString Item)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	AWaypoint* Waypoint = GetWorld()->SpawnActor<AWaypoint>(Location, FRotator::ZeroRotator, SpawnParams);
	if (Waypoint)
	{
		Waypoint->OwningPlayer = this;
		if (!Item.IsEmpty())
		{
			Waypoint->SetItem(Item);
		}
		Waypoint->PathIndex = Path.Num();
	}
	return Waypoint;
}

// Add a waypoint to the path array
void ABaseCharacter::AddWaypointToPath(AWaypoint* Waypoint)
{
	if (!Waypoint) return;
	Waypoint->PathIndex = Path.Num();
	Path.Add(Waypoint);
}

// Begin moving along the placed waypoints
void ABaseCharacter::StartFollowingPath()
{
	if (Path.Num() > 0)
	{
		CurrentWaypointIndex = 0;
		bIsFollowingPath = true;
	}
}

// Clear the current path and reset state
void ABaseCharacter::ResetPath()
{
	Path.Empty();
	CurrentWaypointIndex = 0;
	bIsFollowingPath = false;
}

// Trigger the action associated with the current waypoint item
void ABaseCharacter::TriggerItemAction()
{
	TriggerAction(Path[CurrentWaypointIndex]->GetItem());
}

// Default implementation for custom actions (override in subclasses)
void ABaseCharacter::TriggerAction_Implementation(const FString& ItemId)
{
	UE_LOG(LogTemp, Warning, TEXT("TriggerAction not overridden. ItemId: %s"), *ItemId);
}

// Advance to the next waypoint or end path following
void ABaseCharacter::MoveToNextWaypoint()
{
	++CurrentWaypointIndex;
	if (CurrentWaypointIndex >= Path.Num())
	{
		bIsFollowingPath = false;
		if (auto* MoveComp = Cast<UCharacterMovementComponent>(GetMovementComponent()))
		{
			MoveComp->StopMovementImmediately();
		}

		MovementCompletedHandle.Broadcast(this);
	}
}

// Handles logic when a waypoint is reached (call within ProcessMovement)
void ABaseCharacter::OnWaypointReached()
{
	// Trigger any item action
	if (Path.IsValidIndex(CurrentWaypointIndex) && Path[CurrentWaypointIndex]->HasItem())
	{
		TriggerItemAction();
	}
	
	// Advance
	MoveToNextWaypoint();
	
	// Redirect velocity to new waypoint path
	if (!bIsFollowingPath) return;
	
	if (auto* MoveComp = Cast<UCharacterMovementComponent>(GetMovementComponent()))
	{
		float Speed = MoveComp->Velocity.Size();
		if (Speed > KINDA_SMALL_NUMBER)
		{
			FVector Dir = (Path[CurrentWaypointIndex]->GetActorLocation() - GetActorLocation());
			Dir.Z = 0;
			Dir.Normalize();
			MoveComp->Velocity = Dir * Speed;
		}
	}
}

float ABaseCharacter::CalculateMovementCost(const FVector& InputLocation)
{
	if (Energy > 0 || GenerativeEnergy > 0)
	{
		FVector LastLocation = Path.Last()->GetActorLocation();

		return FVector::Dist(InputLocation, LastLocation);
	}

	return 0;
}

void ABaseCharacter::SpendEnergyToMove(const float MoveCost)
{
	if (GenerativeEnergy > 0)
	{
		float EnergyFactor = GenerativeEnergy > MoveCost ? MoveCost : GenerativeEnergy;
		Energy = Energy + (EnergyFactor * 2);
		GenerativeEnergy = GenerativeEnergy - EnergyFactor;
	}

	Energy = Energy - MoveCost;
}
