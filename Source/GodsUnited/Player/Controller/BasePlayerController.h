// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BasePlayerController.generated.h"

class APvPGameMode;
class ABaseCharacter;

/**
 * 
 */
UCLASS()
class GODSUNITED_API ABasePlayerController : public APlayerController
{
	GENERATED_BODY()
    
public:
	ABasePlayerController();
    
	// Called when the game starts
	virtual void BeginPlay() override;
    
	// Called every frame
	virtual void Tick(float DeltaTime) override;
    
	// Handle left mouse button click
	UFUNCTION(BlueprintCallable, Category = "Input")
	void OnLeftMouseClick();
    
	// Handle right mouse button click
	UFUNCTION(BlueprintCallable, Category = "Input")
	void OnRightMouseClick();
    
	// Toggle between preparation and action phases
	UFUNCTION(BlueprintCallable, Category = "Game")
	void ToggleGamePhase();
    
	// Set up input bindings
	virtual void SetupInputComponent() override;
    
private:
	// Reference to the game mode
	UPROPERTY()
	APvPGameMode* GameMode;
    
	// Reference to the player character
	UPROPERTY()
	ABaseCharacter* PlayerCharacter;
    
	// Perform a trace under the mouse cursor
	bool GetHitResultUnderCursor(FHitResult& HitResult) const;
    
	// Process mouse click and forward to character
	void ProcessDrop(FString ItemId);
};
