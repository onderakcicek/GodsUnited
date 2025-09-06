// Fill out your copyright notice in the Description page of Project Settings.

#include "CardDragDropOperation.h"
#include "../Player/BasePlayerDefinitions.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"
#include "GodsUnited/Player/Character/BaseCharacter.h"

FORCEINLINE bool IsInsideCircle2D(const FVector& Center, const FVector& Point, float Radius)
{
	const float dx = Point.X - Center.X;
	const float dy = Point.Y - Center.Y;
	return (dx*dx + dy*dy) <= (Radius * Radius);
}

FORCEINLINE float CircleDistanceInDiameters(const FVector& Center, const FVector& Point, float Radius)
{
	const float dx = Point.X - Center.X;
	const float dy = Point.Y - Center.Y;
	const float distSq = dx*dx + dy*dy;
	const float diamSq  = (2.f * Radius) * (2.f * Radius);

	// How many diameters squared
	return FMath::Sqrt(distSq / diamSq);
}

UCardDragDropOperation::UCardDragDropOperation()
{
	DraggingActor = nullptr;
	DraggingActorClass = nullptr;
	FinalActorClass = nullptr;
	LastValidDropLocation = FVector::ZeroVector;
}

void UCardDragDropOperation::StartDragOperation(TSubclassOf<AActor> InDraggingActorClass,
                                                TSubclassOf<AActor> InFinalActorClass,
                                                const FPointerEvent& PointerEvent)
{
	DraggingActorClass = InDraggingActorClass;
	FinalActorClass = InFinalActorClass;

	SpawnDraggingActor();
	OnDragging(PointerEvent); // First frame
}

void UCardDragDropOperation::SpawnDraggingActor()
{
	if (!DraggingActorClass) return;

	APlayerController* PC = CachedPC.Get();
	if (!PC) return;
	UWorld* World = PC->GetWorld();
	if (!World) return;

	DraggingActor = World->SpawnActor<AActor>(DraggingActorClass, FVector::ZeroVector, FRotator::ZeroRotator);
	if (!DraggingActor) return;
	
	if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(DraggingActor->GetRootComponent()))
	{
		Root->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void UCardDragDropOperation::Dragged_Implementation(const FPointerEvent& PointerEvent)
{
	Super::Dragged_Implementation(PointerEvent);
	OnDragging(PointerEvent);
}

void UCardDragDropOperation::Drop_Implementation(const FPointerEvent& PointerEvent)
{
	Super::Drop_Implementation(PointerEvent);
    
	APlayerController* PC = CachedPC.Get();
	if (!PC) return;
	UWorld* World = PC->GetWorld();
	if (!World) return;

	// Update drop position
	FVector2D Pixel, Unused;
	USlateBlueprintLibrary::AbsoluteToViewport(PC, PointerEvent.GetScreenSpacePosition(), Pixel, Unused);
	FVector2D Screen = Pixel;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(CardDragTrace), /*bTraceComplex=*/false);
	if (DraggingActor)
		Params.AddIgnoredActor(DraggingActor);

	bool bHit = PC->GetHitResultAtScreenPosition(Screen, ECC_Visibility, false, Hit);
	
	// Debug log
	UE_LOG(LogTemp, Warning, TEXT("Drop - Hit: %s, FinalActorClass: %s, World: %s"), 
		bHit ? TEXT("True") : TEXT("False"),
		FinalActorClass ? TEXT("Valid") : TEXT("Invalid"),
		World ? TEXT("Valid") : TEXT("Invalid"));

	if (bHit)
	{
		LastValidDropLocation = Hit.Location;
	}
    
	// Spawn final actor
	if (FinalActorClass && (bHit || LastValidDropLocation != FVector::ZeroVector))
	{
		FVector SpawnLocation = bHit ? Hit.Location : LastValidDropLocation;
		
		// Spawn the actor
		AActor* SpawnedActor = World->SpawnActor<AActor>(FinalActorClass, SpawnLocation, FRotator::ZeroRotator);
		
		if (SpawnedActor)
		{
			// Calculate actor bounds
			FVector Origin, BoxExtent;
			SpawnedActor->GetActorBounds(false, Origin, BoxExtent);
			
			// Adjust position so feet touch the ground
			FVector AdjustedLocation = SpawnLocation;
			AdjustedLocation.Z += BoxExtent.Z; // Half height of actor upward
			
			SpawnedActor->SetActorLocation(AdjustedLocation);
			
			// Debug log
			UE_LOG(LogTemp, Warning, TEXT("Spawned Actor: Success at location: %s, BoxExtent.Z: %f"), 
				*AdjustedLocation.ToString(), BoxExtent.Z);
			
			// Draw debug circle around character (character at center)
			float CircleRadius = FMath::Max(BoxExtent.X, BoxExtent.Y) * CardSystemConstants::DRAG_CIRCLE_RADIUS_MULTIPLIER;
			DrawDebugCircle(World, AdjustedLocation, CircleRadius, CardSystemConstants::DEBUG_CIRCLE_SEGMENTS, 
				FColor::Blue, false, CardSystemConstants::DEBUG_DURATION_LONG, 0, CardSystemConstants::DEBUG_CIRCLE_THICKNESS, 
				FVector(0,1,0), FVector(1,0,0));
			
			// Debug sphere for spawned actor (under the feet)
			DrawDebugSphere(World, SpawnLocation, CardSystemConstants::DEBUG_SPHERE_SIZE, 
				PlayerDebugConstants::SPHERE_SEGMENTS, FColor::Black, false, CardSystemConstants::DEBUG_DURATION_LONG);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to spawn actor at location: %s"), *SpawnLocation.ToString());
		}
	}
    
	// Destroy dragging actor
	DestroyDraggingActor();
}

void UCardDragDropOperation::DragCancelled_Implementation(const FPointerEvent& PointerEvent)
{
	Super::DragCancelled_Implementation(PointerEvent);
	DestroyDraggingActor();
}

void UCardDragDropOperation::DestroyDraggingActor()
{
	if (DraggingActor)
	{
		DraggingActor->Destroy();
		DraggingActor = nullptr;
	}
}

void UCardDragDropOperation::OnDragging(const FPointerEvent& PointerEvent)
{
	APlayerController* PC = CachedPC.Get();
	if (!PC) return;
	UWorld* World = PC->GetWorld();
	if (!World) return;

	//-------------------------------------------
	//  LOCATION HANDLING
	//-------------------------------------------
	
	// Get viewport pixel position
	FVector2D Pixel, Unused;
	USlateBlueprintLibrary::AbsoluteToViewport(PC, PointerEvent.GetScreenSpacePosition(), Pixel, Unused);
	FVector2D Screen = Pixel;

	// Raycast to ground
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(CardDragTrace), /*bTraceComplex=*/false);
	if (DraggingActor) Params.AddIgnoredActor(DraggingActor);
	
	FVector AdjustedLocation;

	if (PC->GetHitResultAtScreenPosition(Screen, ECC_Visibility, false, Hit))
	{
		LastValidDropLocation = Hit.Location;

		if (DraggingActor)
		{
			// Calculate actor bounds
			FVector Origin, BoxExtent;
			DraggingActor->GetActorBounds(false, Origin, BoxExtent);
			
			// Adjust position so feet touch the ground
			AdjustedLocation = Hit.Location;
			AdjustedLocation.Z += BoxExtent.Z; // Half height of actor upward
			
			DraggingActor->SetActorLocation(AdjustedLocation);
			
			// Draw debug circle around character (character at center)
			float CircleRadius = FMath::Max(BoxExtent.X, BoxExtent.Y) * CardSystemConstants::DRAG_CIRCLE_RADIUS_MULTIPLIER;
			DrawDebugCircle(World, AdjustedLocation, CircleRadius, CardSystemConstants::DEBUG_CIRCLE_SEGMENTS, 
				FColor::Green, false, CardSystemConstants::DEBUG_DURATION_SHORT, 0, PlayerDebugConstants::LINE_THICKNESS, 
				FVector(0,1,0), FVector(1,0,0));
		}
		
		DrawDebugSphere(World, LastValidDropLocation + FVector(0, 0, CardSystemConstants::DEBUG_SPHERE_HEIGHT_OFFSET), 
			CardSystemConstants::DEBUG_SPHERE_SIZE, PlayerDebugConstants::SPHERE_SEGMENTS, 
			FColor::Green, false, CardSystemConstants::DEBUG_DURATION_SHORT);
	}

	//-------------------------------------------
	//  ENERGY CALCULATION
	//-------------------------------------------
	
	// Get player's last location
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get PlayerChar"));
		return;
	}
	
	FVector PlayerLastLocation = PlayerCharacter->GetLastMoveLocation();
	if (IsInsideCircle2D(PlayerLastLocation, AdjustedLocation, CardSystemConstants::PLACEMENT_RADIUS))
	{
		UE_LOG(LogTemp, Warning, TEXT("Inside placement radius"));
	}

	float EnergyCost = CircleDistanceInDiameters(PlayerLastLocation, AdjustedLocation, CardSystemConstants::PLACEMENT_RADIUS);
	UE_LOG(LogTemp, Warning, TEXT("Energy Cost: %f"), EnergyCost);
}