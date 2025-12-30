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
		
		// Sets the userId, must be configured before initialization
		// By default the GameAnalytics SDK will generate a default userId
		// GameAnalytics->ConfigureUserId("myCustomUserId");

		// setting the credentials
		GameAnalytics->Initialize("3e8b6f26bcd6d4d3df004d6656838cbe", "d94836251566d6b6313e6f315fae16e4abe6c834");

		// sets build version
		// GameAnalytics->ConfigureBuild(TEXT("1.0.0"));
		
		// sets build version according to Project Settings -> Project Version
		GameAnalytics->ConfigureAutoDetectAppVersion(true);

		// Sets a secondary identifier that you can use to aggregate and link data sets
		//GameAnalytics->ConfigureExternalUserId("iyo");

		// If the app has the required permissions, GameAnalytics will track the advertising IDs on certain platforms such as Android and iOS.
		// If you wish to disable tracking, for privacy reasons, you can call the following function
		//GameAnalytics->EnableAdvertisingId(false);
		
		SessionStarted();
	}
}

void UDataCollectionSubsystem::Deinitialize()
{
	SessionEnded();

	Super::Deinitialize();
}

void UDataCollectionSubsystem::SessionStarted()
{
	if (!IsEditorRuntime() && bEnabled)
	{
		UGameAnalytics* GameAnalytics = FGameAnalyticsModule::Get().GetInstance();
		GameAnalytics->StartSession();	
	}
}

void UDataCollectionSubsystem::SessionEnded()
{
	if (!IsEditorRuntime() && bEnabled)
	{
		UGameAnalytics* GameAnalytics = FGameAnalyticsModule::Get().GetInstance();
		GameAnalytics->EndSession();
	}
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
