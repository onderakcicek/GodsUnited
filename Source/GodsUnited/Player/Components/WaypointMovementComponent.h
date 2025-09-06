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
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

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
	 * Calculate movement cost from current location to target location
	 * @param InputLocation Target location for movement
	 * @return Energy cost for movement
	 */
	UFUNCTION(BlueprintPure, Category = "Energy System")
	float CalculateMovementCost(const FVector& InputLocation) const;

	/**
	 * Spend energy for movement, using bonus energy first
	 * @param MoveCost Amount of energy to spend
	 */
	UFUNCTION(BlueprintCallable, Category = "Energy System")
	void SpendEnergyToMove(float MoveCost);

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

protected:
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
};