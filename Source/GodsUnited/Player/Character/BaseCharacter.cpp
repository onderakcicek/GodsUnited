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
	// Set this character to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;

	// Initialize properties
	CurrentWaypointIndex = 0;
	bIsFollowingPath = false;
	MovementTolerance = 100.0f;

	//RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Get a reference to the game mode
	GameMode = Cast<APvPGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	// Set a fixed Z position
	FVector CurrentLocation = GetActorLocation();
	SetActorLocation(FVector(CurrentLocation.X, CurrentLocation.Y, PlayerGroundOffset));
}

// Called every frame
// TODO: early exit and log
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	const bool bIsFollowing = IsFollowingPath();
	auto MoveComp = GetMovementComponent();

	// If we're in action phase and following a path, handle movement
	if (bIsFollowing && MoveComp && GameMode && GameMode->GetCurrentPhase() == EPvPGamePhase::Action)
	{
		MoveToCurrentWaypoint();

		// Check if we've reached the current waypoint
		if (HasReachedCurrentWaypoint())
		{
			// Check if this waypoint was created with a right click
			if (CurrentWaypointIndex < Path.Num() && IsValid(Path[CurrentWaypointIndex]->GetItem().GetObject()))
			{
				// Trigger the right click action
				TriggerItemAction();
			}

			// Move to the next waypoint
			MoveToNextWaypoint();
		}
	}

	// Debug visualization: Draw lines between waypoints
	if (Path.Num() > 1)
	{
		for (int32 i = 0; i < Path.Num() - 1; i++)
		{
			if (Path[i] && Path[i + 1])
			{
				// Draw a line between waypoints
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

		// Highlight current waypoint if following path
		if (bIsFollowingPath && CurrentWaypointIndex < Path.Num() && Path[CurrentWaypointIndex])
		{
			// Draw sphere around current target waypoint
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

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Input bindings would go here
	// For example: PlayerInputComponent->BindAction("LeftClick", IE_Pressed, this, &ABaseCharacter::OnLeftClick);
	// Note: Mouse click events are typically handled in the player controller
}

void ABaseCharacter::OnMouseClick(FHitResult HitResult, bool bIsRightClick)
{
	// Only create waypoints in preparation phase
	if (GameMode && GameMode->GetCurrentPhase() == EPvPGamePhase::Preparation)
	{
		// Create a waypoint at the hit location
		FVector HitLocation = HitResult.Location;

		if (AWaypoint* NewWaypoint = CreateWaypoint(HitLocation, nullptr))
		{
			AddWaypointToPath(NewWaypoint);
		}
	}
}

AWaypoint* ABaseCharacter::CreateWaypoint(FVector Location, TScriptInterface<IItemInterface> Item)
{
	// Adjust waypoint height to be at ground level or fixed height
	// For top-down view, we'll place waypoints at a fixed height (slightly above zero)
	//Location.Z = 10.0f;

	// Spawn a waypoint actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	AWaypoint* Waypoint = GetWorld()->SpawnActor<AWaypoint>(Location, FRotator::ZeroRotator, SpawnParams);

	if (Waypoint)
	{
		// Setup waypoint properties
		Waypoint->OwningPlayer = this;
		if (Item)
		{
			Waypoint->SetItem(Item);
		}

		// Set index value here
		Waypoint->PathIndex = Path.Num(); // Index will be the current size of the path array

		// Log for debugging
		UE_LOG(LogTemp, Warning, TEXT("Created waypoint %d at location %s. IsRightClick: %s"),
		       Waypoint->PathIndex,
		       *Location.ToString(),
		       Waypoint->HasItem() ? TEXT("True") : TEXT("False"));
	}

	return Waypoint;
}

void ABaseCharacter::AddWaypointToPath(AWaypoint* Waypoint)
{
	if (Waypoint)
	{
		// Set the correct path index for this waypoint
		Waypoint->PathIndex = Path.Num();

		// Add to path array
		Path.Add(Waypoint);

		// Log for debugging
		UE_LOG(LogTemp, Display, TEXT("Added waypoint %d to path. IsRightClick: %s"),
		       Waypoint->PathIndex,
		       Waypoint->HasItem() ? TEXT("True") : TEXT("False"));
	}
}

void ABaseCharacter::StartFollowingPath()
{
	if (Path.Num() > 0)
	{
		CurrentWaypointIndex = 0;
		bIsFollowingPath = true;

		UE_LOG(LogTemp, Display, TEXT("Character started following path with %d waypoints"), Path.Num());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Attempted to follow path, but no waypoints exist"));
	}
}

void ABaseCharacter::ResetPath()
{
	// Clear the path array
	Path.Empty();
	CurrentWaypointIndex = 0;
	bIsFollowingPath = false;

	UE_LOG(LogTemp, Display, TEXT("Character path has been reset"));
}

void ABaseCharacter::TriggerItemAction()
{
	// Log that the action was triggered
	UE_LOG(LogTemp, Display, TEXT("Right-click action triggered at waypoint %d"), CurrentWaypointIndex);

	// Call the blueprint event
	TriggerAction();
}

void ABaseCharacter::MoveToCurrentWaypoint()
{
	if (Path.Num() > 0 && CurrentWaypointIndex < Path.Num())
	{
		const AWaypoint* CurrentWaypoint = Path[CurrentWaypointIndex];

		if (CurrentWaypoint)
		{
			// Current position and target position
			FVector CurrentLocation = GetActorLocation();
			FVector TargetLocation = CurrentWaypoint->GetActorLocation();

			// Adjust target to only move in the horizontal plane
			FVector AdjustedTarget = FVector(TargetLocation.X, TargetLocation.Y, CurrentLocation.Z);

			//UAIBlueprintHelperLibrary::SimpleMoveToLocation(GetController(), AdjustedTarget);
			FVector ToTarget = TargetLocation - GetActorLocation();
			if (ToTarget.Size() > 10.f)
			{
				FVector Direction = ToTarget.GetSafeNormal();
				AddMovementInput(Direction, 1.0f);
			}

			// Calculate rotation direction and rotate the character
			if (!AdjustedTarget.Equals(CurrentLocation, 0.1f))
			{
				FVector Direction = (AdjustedTarget - CurrentLocation).GetSafeNormal();

				// Debug: Draw arrow showing the rotation direction
				FVector StartLocation = GetActorLocation() + FVector(0, 0, 50);
				FVector EndLocation = StartLocation + Direction * 200.0f;

				DrawDebugDirectionalArrow(
					GetWorld(),
					StartLocation,
					EndLocation,
					100.0f, // Arrow size
					FColor::Orange, // Arrow color
					false, // Persist lines (false = sadece bu frame göster)
					-1.0f, // Life time (sadece bu frame gözüksün)
					0, // Depth priority
					10.0f // Line thickness
				);
			}

			if (CurrentWaypointIndex > 0 && CurrentWaypointIndex < Path.Num())
			{
				AWaypoint* Previous = Path[CurrentWaypointIndex - 1];
				DrawDebugDirectionalArrow(
					GetWorld(),
					Previous->GetActorLocation(),
					TargetLocation,
					20.0f, // Arrow size
					FColor::Blue,
					false,
					-1.0f,
					0,
					8.0f // Thickness
				);
			}
		}
	}
}

bool ABaseCharacter::HasReachedCurrentWaypoint() const
{
	if (Path.Num() > 0 && CurrentWaypointIndex < Path.Num())
	{
		AWaypoint* CurrentWaypoint = Path[CurrentWaypointIndex];

		if (CurrentWaypoint)
		{
			// Calculate horizontal distance to waypoint (ignore Z axis for distance calculation)
			FVector CharacterLocation = GetActorLocation();
			FVector WaypointLocation = CurrentWaypoint->GetActorLocation();

			// Ignore Z axis for distance calculation
			CharacterLocation.Z = 0;
			WaypointLocation.Z = 0;

			// Calculate only XY distance
			float Distance = FVector::Distance(CharacterLocation, WaypointLocation);
			UE_LOG(LogTemp, Display, TEXT("Current Distance: %f"), Distance);
			// Return true if within tolerance (only consider XY distance)
			return Distance <= MovementTolerance;
		}
	}

	return false;
}

void ABaseCharacter::MoveToNextWaypoint()
{
	CurrentWaypointIndex++;

	// Check if we've reached the end of the path
	if (CurrentWaypointIndex >= Path.Num())
	{
		// We've completed the path
		bIsFollowingPath = false;
		UE_LOG(LogTemp, Display, TEXT("Character has completed the path"));

		// Optional: Switch back to preparation phase
		if (GameMode)
		{
			// Uncomment to automatically switch back to preparation phase
			GameMode->StartPreparationPhase();
		}
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Moving to waypoint %d"), CurrentWaypointIndex);
	}
}
