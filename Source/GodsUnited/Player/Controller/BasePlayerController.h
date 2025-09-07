#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Engine/HitResult.h"
#include "BasePlayerController.generated.h"

class APvPGameMode;
class ABaseCharacter;
class UGenericDragDropOperation;

/**
 * Base player controller with drag and drop support for movement waypoints
 */
UCLASS()
class GODSUNITED_API ABasePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABasePlayerController();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

	//-------------------------------------------
	// Movement Waypoint Drag&Drop Configuration
	//-------------------------------------------

	/** Actor class to spawn during movement drag preview */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Drag Settings")
	TSubclassOf<AActor> MovementDraggingActorClass;

	/** Actor class to spawn on movement waypoint drop (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Drag Settings")
	TSubclassOf<AActor> MovementFinalActorClass;

	/** Minimum drag distance to start drag operation (prevents accidental drags) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Drag Settings")
	float MinimumDragDistance = 50.0f;

	//-------------------------------------------
	// Input Handling
	//-------------------------------------------

	/** Handle touch/mouse button down events for drag detection */
	void OnLeftMouseDown();

	/** Handle touch/mouse button up events */
	void OnLeftMouseUp();

	/** Handle mouse movement for drag detection */
	void OnMouseMove();

	/** Handle touch started events */
	void OnTouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handle touch moved events */
	void OnTouchMoved(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handle touch ended events */
	void OnTouchEnded(ETouchIndex::Type FingerIndex, FVector Location);

	/** Toggle game phase (for testing) */
	void ToggleGamePhase();

protected:
	//-------------------------------------------
	// Drag Detection State
	//-------------------------------------------

	/** Whether we're currently tracking for potential drag */
	UPROPERTY()
	bool bIsDragTracking;

	/** Initial mouse position when tracking started */
	UPROPERTY()
	FVector2D InitialMousePosition;

	/** Current drag operation if active */
	UPROPERTY()
	UGenericDragDropOperation* CurrentDragOperation;

	//-------------------------------------------
	// Drag&Drop Lifecycle
	//-------------------------------------------

	/**
	 * Start drag detection on mouse down
	 * @param MousePosition Screen position where mouse was pressed
	 */
	void StartDragTracking(const FVector2D& MousePosition);

	/**
	 * Check if mouse has moved far enough to start drag operation
	 * @param CurrentMousePosition Current screen position
	 * @return True if drag should be initiated
	 */
	bool ShouldStartDrag(const FVector2D& CurrentMousePosition) const;

	/**
	 * Initiate empty space drag operation for movement waypoint
	 * @param StartPosition Initial screen position where drag began
	 */
	void StartMovementDrag(const FVector2D& StartPosition);

	/**
	 * Stop current drag tracking without starting drag operation
	 */
	void StopDragTracking();

	/**
	 * Cancel current drag operation if active
	 */
	void CancelCurrentDrag();

	//-------------------------------------------
	// Utility Methods
	//-------------------------------------------

	/**
	 * Get hit result under cursor at screen position
	 * @param HitResult Output hit result
	 * @param ScreenPosition Screen position to trace from (optional, uses cursor if not provided)
	 * @return True if valid hit was found
	 */
	bool GetHitResultUnderCursor(FHitResult& HitResult, const FVector2D* ScreenPosition = nullptr) const;

	/**
	 * Legacy click processing method for backward compatibility
	 * @param ItemId Item identifier for waypoint
	 */
	void ProcessDrop(FString ItemId);

	/**
	 * Process touch click for waypoint placement
	 * @param TouchWorldLocation World location where touch occurred
	 */
	void ProcessTouchClick(const FVector& TouchWorldLocation);

	//-------------------------------------------
	// Component References
	//-------------------------------------------

	/** Cached reference to game mode */
	UPROPERTY()
	APvPGameMode* GameMode;

	/** Cached reference to player character */
	UPROPERTY()
	ABaseCharacter* PlayerCharacter;
};