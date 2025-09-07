// GenericDragDropOperation.cpp Implementation

#include "GenericDragDropOperation.h"
#include "../Player/BasePlayerDefinitions.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"
#include "GodsUnited/Player/Character/BaseCharacter.h"
#include "GodsUnited/GameLogic/Managers/ActionManager/Waypoint.h"
#include "GodsUnited/Player/Components/WaypointMovementComponent.h"

UGenericDragDropOperation::UGenericDragDropOperation()
{
	DraggingActor = nullptr;
	DraggingActorClass = nullptr;
	FinalActorClass = nullptr;
	LastValidDropLocation = FVector::ZeroVector;
	CachedEnergyCost = 0;
	ItemId = TEXT("");
}

void UGenericDragDropOperation::StartDragOperation(TSubclassOf<AActor> InDraggingActorClass,
                                                   TSubclassOf<AActor> InFinalActorClass,
                                                   const FString& InItemId,
                                                   const FPointerEvent& PointerEvent)
{
	// Validate input parameters
	if (!InDraggingActorClass)
	{
		UE_LOG(LogTemp, Error, TEXT("GenericDragDropOperation: DraggingActorClass is null"));
		return;
	}

	// FinalActorClass is optional for movement waypoints
	if (!InFinalActorClass && !InItemId.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("GenericDragDropOperation: FinalActorClass is null for card drag"));
	}

	// Validate cached player controller
	if (!CachedPC.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("GenericDragDropOperation: CachedPC is invalid"));
		return;
	}

	DraggingActorClass = InDraggingActorClass;
	FinalActorClass = InFinalActorClass;
	ItemId = InItemId;

	// Log drag operation type
	FString DragType = IsMovementDrag() ? TEXT("Movement") : TEXT("Card");
	UE_LOG(LogTemp, Display, TEXT("GenericDragDropOperation: Starting %s drag operation"), *DragType);

	SpawnDraggingActor();
	
	// Only proceed with initial drag if spawning succeeded
	if (DraggingActor)
	{
		OnDragging(PointerEvent);
	}
}

void UGenericDragDropOperation::SpawnDraggingActor()
{
	// Validate actor class
	if (!DraggingActorClass)
	{
		UE_LOG(LogTemp, Error, TEXT("GenericDragDropOperation: DraggingActorClass is null"));
		return;
	}

	// Validate player controller
	APlayerController* PC = CachedPC.Get();
	if (!PC || !IsValid(PC))
	{
		UE_LOG(LogTemp, Error, TEXT("GenericDragDropOperation: PlayerController is invalid"));
		return;
	}

	// Validate world context
	UWorld* World = PC->GetWorld();
	if (!World || !IsValid(World))
	{
		UE_LOG(LogTemp, Error, TEXT("GenericDragDropOperation: World is invalid"));
		return;
	}

	// Clean up existing dragging actor if any
	DestroyDraggingActor();

	// Spawn new dragging actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	DraggingActor = World->SpawnActor<AActor>(DraggingActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	
	if (!DraggingActor)
	{
		UE_LOG(LogTemp, Error, TEXT("GenericDragDropOperation: Failed to spawn dragging actor"));
		return;
	}
	
	// Disable collision for preview actor
	if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(DraggingActor->GetRootComponent()))
	{
		Root->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	UE_LOG(LogTemp, Display, TEXT("GenericDragDropOperation: Successfully spawned dragging actor"));
}

void UGenericDragDropOperation::Dragged_Implementation(const FPointerEvent& PointerEvent)
{
	Super::Dragged_Implementation(PointerEvent);
	OnDragging(PointerEvent);
}

void UGenericDragDropOperation::Drop_Implementation(const FPointerEvent& PointerEvent)
{
	Super::Drop_Implementation(PointerEvent);
    
	APlayerController* PC = CachedPC.Get();
	if (!PC) 
	{
		DestroyDraggingActor();
		return;
	}

	// Update drop position
	FVector2D Pixel, Unused;
	USlateBlueprintLibrary::AbsoluteToViewport(PC, PointerEvent.GetScreenSpacePosition(), Pixel, Unused);

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(GenericDragTrace), false);
	if (DraggingActor)
		Params.AddIgnoredActor(DraggingActor);

	bool bHit = PC->GetHitResultAtScreenPosition(Pixel, ECC_Visibility, false, Hit);
	
	if (bHit)
	{
		LastValidDropLocation = Hit.Location;
	}

	// Execute waypoint placement and final actor spawn
	bool bPlacementSuccess = false;
	if (bHit || LastValidDropLocation != FVector::ZeroVector)
	{
		FHitResult FinalHit;
		FinalHit.bBlockingHit = true;
		FinalHit.Location = bHit ? Hit.Location : LastValidDropLocation;
		
		bPlacementSuccess = ExecuteWaypointPlacement(FinalHit);
	}

	if (!bPlacementSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("GenericDragDropOperation: Placement failed"));
	}
    
	// Always destroy dragging actor
	DestroyDraggingActor();
}

bool UGenericDragDropOperation::ExecuteWaypointPlacement(const FHitResult& HitResult)
{
	if (!PlayerCharacter || !PlayerCharacter->GetWaypointMovementComponent())
	{
		UE_LOG(LogTemp, Error, TEXT("GenericDragDropOperation: PlayerCharacter or WaypointMovementComponent is null"));
		return false;
	}

	UWaypointMovementComponent* MovementComp = PlayerCharacter->GetWaypointMovementComponent();
	FVector PlacementLocation = HitResult.Location;

	// Validate placement using component logic
	int32 EnergyCost = 0;
	bool bCanPlace = false;

	if (IsMovementDrag())
	{
		// Movement waypoint validation
		bCanPlace = MovementComp->CanPlaceMovementWaypoint(PlacementLocation);
		if (bCanPlace)
		{
			EnergyCost = MovementComp->CalculateWaypointEnergyCost(PlacementLocation);
			bCanPlace = MovementComp->HasSufficientEnergy(EnergyCost);
		}
	}
	else
	{
		// Card waypoint validation
		bCanPlace = MovementComp->CanPlaceCardWaypoint(PlacementLocation, EnergyCost);
	}

	if (!bCanPlace)
	{
		// Provide visual feedback for failed placement
		UWorld* World = PlayerCharacter->GetWorld();
		if (World)
		{
			DrawDebugSphere(World, PlacementLocation, CardSystemConstants::DEBUG_SPHERE_SIZE * 2.0f, 
				PlayerDebugConstants::SPHERE_SEGMENTS, FColor::Red, false, 2.0f);
			
			FString Reason = IsMovementDrag() ? TEXT("Too Close or Insufficient Energy") : TEXT("Insufficient Energy");
			DrawDebugString(World, PlacementLocation + FVector(0, 0, 100), *Reason, 
				nullptr, FColor::Red, 2.0f);
		}
		return false;
	}

	// Create waypoint using component
	MovementComp->HandleMouseClick(HitResult, ItemId);

	// Spawn final actor if specified
	if (FinalActorClass)
	{
		AActor* FinalActor = SpawnFinalActor(PlacementLocation);
		if (FinalActor)
		{
			UE_LOG(LogTemp, Display, TEXT("GenericDragDropOperation: Successfully spawned final actor"));
		}
	}

	FString PlacementType = IsMovementDrag() ? TEXT("Movement") : TEXT("Card");
	UE_LOG(LogTemp, Display, TEXT("GenericDragDropOperation: Successfully placed %s waypoint. Energy cost: %d"), 
		*PlacementType, EnergyCost);

	return true;
}

AActor* UGenericDragDropOperation::SpawnFinalActor(const FVector& SpawnLocation)
{
	if (!FinalActorClass)
		return nullptr;

	APlayerController* PC = CachedPC.Get();
	if (!PC)
		return nullptr;

	UWorld* World = PC->GetWorld();
	if (!World)
		return nullptr;

	// Spawn final actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	AActor* FinalActor = World->SpawnActor<AActor>(FinalActorClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	
	if (FinalActor)
	{
		// Adjust position based on actor bounds
		FVector Origin, BoxExtent;
		FinalActor->GetActorBounds(false, Origin, BoxExtent);
		
		FVector AdjustedLocation = SpawnLocation;
		AdjustedLocation.Z += BoxExtent.Z;
		
		FinalActor->SetActorLocation(AdjustedLocation);
		
		// Provide visual feedback for successful placement
		float CircleRadius = FMath::Max(BoxExtent.X, BoxExtent.Y) * CardSystemConstants::DRAG_CIRCLE_RADIUS_MULTIPLIER;
		DrawDebugCircle(World, AdjustedLocation, CircleRadius, CardSystemConstants::DEBUG_CIRCLE_SEGMENTS, 
			FColor::Green, false, CardSystemConstants::DEBUG_DURATION_LONG, 0, CardSystemConstants::DEBUG_CIRCLE_THICKNESS, 
			FVector(0,1,0), FVector(1,0,0));
	}

	return FinalActor;
}

void UGenericDragDropOperation::DragCancelled_Implementation(const FPointerEvent& PointerEvent)
{
	Super::DragCancelled_Implementation(PointerEvent);
	DestroyDraggingActor();
}

void UGenericDragDropOperation::DestroyDraggingActor()
{
	if (DraggingActor && IsValid(DraggingActor))
	{
		DraggingActor->Destroy();
		DraggingActor = nullptr;
	}
}

void UGenericDragDropOperation::OnDragging(const FPointerEvent& PointerEvent)
{
	APlayerController* PC = CachedPC.Get();
	if (!PC) return;
	UWorld* World = PC->GetWorld();
	if (!World) return;

	// Update drag location and positioning
	FVector DragLocation = UpdateDragLocation(PointerEvent);
	
	// Calculate and display energy cost
	CalculateAndDisplayEnergyCost(DragLocation);
}

FVector UGenericDragDropOperation::UpdateDragLocation(const FPointerEvent& PointerEvent)
{
	// Validate basic requirements
	APlayerController* PC = CachedPC.Get();
	if (!PC || !IsValid(PC))
	{
		UE_LOG(LogTemp, Warning, TEXT("GenericDragDropOperation: Invalid PlayerController in UpdateDragLocation"));
		return FVector::ZeroVector;
	}
	
	UWorld* World = PC->GetWorld();
	if (!World || !IsValid(World))
	{
		UE_LOG(LogTemp, Warning, TEXT("GenericDragDropOperation: Invalid World in UpdateDragLocation"));
		return FVector::ZeroVector;
	}

	// Validate pointer event
	FVector2D ScreenPos = PointerEvent.GetScreenSpacePosition();
	if (ScreenPos.ContainsNaN())
	{
		UE_LOG(LogTemp, Warning, TEXT("GenericDragDropOperation: Screen position contains NaN values"));
		return FVector::ZeroVector;
	}

	UE_LOG(LogTemp, Error, TEXT("=== DRAG LOCATION DEBUG ==="));
	UE_LOG(LogTemp, Error, TEXT("PointerEvent ScreenPos: %s"), *PointerEvent.GetScreenSpacePosition().ToString());
    
	// Get viewport pixel position
	FVector2D Pixel, Unused;
	USlateBlueprintLibrary::AbsoluteToViewport(PC, PointerEvent.GetScreenSpacePosition(), Pixel, Unused);
	UE_LOG(LogTemp, Error, TEXT("Converted to Viewport Pixel: %s"), *Pixel.ToString());

	// Perform raycast to ground
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(GenericDragTrace), false);
	if (DraggingActor && IsValid(DraggingActor)) 
	{
		Params.AddIgnoredActor(DraggingActor);
	}
	
	FVector AdjustedLocation = FVector::ZeroVector;

	if (PC->GetHitResultAtScreenPosition(Pixel, ECC_Visibility, false, Hit))
	{
		UE_LOG(LogTemp, Error, TEXT("Hit Location: %s"), *Hit.Location.ToString());
		
		// Validate hit result
		if (!Hit.bBlockingHit || Hit.Location.ContainsNaN())
		{
			UE_LOG(LogTemp, Warning, TEXT("GenericDragDropOperation: Invalid hit result"));
			return FVector::ZeroVector;
		}

		LastValidDropLocation = Hit.Location;

		if (DraggingActor && IsValid(DraggingActor))
		{
			// Calculate actor bounds for proper ground positioning
			FVector Origin, BoxExtent;
			DraggingActor->GetActorBounds(false, Origin, BoxExtent);
			
			// Validate bounds
			if (BoxExtent.ContainsNaN() || BoxExtent.IsNearlyZero())
			{
				UE_LOG(LogTemp, Warning, TEXT("GenericDragDropOperation: Invalid actor bounds"));
				BoxExtent = FVector(50.0f, 50.0f, 50.0f); // Default fallback
			}
			
			// Adjust position so feet touch the ground
			AdjustedLocation = Hit.Location;
			AdjustedLocation.Z += BoxExtent.Z; // Half height of actor upward
			
			// Check for snap to last waypoint (only for movement drags)
			if (IsMovementDrag())
			{
				FVector SnappedLocation = CheckLastWaypointSnap(AdjustedLocation);
				bool bIsSnapping = (SnappedLocation != AdjustedLocation);
				
				// Use snapped location if snapping occurred
				if (bIsSnapping)
				{
					AdjustedLocation = SnappedLocation;
				}
				
				// Render snap visualization
				RenderSnapVisualization(World, AdjustedLocation, bIsSnapping);
			}
			
			// Validate final location
			if (!AdjustedLocation.ContainsNaN())
			{
				DraggingActor->SetActorLocation(AdjustedLocation);
				
				// Render drag visualization
				RenderDragVisualization(World, AdjustedLocation, BoxExtent);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("GenericDragDropOperation: Adjusted location contains NaN values"));
			}
		}
		
		// Draw ground hit indicator
		DrawDebugSphere(World, LastValidDropLocation + FVector(0, 0, CardSystemConstants::DEBUG_SPHERE_HEIGHT_OFFSET), 
			CardSystemConstants::DEBUG_SPHERE_SIZE, PlayerDebugConstants::SPHERE_SEGMENTS, 
			FColor::Green, false, CardSystemConstants::DEBUG_DURATION_SHORT);
	}

	return AdjustedLocation;
}

void UGenericDragDropOperation::CalculateAndDisplayEnergyCost(const FVector& DragLocation)
{
	if (!PlayerCharacter || !PlayerCharacter->GetWaypointMovementComponent())
	{
		UE_LOG(LogTemp, Warning, TEXT("GenericDragDropOperation: PlayerCharacter or WaypointMovementComponent is null"));
		return;
	}
	
	UWaypointMovementComponent* MovementComp = PlayerCharacter->GetWaypointMovementComponent();
	int32 EnergyCost = MovementComp->CalculateWaypointEnergyCost(DragLocation);
	
	// Cache the cost for drop validation
	CachedEnergyCost = EnergyCost;
	
	// Check affordability
	bool bCanAfford = MovementComp->HasSufficientEnergy(EnergyCost);
	
	FString DragType = IsMovementDrag() ? TEXT("Movement") : TEXT("Card");
	
	if (EnergyCost == 0)
	{
		UE_LOG(LogTemp, Verbose, TEXT("GenericDragDropOperation: %s drag within free placement radius"), *DragType);
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("GenericDragDropOperation: %s drag energy cost: %d rings - %s"), 
			*DragType, EnergyCost, bCanAfford ? TEXT("Affordable") : TEXT("Too Expensive"));
	}
}

void UGenericDragDropOperation::RenderDragVisualization(UWorld* World, const FVector& ActorLocation, const FVector& BoxExtent)
{
	if (!World)
		return;

	// Draw debug circle around actor indicating placement area
	float CircleRadius = FMath::Max(BoxExtent.X, BoxExtent.Y) * CardSystemConstants::DRAG_CIRCLE_RADIUS_MULTIPLIER;
	FColor CircleColor = IsMovementDrag() ? FColor::Blue : FColor::Green;
	
	DrawDebugCircle(World, ActorLocation, CircleRadius, CardSystemConstants::DEBUG_CIRCLE_SEGMENTS, 
		CircleColor, false, CardSystemConstants::DEBUG_DURATION_SHORT, 0, PlayerDebugConstants::LINE_THICKNESS, 
		FVector(0,1,0), FVector(1,0,0));
}

FVector UGenericDragDropOperation::CheckLastWaypointSnap(const FVector& DragLocation)
{
	if (!PlayerCharacter)
	{
		return DragLocation;
	}

	// Get last waypoint from character
	AWaypoint* LastWaypoint = PlayerCharacter->GetLastWaypoint();
	if (!LastWaypoint)
	{
		return DragLocation;
	}

	// Check distance to last waypoint
	FVector WaypointLocation = LastWaypoint->GetActorLocation();
	float SnapDistance = GetSnapRadius();
	
	// 2D distance check (ignore Z)
	FVector DragLocation2D = DragLocation;
	FVector WaypointLocation2D = WaypointLocation;
	DragLocation2D.Z = 0;
	WaypointLocation2D.Z = 0;
	
	float Distance = FVector::Dist(DragLocation2D, WaypointLocation2D);
	
	if (Distance <= SnapDistance)
	{
		// Snap to waypoint location but keep original Z
		FVector SnapLocation = WaypointLocation;
		SnapLocation.Z = DragLocation.Z;
		
		UE_LOG(LogTemp, Display, TEXT("GenericDragDropOperation: Snapping to last waypoint at distance %.2f"), Distance);
		return SnapLocation;
	}
	
	return DragLocation;
}

float UGenericDragDropOperation::GetSnapRadius() const
{
	return CardSystemConstants::PLACEMENT_RADIUS;
}

void UGenericDragDropOperation::RenderSnapVisualization(UWorld* World, const FVector& SnapLocation, bool bIsSnapping)
{
	if (!World)
		return;

	// Different visual feedback based on snap state
	FColor CircleColor = bIsSnapping ? FColor::Orange : FColor::Blue;
	float CircleThickness = bIsSnapping ? 5.0f : PlayerDebugConstants::LINE_THICKNESS;
	
	// Draw snap radius circle
	DrawDebugCircle(World, SnapLocation, GetSnapRadius(), CardSystemConstants::DEBUG_CIRCLE_SEGMENTS, 
		CircleColor, false, CardSystemConstants::DEBUG_DURATION_SHORT, 0, CircleThickness, 
		FVector(0,1,0), FVector(1,0,0));
		
	if (bIsSnapping)
	{
		// Draw snap indicator
		DrawDebugSphere(World, SnapLocation, CardSystemConstants::DEBUG_SPHERE_SIZE * 1.5f, 
			PlayerDebugConstants::SPHERE_SEGMENTS, FColor::Orange, false, CardSystemConstants::DEBUG_DURATION_SHORT);
	}
}

bool UGenericDragDropOperation::CanAffordEnergyDrop(int32 RequiredEnergy) const
{
	if (!PlayerCharacter || !PlayerCharacter->GetWaypointMovementComponent())
		return false;
		
	return PlayerCharacter->GetWaypointMovementComponent()->HasSufficientEnergy(RequiredEnergy);
}