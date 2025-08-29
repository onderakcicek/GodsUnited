// Fill out your copyright notice in the Description page of Project Settings.


#include "CardDragDropOperation.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/Widget.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

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
	UpdateDraggingActorPosition(PointerEvent); // ilk karede
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
	UpdateDraggingActorPosition(PointerEvent);
}

void UCardDragDropOperation::Drop_Implementation(const FPointerEvent& PointerEvent)
{
	Super::Drop_Implementation(PointerEvent);
    
	APlayerController* PC = CachedPC.Get();
	if (!PC) return;
	UWorld* World = PC->GetWorld();
	if (!World) return;

	// Drop pozisyonunu güncelle
	FVector2D Pixel, Unused;
	USlateBlueprintLibrary::AbsoluteToViewport(PC, PointerEvent.GetScreenSpacePosition(), Pixel, Unused);
	FVector2D Screen = Pixel;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(CardDragTrace), /*bTraceComplex=*/false);
	if (DraggingActor)
		Params.AddIgnoredActor(DraggingActor);

	bool bHit = PC->GetHitResultAtScreenPosition(Screen, ECC_Visibility, false, Hit);
	
	// Debug için log ekleyelim
	UE_LOG(LogTemp, Warning, TEXT("Drop - Hit: %s, FinalActorClass: %s, World: %s"), 
		bHit ? TEXT("True") : TEXT("False"),
		FinalActorClass ? TEXT("Valid") : TEXT("Invalid"),
		World ? TEXT("Valid") : TEXT("Invalid"));

	if (bHit)
	{
		LastValidDropLocation = Hit.Location;
	}
    
	// Final actor'ü spawn et
	if (FinalActorClass && (bHit || LastValidDropLocation != FVector::ZeroVector))
	{
		FVector SpawnLocation = bHit ? Hit.Location : LastValidDropLocation;
		
		// Actor'ü spawn et
		AActor* SpawnedActor = World->SpawnActor<AActor>(FinalActorClass, SpawnLocation, FRotator::ZeroRotator);
		
		if (SpawnedActor)
		{
			// Actor'ün bounds'unu hesapla
			FVector Origin, BoxExtent;
			SpawnedActor->GetActorBounds(false, Origin, BoxExtent);
			
			// Ayakların zemine basması için pozisyonu ayarla
			FVector AdjustedLocation = SpawnLocation;
			AdjustedLocation.Z += BoxExtent.Z; // Actor'ün yarı yüksekliği kadar yukarı
			
			SpawnedActor->SetActorLocation(AdjustedLocation);
			
			// Debug log
			UE_LOG(LogTemp, Warning, TEXT("Spawned Actor: Success at location: %s, BoxExtent.Z: %f"), 
				*AdjustedLocation.ToString(), BoxExtent.Z);
			
			// Karakterin etrafında debug circle çiz (karakter merkezde)
			float CircleRadius = FMath::Max(BoxExtent.X, BoxExtent.Y) * 10.f; // Biraz daha büyük radius
			DrawDebugCircle(World, AdjustedLocation, CircleRadius, 32, FColor::Blue, false, 3.0f, 0, 3.0f, FVector(0,1,0), FVector(1,0,0));
			
			// Spawned actor için debug sphere (ayakların altında)
			DrawDebugSphere(World, SpawnLocation, 20.f, 12, FColor::Black, false, 3.0f);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to spawn actor at location: %s"), *SpawnLocation.ToString());
		}
	}
    
	// Dragging actor'ü sil
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

void UCardDragDropOperation::UpdateDraggingActorPosition(const FPointerEvent& PointerEvent)
{
	APlayerController* PC = CachedPC.Get();
	if (!PC) return;
	UWorld* World = PC->GetWorld();
	if (!World) return;

	// Viewport piksel pozisyonu al
	FVector2D Pixel, Unused;
	USlateBlueprintLibrary::AbsoluteToViewport(PC, PointerEvent.GetScreenSpacePosition(), Pixel, Unused);
	FVector2D Screen = Pixel;

	// Raycast zemine
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(CardDragTrace), /*bTraceComplex=*/false);
	if (DraggingActor) Params.AddIgnoredActor(DraggingActor);

	if (PC->GetHitResultAtScreenPosition(Screen, ECC_Visibility, false, Hit))
	{
		LastValidDropLocation = Hit.Location;

		if (DraggingActor)
		{
			// Actor'ün bounds'unu hesapla
			FVector Origin, BoxExtent;
			DraggingActor->GetActorBounds(false, Origin, BoxExtent);
			
			// Ayakların zemine basması için pozisyonu ayarla
			FVector AdjustedLocation = Hit.Location;
			AdjustedLocation.Z += BoxExtent.Z; // Actor'ün yarı yüksekliği kadar yukarı
			
			DraggingActor->SetActorLocation(AdjustedLocation);
			
			// Karakterin etrafında debug circle çiz (karakter merkezde)
			float CircleRadius = FMath::Max(BoxExtent.X, BoxExtent.Y) * 10.f; // Biraz daha büyük radius
			DrawDebugCircle(World, AdjustedLocation, CircleRadius, 32, FColor::Green, false, 0.03f, 0, 2.0f, FVector(0,1,0), FVector(1,0,0));
		}
		
		DrawDebugSphere(World, LastValidDropLocation + FVector(0,0,10), 20.f, 12, FColor::Green, false, 0.03f);
	}
}

FORCEINLINE bool IsInsideCircle2D(const FVector& Center, const FVector& Point, float Radius)
{
	const float dx = Point.X - Center.X;
	const float dy = Point.Y - Center.Y;
	return (dx*dx + dy*dy) <= (Radius * Radius);
}

FORCEINLINE float CircleDistanceInDiameters_NoSqrt(const FVector& Center, const FVector& Point, float Radius)
{
	const float dx = Point.X - Center.X;
	const float dy = Point.Y - Center.Y;
	const float distSq = dx*dx + dy*dy;
	const float diamSq  = (2.f * Radius) * (2.f * Radius);

	// kaç çapın karesi
	return FMath::Sqrt(distSq / diamSq);
}