#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/HitResult.h"
#include "WaypointMovementComponent.generated.h"

class AWaypoint;
class ABaseCharacter;
class APvPGameMode;

// Delegate for movement completion events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMovementCompleted, ABaseCharacter*, Character);

/**
 * Component responsible for waypoint-based movement system
 * Handles path planning, execution, and energy management
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GODSUNITED_API UWaypointMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWaypointMovementComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	//-------------------------------------------
	// Path Management
	//-------------------------------------------

	/**
	 * Handle mouse click to place waypoints during Preparation phase
	 * @param HitResult World location where click occurred
	 * @param ItemId Optional item to attach to waypoint
	 */
	UFUNCTION(BlueprintCallable, Category = "Waypoint Movement")
	void HandleMouseClick(FHitResult HitResult, const FString& ItemId = TEXT(""));

	/**
	 * Create a waypoint actor at specified location
	 * @param Location World position for waypoint
	 * @param Item Optional item identifier
	 * @return Created waypoint actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Waypoint Movement")
	AWaypoint* CreateWaypoint(FVector Location, const FString& Item = TEXT(""));

	/**
	 * Add waypoint to the movement path
	 * @param Waypoint Waypoint to add to path
	 */
	UFUNCTION(BlueprintCallable, Category = "Waypoint Movement")
	void AddWaypointToPath(AWaypoint* Waypoint);

	/**
	 * Clear current path and reset movement state
	 */
	UFUNCTION(BlueprintCallable, Category = "Waypoint Movement")
	void ResetPath();

	/**
	 * Initialize starting waypoint at character's current position
	 * Called at the beginning of preparation phase
	 */
	UFUNCTION(BlueprintCallable, Category = "Waypoint Movement")
	void InitializeStartingWaypoint();

	/**
	 * Get the last waypoint in the current path
	 * @return Last waypoint or nullptr if path is empty
	 */
	UFUNCTION(BlueprintPure, Category = "Waypoint Movement")
	AWaypoint* GetLastWaypoint() const;

	/**
	 * Get the location of the last movement waypoint
	 * @return World location of last waypoint or character location if no path exists
	 */
	UFUNCTION(BlueprintPure, Category = "Waypoint Movement")
	FVector GetLastMoveLocation() const;

	/**
	 * Cancel the last waypoint in the path and refund its energy cost
	 * Cannot cancel the starting waypoint (minimum 1 waypoint guarantee)
	 * @return True if waypoint was successfully cancelled
	 */
	UFUNCTION(BlueprintCallable, Category = "Waypoint Movement")
	bool CancelLastWaypoint();

	/**
	 * Check if last waypoint can be cancelled
	 * Starting waypoint cannot be cancelled
	 * @return True if cancel operation is valid
	 */
	UFUNCTION(BlueprintPure, Category = "Waypoint Movement")
	bool CanCancelLastWaypoint() const;

	//-------------------------------------------
	// Movement Execution
	//-------------------------------------------

	/**
	 * Begin following the waypoint path
	 */
	UFUNCTION(BlueprintCallable, Category = "Waypoint Movement")
	void StartFollowingPath();

	/**
	 * Stop following path and halt movement
	 */
	UFUNCTION(BlueprintCallable, Category = "Waypoint Movement")
	void StopFollowingPath();

	/**
	 * Check if character is currently following a path
	 */
	UFUNCTION(BlueprintPure, Category = "Waypoint Movement")
	bool IsFollowingPath() const { return bIsFollowingPath; }

	/**
	 * Get current waypoint index in the path
	 */
	UFUNCTION(BlueprintPure, Category = "Waypoint Movement")
	int32 GetCurrentWaypointIndex() const { return CurrentWaypointIndex; }

	//-------------------------------------------
	// Energy System
	//-------------------------------------------

	/**
	 * Spend energy for movement, using bonus energy first
	 * @param MoveCost Amount of energy to spend
	 */
	UFUNCTION(BlueprintCallable, Category = "Energy System")
	void SpendEnergyToMove(int32 MoveCost);

	/**
	 * Check if a movement-only waypoint can be placed at target location
	 * Movement waypoints cannot be placed within placement radius of last waypoint
	 * @param TargetLocation World position where waypoint would be placed
	 * @return True if movement waypoint can be placed
	 */
	UFUNCTION(BlueprintPure, Category = "Waypoint Validation")
	bool CanPlaceMovementWaypoint(const FVector& TargetLocation) const;

	/**
	 * Check if a card waypoint can be placed at target location
	 * Card waypoints can always be placed if energy is sufficient
	 * @param TargetLocation World position where waypoint would be placed
	 * @param OutEnergyCost Energy cost for placement (in rings)
	 * @return True if card waypoint can be placed
	 */
	UFUNCTION(BlueprintPure, Category = "Waypoint Validation")
	bool CanPlaceCardWaypoint(const FVector& TargetLocation, int32& OutEnergyCost) const;

	/**
	 * Calculate total available energy including buffer energy with multiplier
	 * @return Total usable energy amount
	 */
	UFUNCTION(BlueprintPure, Category = "Energy System")
	int32 GetAvailableEnergy() const;

	/**
	 * Calculate distance from target location to last waypoint position
	 * @param TargetLocation Target position to measure distance to
	 * @return 2D distance to last waypoint
	 */
	UFUNCTION(BlueprintPure, Category = "Waypoint Validation")
	float GetDistanceToLastWaypoint(const FVector& TargetLocation) const;

	/**
	 * Calculate energy cost in rings for placing waypoint at target location
	 * Uses ring-based calculation system from BasePlayerDefinitions
	 * @param TargetLocation World position for waypoint placement
	 * @return Energy cost in rings (0 if within free radius)
	 */
	UFUNCTION(BlueprintPure, Category = "Energy System")
	int32 CalculateWaypointEnergyCost(const FVector& TargetLocation) const;

	/**
	 * Check if player has sufficient energy for a given cost
	 * @param RequiredEnergy Energy amount needed
	 * @return True if player can afford the energy cost
	 */
	UFUNCTION(BlueprintPure, Category = "Energy System")
	bool HasSufficientEnergy(int32 RequiredEnergy) const;
	

	//-------------------------------------------
	// Events
	//-------------------------------------------

	/**
	 * Broadcast when movement along path is completed
	 */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMovementCompleted OnMovementCompleted;

protected:
	//-------------------------------------------
	// Internal Movement Logic
	//-------------------------------------------

	/**
	 * Main movement processing logic called each frame
	 */
	void ProcessMovement();

	/**
	 * Process waypoint sequence and handle U-turn detection
	 */
	void ProcessWaypointSequence();

	/**
	 * Detect and handle U-turn situations for smooth movement
	 */
	void HandleUTurnDetection();

	/**
	 * Provide visual feedback when waypoint placement is invalid
	 * @param Location World position where invalid placement was attempted
	 * @param Reason String describing why placement failed
	 */
	void ProvideInvalidPlacementFeedback(const FVector& Location, const FString& Reason);

	/**
	 * Provide visual feedback when waypoint placement is successful
	 * @param Location World position where waypoint was placed
	 * @param PlacementType Type of waypoint placed (Movement/Card)
	 * @param EnergyCost Energy cost for the placement
	 */
	void ProvideValidPlacementFeedback(const FVector& Location, const FString& PlacementType, int32 EnergyCost);

	/**
	 * Move character toward current waypoint with turn slowdown
	 */
	void MoveToCurrentWaypoint();

	/**
	 * Check if character has reached current waypoint
	 */
	bool HasReachedCurrentWaypoint() const;

	/**
	 * Advance to next waypoint in sequence
	 */
	void MoveToNextWaypoint();

	/**
	 * Handle logic when a waypoint is reached
	 */
	void OnWaypointReached();

	/**
	 * Trigger action associated with current waypoint item
	 */
	void TriggerItemAction();

	/**
	 * Render debug visualization for path and movement
	 */
	void RenderDebugVisualization();

	//-------------------------------------------
	// Path Data
	//-------------------------------------------

	/** Array of waypoints defining the movement path */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path")
	TArray<AWaypoint*> Path;

	/** Current waypoint index in the path */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path")
	int32 CurrentWaypointIndex;

	/** Whether character is currently following the path */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path")
	bool bIsFollowingPath;

	/** Whether component is currently accepting new waypoints */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool bAcceptingWaypoints;

	//-------------------------------------------
	// Movement Settings
	//-------------------------------------------

	/** Distance tolerance for reaching waypoints */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MovementTolerance;

	//-------------------------------------------
	// Component References
	//-------------------------------------------

	/** Cached reference to owner character */
	UPROPERTY()
	ABaseCharacter* OwnerCharacter;

	/** Cached reference to game mode */
	UPROPERTY()
	APvPGameMode* GameMode;

private:
	//-------------------------------------------
	// Internal Helpers
	//-------------------------------------------

	/**
	 * Cache component references for performance
	 */
	void CacheReferences();

	/**
	 * Validate that component is properly initialized
	 */
	bool IsValidForMovement() const;

	/**
	 * Refund energy to player after waypoint cancellation
	 * @param EnergyAmount Amount of energy to refund
	 */
	void RefundEnergy(int32 EnergyAmount);

	/**
	 * Update path indices for all waypoints after path modification
	 */
	void UpdateWaypointIndices();

	/**
	 * Provide visual feedback for waypoint cancellation
	 * @param CancelledLocation Location where waypoint was cancelled
	 * @param EnergyRefunded Amount of energy that was refunded
	 * @param bHadItem Whether cancelled waypoint had an item
	 * @param ItemId ID of the cancelled item
	 */
	void ProvideCancelFeedback(const FVector& CancelledLocation, int32 EnergyRefunded, bool bHadItem, const FString& ItemId);
};
