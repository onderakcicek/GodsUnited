// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Definitions.h"
#include "GameFramework/GameModeBase.h"
#include "PvPGameMode.generated.h"

/**
 * Two-phase game mode that switches between preparation and action
 */
UCLASS()
class GODSUNITED_API APvPGameMode : public AGameModeBase
{
	GENERATED_BODY()
    
public:
	APvPGameMode();

	// Called when the game starts
	virtual void BeginPlay() override;
    
	// Called every frame
	virtual void Tick(float DeltaTime) override;
    
	// Switches from preparation to action phase
	UFUNCTION(BlueprintCallable, Category = "Game")
	void StartActionPhase();
    
	// Switches from action to preparation phase
	UFUNCTION(BlueprintCallable, Category = "Game")
	void StartPreparationPhase();
    
	// Returns the current game phase
	UFUNCTION(BlueprintCallable, Category = "Game")
	EPvPGamePhase GetCurrentPhase() const { return CurrentPhase; }
    
	// Event that gets called when the game phase changes
	UFUNCTION(BlueprintImplementableEvent, Category = "Game")
	void OnGamePhaseChanged(EPvPGamePhase NewPhase);
    
	// Debug display toggle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebugInfo;

private:
	// Current game phase
	UPROPERTY(VisibleAnywhere, Category = "Game")
	EPvPGamePhase CurrentPhase;
};