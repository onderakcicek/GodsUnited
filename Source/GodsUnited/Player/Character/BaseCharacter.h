#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/HitResult.h"
#include "BaseCharacter.generated.h"

class AWaypoint;
class APvPGameMode;
class UWaypointMovementComponent;

/**
 * Base character class for player-controlled units
 * Delegates movement logic to WaypointMovementComponent
 */
UCLASS()
class GODSUNITED_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABaseCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	//-------------------------------------------
	// Waypoint System (Delegated to Component)
	//-------------------------------------------
	
	/**
	 * Get the last waypoint in the current path
	 * @return Last waypoint or nullptr if path is empty
	 */
	UFUNCTION(BlueprintPure, Category = "Waypoint Movement")
	AWaypoint* GetLastWaypoint();

	/**
	 * Get the location of the last movement waypoint
	 * @return World location of last waypoint or character location if no path exists
	 */
	UFUNCTION(BlueprintPure, Category = "Waypoint Movement")
	FVector GetLastMoveLocation();

	/**
	 * Begin following the waypoint path
	 */
	UFUNCTION(BlueprintCallable, Category = "Waypoint Movement")
	void StartFollowingPath();

	/**
	 * Clear current path and reset movement state
	 */
	UFUNCTION(BlueprintCallable, Category = "Waypoint Movement")
	void ResetPath();

	/**
	 * Initialize starting waypoint at character's current position
	 */
	UFUNCTION(BlueprintCallable, Category = "Waypoint Movement")
	void InitializeStartingWaypoint();

	/**
	 * Create a waypoint actor at specified location
	 * @param Location World position for waypoint
	 * @param Item Optional item identifier
	 * @return Created waypoint actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Waypoint Movement")
	AWaypoint* CreateWaypoint(FVector Location, FString Item = TEXT(""));

	/**
	 * Add waypoint to the movement path
	 * @param Waypoint Waypoint to add to path
	 */
	UFUNCTION(BlueprintCallable, Category = "Waypoint Movement")
	void AddWaypointToPath(AWaypoint* Waypoint);

	/**
	 * Check if character is currently following a path
	 */
	UFUNCTION(BlueprintPure, Category = "Waypoint Movement")
	bool IsFollowingPath() const;

	/**
	 * Get current waypoint index in the path
	 */
	UFUNCTION(BlueprintPure, Category = "Waypoint Movement")
	int32 GetCurrentWaypointIndex() const;

	//-------------------------------------------
	// Input & Interaction
	//-------------------------------------------
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/**
	 * Handle mouse clicks to place waypoints in Preparation phase
	 * @param HitResult World location where click occurred
	 * @param ItemId Optional item to attach to waypoint
	 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void OnMouseClick(FHitResult HitResult, FString ItemId = TEXT(""));

	//-------------------------------------------
	// Action System
	//-------------------------------------------
	
	/**
	 * Trigger action associated with an item
	 * Override in subclasses for custom behavior
	 * @param ItemId Identifier of the item to trigger
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Actions")
	void TriggerAction(const FString& ItemId);
	virtual void TriggerAction_Implementation(const FString& ItemId);

	//-------------------------------------------
	// Energy System
	//-------------------------------------------
	
	/**
	 * Calculate movement cost from current location to target location
	 * @param InputLocation Target location for movement
	 * @return Energy cost for movement
	 */
	UFUNCTION(BlueprintPure, Category = "Energy System")
	int32 CalculateMovementCost(const FVector& InputLocation);

	/**
	 * Spend energy for movement, using bonus energy first
	 * @param MoveCost Amount of energy to spend
	 */
	UFUNCTION(BlueprintCallable, Category = "Energy System")
	void SpendEnergyToMove(const int32 MoveCost);

	//-------------------------------------------
	// Component Access
	//-------------------------------------------
	
	/**
	 * Get the waypoint movement component
	 * @return WaypointMovementComponent reference
	 */
	UFUNCTION(BlueprintPure, Category = "Components")
	UWaypointMovementComponent* GetWaypointMovementComponent() const;

	//-------------------------------------------
	// Events
	//-------------------------------------------
	
	/**
	 * Called when movement along path is completed
	 * Can be overridden in Blueprint for custom behavior
	 * @param Character The character that completed movement
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Events")
	void OnMovementCompletedEvent(ABaseCharacter* Character);
	virtual void OnMovementCompletedEvent_Implementation(ABaseCharacter* Character);

protected:
	//-------------------------------------------
	// Event Handlers
	//-------------------------------------------
	
	/**
	 * Handle movement completion from component
	 * @param Character The character that completed movement
	 */
	UFUNCTION()
	void OnMovementCompleted(ABaseCharacter* Character);

public:
	//-------------------------------------------
	// Character Properties
	//-------------------------------------------
	
	/** Current energy available for movement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy")
	int32 Energy;

	/** Bonus energy that provides 2x conversion rate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy")
	int32 BufferEnergy;

	/** Z-offset for ground positioning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float PlayerGroundOffset;

protected:
	//-------------------------------------------
	// Components
	//-------------------------------------------
	
	/** Component responsible for waypoint-based movement */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWaypointMovementComponent* WaypointMovementComponent;
};