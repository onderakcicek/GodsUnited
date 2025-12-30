// Fill out your copyright notice in the Description page of Project Settings.

#include "DataCollectionSubsystem.h"

#include "GameAnalytics.h"
#include "GameAnalyticsModule.h"

bool UDataCollectionSubsystem::IsEditorRuntime() const
{
#if WITH_EDITOR
	return true;
#else
	return false;
#endif
}

void UDataCollectionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	if (!IsEditorRuntime() && bEnabled)
	{
		UGameAnalytics* GameAnalytics = FGameAnalyticsModule::Get().GetInstance();
		GameAnalytics->Initialize("3e8b6f26bcd6d4d3df004d6656838cbe", "d94836251566d6b6313e6f315fae16e4abe6c834");
		SessionStarted();
	}
}

void UDataCollectionSubsystem::Deinitialize()
{
	if (!IsEditorRuntime() && bEnabled)
	{
		UGameAnalytics* GameAnalytics = FGameAnalyticsModule::Get().GetInstance();
		GameAnalytics->EndSession();
	}

	Super::Deinitialize();
}

void UDataCollectionSubsystem::SessionStarted()
{
	UGameAnalytics* GameAnalytics = FGameAnalyticsModule::Get().GetInstance();
	GameAnalytics->StartSession();
}

void UDataCollectionSubsystem::SessionEnded()
{
}

void UDataCollectionSubsystem::MatchStarted()
{
}

void UDataCollectionSubsystem::MatchEnded(bool bWin)
{
}

void UDataCollectionSubsystem::RematchClicked()
{
}

void UDataCollectionSubsystem::ReturnToLoadOutClicked()
{
}

void UDataCollectionSubsystem::SendEvent(const FString& EventName)
{
	if (bEnabled)
	{
		//UGameAnalytics::AddDesignEvent(EventName);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[DATA MOCK] %s"), *EventName);
		//UE_LOG(LogTemp, Log, TEXT("[DATA MOCK] %s = %f"), *EventName, Value);
	}
}
