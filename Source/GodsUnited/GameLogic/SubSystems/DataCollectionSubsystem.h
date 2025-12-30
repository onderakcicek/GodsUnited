// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

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

	/* ---------- Session ---------- */
	UFUNCTION(BlueprintCallable, Category="Data Collection|Session")
	void SessionStarted();

	UFUNCTION(BlueprintCallable, Category="Data Collection|Session")
	void SessionEnded();

	/* ---------- Match ---------- */

	UFUNCTION(BlueprintCallable, Category="Data Collection|Match")
	void MatchStarted();

	UFUNCTION(BlueprintCallable, Category="Data Collection|Match")
	void MatchEnded(bool bWin);

	/* ---------- UI ---------- */

	UFUNCTION(BlueprintCallable, Category="Data Collection|UI")
	void RematchClicked();

	UFUNCTION(BlueprintCallable, Category="Data Collection|UI")
	void ReturnToLoadOutClicked();

private:
	bool bEnabled = true;

	bool IsEditorRuntime() const;
	
	void SendEvent(const FString& EventName);
};
