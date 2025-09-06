#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Engine/HitResult.h"
#include "CardDragDropOperation.generated.h"

class ABaseCharacter;

/**
 * Custom drag drop operation for card system
 * Handles spawning temporary actors during drag and final placement on drop
 */
UCLASS()
class GODSUNITED_API UCardDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UCardDragDropOperation();

	//-------------------------------------------
	// Drag Operation Lifecycle
	//-------------------------------------------
	
	/**
	 * Initialize and start the drag operation
	 * @param InDraggingActorClass Class to spawn for drag preview
	 * @param InFinalActorClass Class to spawn on successful drop
	 * @param PointerEvent Initial pointer event that started the drag
	 */
	void StartDragOperation(TSubclassOf<AActor> InDraggingActorClass,
	                        TSubclassOf<AActor> InFinalActorClass,
	                        const FPointerEvent& PointerEvent);

	//-------------------------------------------
	// UDragDropOperation Overrides
	//-------------------------------------------
	
	virtual void Dragged_Implementation(const FPointerEvent& PointerEvent) override;
	virtual void Drop_Implementation(const FPointerEvent& PointerEvent) override;
	virtual void DragCancelled_Implementation(const FPointerEvent& PointerEvent) override;

	//-------------------------------------------
	// Public Properties
	//-------------------------------------------
	
	/** Cached player controller reference for performance */
	UPROPERTY()
	TWeakObjectPtr<APlayerController> CachedPC;

	/** Reference to player character for energy calculations */
	UPROPERTY()
	ABaseCharacter* PlayerCharacter;

protected:
	//-------------------------------------------
	// Drag Actor Management
	//-------------------------------------------
	
	/**
	 * Spawn the temporary actor used during dragging
	 */
	void SpawnDraggingActor();

	/**
	 * Clean up and destroy the dragging actor
	 */
	void DestroyDraggingActor();

	//-------------------------------------------
	// Drag Event Handling
	//-------------------------------------------
	
	/**
	 * Main drag handling logic called during drag events
	 * @param PointerEvent Current pointer/mouse event
	 */
	void OnDragging(const FPointerEvent& PointerEvent);

	/**
	 * Update drag location and position dragging actor
	 * @param PointerEvent Current pointer event
	 * @return Adjusted world location of the dragged actor
	 */
	FVector UpdateDragLocation(const FPointerEvent& PointerEvent);

	/**
	 * Calculate and display energy cost for current drag position
	 * @param DragLocation Current world position of dragged actor
	 */
	void CalculateAndDisplayEnergyCost(const FVector& DragLocation);

	/**
	 * Render visual feedback during drag operation
	 * @param World World context for rendering
	 * @param ActorLocation Position of the dragged actor
	 * @param BoxExtent Bounds of the dragged actor
	 */
	void RenderDragVisualization(UWorld* World, const FVector& ActorLocation, const FVector& BoxExtent);

protected:
	//-------------------------------------------
	// Drag State
	//-------------------------------------------
	
	/** Actor class spawned during drag for preview */
	UPROPERTY()
	TSubclassOf<AActor> DraggingActorClass;

	/** Actor class spawned on successful drop */
	UPROPERTY()
	TSubclassOf<AActor> FinalActorClass;

	/** Temporary actor instance used during drag */
	UPROPERTY()
	AActor* DraggingActor;

	/** Last valid world location for drop placement */
	UPROPERTY()
	FVector LastValidDropLocation;
};