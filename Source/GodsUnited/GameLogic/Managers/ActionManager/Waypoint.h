// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Waypoint.generated.h"

// Forward declarations
class UStaticMeshComponent;
class UTextRenderComponent;
class USphereComponent;

/**
 * Waypoint actor used for character path creation
 */
UCLASS()
class GODSUNITED_API AWaypoint : public AActor
{
	GENERATED_BODY()
    
public:    
	// Sets default values for this actor's properties
	AWaypoint();

	// Visual representation of the waypoint
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* WaypointMesh;
    
	// Whether this waypoint was created with right click (triggers special action)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	bool bIsRightClick;
    
	// The index of this waypoint in the path
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	int32 PathIndex;
    
	// Reference to the player who created this waypoint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	class ABaseCharacter* OwningPlayer;

	// Text component to display waypoint index
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UTextRenderComponent* IndexText;
 
	// Debug sphere to better visualize the waypoint
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* CollisionSphere;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Last known path index (for tracking changes)
	UPROPERTY()
	int32 LastKnownPathIndex;
public:    
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};