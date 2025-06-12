// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"

#include "GodsUnited/GameLogic/GameModes/PvPGameMode/Definitions.h"
#include "GodsUnited/GameLogic/GameModes/PvPGameMode/PvPGameMode.h"
#include "GodsUnited/GameLogic/Managers/ActionManager/Waypoint.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
	// Enable Tick() to be called every frame
	PrimaryActorTick.bCanEverTick = true;

	// Initialize default values
	CurrentWaypointIndex = 0;
	bIsFollowingPath = false;
	MovementTolerance = 10.0f;

	// RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

// Called when the game starts or when this actor is spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Cache a reference to the game mode
	GameMode = Cast<APvPGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	// Lock the character to a fixed Z (height) position on spawn
	FVector CurrentLocation = GetActorLocation();
	SetActorLocation(FVector(CurrentLocation.X, CurrentLocation.Y, PlayerGroundOffset));

	if (UCharacterMovementComponent* MoveComp = Cast<UCharacterMovementComponent>(GetMovementComponent()))
	{
		MoveComp->BrakingDecelerationWalking = 0.f;
		MoveComp->GroundFriction = 0.f;

		//MoveComp->BrakingDecelerationWalking = 2048.f;  // veya sizin için uygun yüksek değer
		//MoveComp->GroundFriction = 8.f;                 // default genelde 8–12 arasındadır
	}
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// During Action phase while following a path, process movement
	if (bIsFollowingPath && GameMode && GameMode->GetCurrentPhase() == EPvPGamePhase::Action)
	{
		// 1) Skip already-reached waypoints and trigger any item actions
		while (Path.IsValidIndex(CurrentWaypointIndex) && HasReachedCurrentWaypoint())
		{
			OnWaypointReached();
			if (!bIsFollowingPath)
			{
				return; // path ended
			}
		}

		// 2) Move towards the current target waypoint
		MoveToCurrentWaypoint();
	}

	// Debug visualization: draw lines between waypoints
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
					FColor::Yellow,
					false,
					-1.0f,
					0,
					2.0f
				);
			}
		}

		// Highlight current waypoint if following
		if (bIsFollowingPath && Path.IsValidIndex(CurrentWaypointIndex) && Path[CurrentWaypointIndex])
		{
			DrawDebugSphere(
				GetWorld(),
				Path[CurrentWaypointIndex]->GetActorLocation(),
				60.0f,
				12,
				FColor::Green,
				false,
				-1.0f,
				0,
				2.0f
			);
		}
	}
}

// Bind gameplay inputs
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Example binding (mouse clicks are typically handled in the PlayerController)
	// PlayerInputComponent->BindAction("LeftClick", IE_Pressed, this, &ABaseCharacter::OnLeftClick);
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

// Spawn a new waypoint actor at the specified location and optional item
AWaypoint* ABaseCharacter::CreateWaypoint(FVector Location, FString Item)
{
	// Configure spawn parameters
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
		UE_LOG(LogTemp, Warning, TEXT("Created waypoint %d at %s. HasItem: %s"),
		       Waypoint->PathIndex,
		       *Location.ToString(),
		       Waypoint->HasItem() ? TEXT("True") : TEXT("False"));
	}
	return Waypoint;
}

// Add a waypoint to the path array
void ABaseCharacter::AddWaypointToPath(AWaypoint* Waypoint)
{
	if (!Waypoint) return;
	Waypoint->PathIndex = Path.Num();
	Path.Add(Waypoint);
	UE_LOG(LogTemp, Display, TEXT("Added waypoint %d to path. HasItem: %s"),
	       Waypoint->PathIndex,
	       Waypoint->HasItem() ? TEXT("True") : TEXT("False"));
}

// Begin moving along the placed waypoints
void ABaseCharacter::StartFollowingPath()
{
	if (Path.Num() > 0)
	{
		CurrentWaypointIndex = 0;
		bIsFollowingPath = true;
		UE_LOG(LogTemp, Display, TEXT("Started following path with %d waypoints"), Path.Num());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot follow path: no waypoints set"));
	}
}

// Clear the current path and reset state
void ABaseCharacter::ResetPath()
{
	Path.Empty();
	CurrentWaypointIndex = 0;
	bIsFollowingPath = false;
	UE_LOG(LogTemp, Display, TEXT("Path has been reset"));
}

// Trigger the action associated with the current waypoint item
void ABaseCharacter::TriggerItemAction()
{
	UE_LOG(LogTemp, Display, TEXT("Triggering item action at waypoint"));
	TriggerAction(Path[CurrentWaypointIndex]->GetItem());
}

// Default implementation for custom actions (override in subclasses)
void ABaseCharacter::TriggerAction_Implementation(const FString& ItemId)
{
	UE_LOG(LogTemp, Warning, TEXT("Default TriggerAction called with item: %s"), *ItemId);
}

// Move towards the current waypoint
void ABaseCharacter::MoveToCurrentWaypoint()
{
	if (!Path.IsValidIndex(CurrentWaypointIndex))
	{
		return;
	}

	FVector CurrentLocation = GetActorLocation();
	FVector TargetLocation = Path[CurrentWaypointIndex]->GetActorLocation();

	// Only consider horizontal movement
	FVector ToTarget = TargetLocation - CurrentLocation;
	ToTarget.Z = 0;

	float DistanceToTarget = ToTarget.Size();

	// Only move if outside tolerance to avoid jitter
	if (DistanceToTarget > MovementTolerance)
	{
		FVector Direction = ToTarget.GetSafeNormal();
		AddMovementInput(Direction, 1.0f);

		// Optional: draw debug arrow for facing direction
		FVector ArrowStart = CurrentLocation + FVector(0, 0, 50);
		FVector ArrowEnd = ArrowStart + Direction * 200.0f;
		DrawDebugDirectionalArrow(
			GetWorld(),
			ArrowStart,
			ArrowEnd,
			100.0f,
			FColor::Orange,
			false,
			-1.0f,
			0,
			10.0f
		);
	}
}

// Check if the character is within tolerance of the waypoint (XY-plane only)
bool ABaseCharacter::HasReachedCurrentWaypoint() const
{
	if (!Path.IsValidIndex(CurrentWaypointIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid waypoint index: %d"), CurrentWaypointIndex);
		return false;
	}

	FVector CharLoc = GetActorLocation();
	FVector WaypointLoc = Path[CurrentWaypointIndex]->GetActorLocation();

	CharLoc.Z = 0;
	WaypointLoc.Z = 0;

	float Distance = FVector::Distance(CharLoc, WaypointLoc);
	UE_LOG(LogTemp, Display, TEXT("Distance to waypoint: %f, Movement Tolerance: %f"), Distance, MovementTolerance);
	return Distance <= MovementTolerance;
}

// Advance to the next waypoint or end path following
void ABaseCharacter::MoveToNextWaypoint()
{
	CurrentWaypointIndex++;
	if (CurrentWaypointIndex >= Path.Num())
	{
		// Stop following and immediately clear any residual movement
		bIsFollowingPath = false;
		if (auto* MoveComp = Cast<UCharacterMovementComponent>(GetMovementComponent()))
		{
			MoveComp->StopMovementImmediately();
		}

		if (GameMode)
		{
			// Uncomment to automatically switch back to preparation phase
			GameMode->StartPreparationPhase();
		}
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Advancing to waypoint %d"), CurrentWaypointIndex);
	}
}

// Handles logic when a waypoint is reached
void ABaseCharacter::OnWaypointReached()
{
	// Ensure current waypoint index is valid
	if (!Path.IsValidIndex(CurrentWaypointIndex))
	{
		return;
	}

	// Trigger item action if present at this waypoint
	if (Path[CurrentWaypointIndex]->HasItem())
	{
		TriggerItemAction();
	}

	// Advance to the next waypoint
	MoveToNextWaypoint();
	if (!bIsFollowingPath)
	{
		return;
	}

	// Redirect existing velocity toward the new waypoint without changing speed
	if (auto* MoveComp = Cast<UCharacterMovementComponent>(GetMovementComponent()))
	{
		float Speed = MoveComp->Velocity.Size();
		if (Speed <= KINDA_SMALL_NUMBER)
		{
			return;
		}

		// Compute vector to next waypoint (XY-plane)
		FVector ToTarget = Path[CurrentWaypointIndex]->GetActorLocation() - GetActorLocation();
		ToTarget.Z = 0;

		// Avoid redirect if already very close to avoid jitter
		if (ToTarget.SizeSquared() <= FMath::Square(MovementTolerance))
		{
			return;
		}

		FVector Direction = ToTarget.GetSafeNormal();
		MoveComp->Velocity = Direction * Speed;
	}
}
