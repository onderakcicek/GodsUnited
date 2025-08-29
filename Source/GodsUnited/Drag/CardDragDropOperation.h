// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Engine/World.h"
#include "CardDragDropOperation.generated.h"

/**
 * 
 */
UCLASS()
class GODSUNITED_API UCardDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UCardDragDropOperation();

	// Spawn edilecek dragging actor class'ı
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drag Drop")
	TSubclassOf<AActor> DraggingActorClass;

	// Spawn edilen dragging actor referansı
	UPROPERTY(BlueprintReadOnly, Category = "Drag Drop")
	AActor* DraggingActor;

	// Final spawn edilecek actor class'ı
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drag Drop")
	TSubclassOf<AActor> FinalActorClass;

	// Drag operation başlatma
	UFUNCTION(BlueprintCallable, Category="Drag Drop")
	void StartDragOperation(TSubclassOf<AActor> InDraggingActorClass,
							TSubclassOf<AActor> InFinalActorClass,
							const FPointerEvent& PointerEvent);

	UPROPERTY()
	TWeakObjectPtr<APlayerController> CachedPC;
	
protected:
	// Override functions
	virtual void Dragged_Implementation(const FPointerEvent& PointerEvent) override;
	virtual void Drop_Implementation(const FPointerEvent& PointerEvent) override;
	virtual void DragCancelled_Implementation(const FPointerEvent& PointerEvent) override;

private:
	
	UPROPERTY()
	FVector LastValidDropLocation;

	void UpdateDraggingActorPosition(const FPointerEvent& PointerEvent);
	void SpawnDraggingActor();
	void DestroyDraggingActor();
};
