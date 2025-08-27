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

	/*if (FinalActorClass)
	{
	    if (UWorld* World = GetWorld())
	    {
	        const FVector SpawnLoc = GetWorldPosFromMouse(PointerEvent);
	        World->SpawnActor<AActor>(FinalActorClass, SpawnLoc, FRotator::ZeroRotator);
	    }
	}

	DestroyDraggingActor();*/
}

void UCardDragDropOperation::DragCancelled_Implementation(const FPointerEvent& PointerEvent)
{
	Super::DragCancelled_Implementation(PointerEvent);
	//DestroyDraggingActor();
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
		DraggingActor->SetActorLocation(Hit.Location);
		DrawDebugSphere(World, Hit.Location + FVector(0,0,10), 20.f, 12, FColor::Green, false, 0.03f);
	}
}