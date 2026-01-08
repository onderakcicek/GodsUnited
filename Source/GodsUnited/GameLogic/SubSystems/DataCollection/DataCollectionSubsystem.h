// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DataCollectionDefinitions.h"

#include "DataCollectionSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class GODSUNITED_API UDataCollectionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/* ---------- EVENTS ---------- */
	
	/* ---------- Session ---------- */
	UFUNCTION(BlueprintCallable, Category="Data Collection|Session")
	void SessionStarted();

	UFUNCTION(BlueprintCallable, Category="Data Collection|Session")
	void SessionEnded();

	/* ---------- Match ---------- */

	UFUNCTION(BlueprintCallable, Category="Data Collection|Match")
	void MatchStarted(EMatchConnectivity Connectivity);

	UFUNCTION(BlueprintCallable, Category="Data Collection|Match")
	void MatchEnded(EMatchConnectivity Connectivity, EMatchResult Result, float Duration = 0.f);
	
	UFUNCTION(BlueprintCallable, Category="Data Collection|Match")
	void MatchTerminated(EMatchConnectivity Connectivity, EMatchTerminateReason Reason, float Duration = 0.f);

	UFUNCTION(BlueprintCallable)
	void PostMatchAction(
		EPostMatchAction Action,
		EMatchConnectivity Connectivity,
		EMatchResult Result,
		float DecisionTimeSeconds
	);

	/* --------------------------------------- */

private:
	bool bEnabled = true;

	bool IsEditorRuntime() const;
	
	void SendDesignEvent(const FString& EventName);

	void SendDesignEventValue(const FString& EventName, float Value);
};
