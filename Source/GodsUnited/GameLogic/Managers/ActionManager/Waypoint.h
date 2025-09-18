// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Waypoint.generated.h"

// Forward declarations
class UStaticMeshComponent;
class UTextRenderComponent;
class USphereComponent;
class IItemInterface;

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
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Visual representation of the waypoint
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* WaypointMesh;
    
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

	UFUNCTION(BlueprintCallable)
	FString GetItem();
	//TScriptInterface<IItemInterface> GetItem();
	
	UFUNCTION(BlueprintCallable)
	void SetItem(FString ItemToSet);
	//void SetItem(TScriptInterface<IItemInterface> ItemToSet);

	UFUNCTION(BlueprintCallable)
	bool HasItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Last known path index (for tracking changes)
	UPROPERTY()
	int32 LastKnownPathIndex;

private:
	// Whether this waypoint was created with an item (triggers special action)
	UPROPERTY(EditAnywhere, Category = "Waypoint")
	FString Item;
	//TScriptInterface<IItemInterface> Item;
};