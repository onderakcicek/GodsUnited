
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Engine/HitResult.h"
#include "GenericDragDropOperation.generated.h"

class ABaseCharacter;

/**
 * Generic drag drop operation for both movement and card systems
 * Handles spawning temporary actors during drag and final placement on drop
 */
UCLASS()
class GODSUNITED_API UGenericDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UGenericDragDropOperation();

	//-------------------------------------------
	// Drag Operation Lifecycle
	//-------------------------------------------
	
	/**
	 * Initialize and start the drag operation
	 * @param InDraggingActorClass Class to spawn for drag preview
	 * @param InFinalActorClass Class to spawn on successful drop
	 * @param InItemId Item identifier (empty for movement, filled for cards)
	 * @param PointerEvent Initial pointer event that started the drag
	 */
	void StartDragOperation(TSubclassOf<AActor> InDraggingActorClass,
	                        TSubclassOf<AActor> InFinalActorClass,
	                        const FString& InItemId,
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

	/** Item identifier for this drag operation */
	UPROPERTY()
	FString ItemId;

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
	// Drop Validation and Execution
	//-------------------------------------------

	/**
	 * Validate and execute waypoint placement
	 * @param HitResult Hit result for drop location
	 * @return True if placement was successful
	 */
	bool ExecuteWaypointPlacement(const FHitResult& HitResult);

	/**
	 * Spawn final actor if specified
	 * @param SpawnLocation World location for final actor
	 * @return Spawned actor or nullptr if failed
	 */
	AActor* SpawnFinalActor(const FVector& SpawnLocation);

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

	//-------------------------------------------
	// Snap Detection
	//-------------------------------------------

	/**
	 * Check if drag position should snap to last waypoint
	 * @param DragLocation Current drag position
	 * @return Snap position if should snap, original position otherwise
	 */
	FVector CheckLastWaypointSnap(const FVector& DragLocation);

	/**
	 * Get snap radius for waypoint detection
	 */
	float GetSnapRadius() const;

	/**
	 * Render snap visualization feedback
	 * @param World World context
	 * @param SnapLocation Position to show snap feedback
	 * @param bIsSnapping Whether currently snapping
	 */
	void RenderSnapVisualization(UWorld* World, const FVector& SnapLocation, bool bIsSnapping);
	
	//-------------------------------------------
	// Drag State
	//-------------------------------------------
	
	/** Actor class spawned during drag for preview */
	UPROPERTY()
	TSubclassOf<AActor> DraggingActorClass;
	
	/** Actor class spawned on successful drop (optional) */
	UPROPERTY()
	TSubclassOf<AActor> FinalActorClass;

	/** Temporary actor instance used during drag */
	UPROPERTY()
	AActor* DraggingActor;

	/** Last valid world location for drop placement */
	UPROPERTY()
	FVector LastValidDropLocation;

	/** Cached energy cost for current drag position */
	UPROPERTY()
	int32 CachedEnergyCost;

private:
	/**
	 * Check if player can afford the energy cost for placement
	 * @param RequiredEnergy Energy cost for placement
	 * @return True if affordable
	 */
	bool CanAffordEnergyDrop(int32 RequiredEnergy) const;

	/**
	 * Determine if this is a movement waypoint (empty ItemId)
	 * @return True if movement waypoint
	 */
	bool IsMovementDrag() const { return ItemId.IsEmpty(); }

	/**
	 * Determine if this is a card waypoint (filled ItemId)
	 * @return True if card waypoint
	 */
	bool IsCardDrag() const { return !ItemId.IsEmpty(); }
};