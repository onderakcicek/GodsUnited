// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

// Forward declarations
class APvPGameMode;
class IItemInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMovementCompletedDelegate, ABaseCharacter*, Character);

namespace PlayerCharacter
{
	constexpr float EnergyIncrementPerRound = 3;
	constexpr float MaxEnergy = 10;
}

UCLASS(Blueprintable, BlueprintType)
class GODSUNITED_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float PlayerGroundOffset;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Reference to the game mode
	UPROPERTY()
	APvPGameMode* GameMode;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called when player clicks in the world
	UFUNCTION(BlueprintCallable, Category = "Input")
	void OnMouseClick(FHitResult HitResult, FString ItemId);

	//-----------------------------------------------------------------------
	// WAYPOINT SYSTEM
	//-----------------------------------------------------------------------
	
	// Create a waypoint at the specified location
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	//class AWaypoint* CreateWaypoint(FVector Location, TScriptInterface<IItemInterface> Item);
	class AWaypoint* CreateWaypoint(FVector Location, FString Item);

	// Start following the created path
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void StartFollowingPath();

	// Reset the character's path
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void ResetPath();

	// Function to be called when passing through a right-click waypoint
	UFUNCTION(BlueprintNativeEvent, Category = "Actions")
	void TriggerAction(const FString& ItemId);
	virtual void TriggerAction_Implementation(const FString& ItemId);

	// C++ implementation of the function to be called at right-click waypoints
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void TriggerItemAction();

	// Add a waypoint to the character's path
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void AddWaypointToPath(class AWaypoint* Waypoint);

	// Get all waypoints in the character's path
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	const TArray<class AWaypoint*>& GetPath() const { return Path; }

	// Movement tolerance (how close to get to a waypoint)
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MovementTolerance;

	UPROPERTY(BlueprintAssignable)
	FMovementCompletedDelegate MovementCompletedHandle;
	
	// Check if the character is currently following a path
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	bool IsFollowingPath() const { return bIsFollowingPath; }
	
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	bool HasMovedToFinalWaypoint() const { return CurrentWaypointIndex >= Path.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Navigation")
	AWaypoint* GetLastWaypoint();

	//-----------------------------------------------------------------------
	// ENERGY SYSTEM
	//-----------------------------------------------------------------------

	void SetEnergy(float EnergyToSet);
	void SetGenerativeEnergy(float EnergyToSet);

	
private:
	//-----------------------------------------------------------------------
	// WAYPOINT SYSTEM
	//-----------------------------------------------------------------------
	
	// Array of waypoints that form the character's path
	UPROPERTY()
	TArray<class AWaypoint*> Path;

	void ProcessMovement();

	// Current waypoint target index
	UPROPERTY()
	int32 CurrentWaypointIndex;

	// Whether the character is currently following the path
	UPROPERTY()
	bool bIsFollowingPath;

	// Move towards the current waypoint
	void MoveToCurrentWaypoint();

	// Check if the character has reached the current waypoint
	bool HasReachedCurrentWaypoint() const;

	// Move to the next waypoint in the path
	void MoveToNextWaypoint();
	void OnWaypointReached();

	//-----------------------------------------------------------------------
	// ENERGY SYSTEM
	//-----------------------------------------------------------------------
	float Energy = 3;
	float GenerativeEnergy = 0;

	// Calculate movement cost based on action queue
	float CalculateMovementCost(const FVector& InputLocation);

	void SpendEnergyToMove(float MoveCost);
	
};
