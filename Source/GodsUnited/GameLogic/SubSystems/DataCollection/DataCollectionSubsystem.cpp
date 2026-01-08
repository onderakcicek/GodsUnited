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

	if (!IsEditorRuntime())
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
	}

	SessionStarted();
}

void UDataCollectionSubsystem::Deinitialize()
{
	SessionEnded();

	Super::Deinitialize();
}

void UDataCollectionSubsystem::SessionStarted()
{
	if (!bEnabled) return;

	if (!IsEditorRuntime())
	{
		UGameAnalytics* GameAnalytics = FGameAnalyticsModule::Get().GetInstance();
		GameAnalytics->StartSession();
	}
	else
	{
		UE_LOG(LogDataCollection, Log, TEXT("session:start"));
	}
}

void UDataCollectionSubsystem::SessionEnded()
{
	if (!bEnabled) return;

	if (!IsEditorRuntime())
	{
		UGameAnalytics* GameAnalytics = FGameAnalyticsModule::Get().GetInstance();
		GameAnalytics->EndSession();
	}
	else
	{
		UE_LOG(LogDataCollection, Log, TEXT("session:end"));
	}
}

void UDataCollectionSubsystem::MatchStarted(EMatchConnectivity Connectivity)
{
	if (!bEnabled) return;

	SendDesignEvent(
		FString::Printf(
			TEXT("match:start:%s"),
			*DataCollection::EnumToToken(Connectivity)
		)
	);
}

void UDataCollectionSubsystem::MatchEnded(EMatchConnectivity Connectivity, EMatchResult Result, float Duration)
{
	if (!bEnabled) return;

	SendDesignEventValue(
		FString::Printf(
			TEXT("match:end:%s:%s"),
			*DataCollection::EnumToToken(Connectivity),
			*DataCollection::EnumToToken(Result)
		),
		Duration
	);
}

void UDataCollectionSubsystem::MatchTerminated(
	EMatchConnectivity Connectivity, EMatchTerminateReason Reason, float Duration
)
{
	if (!bEnabled) return;

	SendDesignEventValue(
		FString::Printf(
			TEXT("match:end:%s:%s:%s"),
			*DataCollection::EnumToToken(Connectivity),
			*DataCollection::EnumToToken(EMatchResult::Terminated),
			*DataCollection::EnumToToken(Reason)
		),
		Duration
	);
}

void UDataCollectionSubsystem::PostMatchAction(
	EPostMatchAction Action, EMatchConnectivity Connectivity, EMatchResult MatchResult, float DecisionTimeSeconds
)
{
	SendDesignEventValue(
		FString::Printf(
			TEXT("postmatch:%s:%s:%s"),
			*DataCollection::EnumToToken(Action),
			*DataCollection::EnumToToken(MatchResult),
			*DataCollection::EnumToToken(Connectivity)
		),
		DecisionTimeSeconds
	);
}

void UDataCollectionSubsystem::SendDesignEvent(const FString& EventName)
{
	if (!IsEditorRuntime())
	{
		UGameAnalytics* GameAnalytics = FGameAnalyticsModule::Get().GetInstance();
		GameAnalytics->AddDesignEvent(EventName);
	}
	else
	{
		UE_LOG(LogDataCollection, Log, TEXT("%s"), *EventName);
	}
}

void UDataCollectionSubsystem::SendDesignEventValue(const FString& EventName, float Value)
{
	if (!IsEditorRuntime())
	{
		UGameAnalytics* GameAnalytics = FGameAnalyticsModule::Get().GetInstance();
		GameAnalytics->AddDesignEventWithValue(EventName, Value);
	}
	else
	{
		UE_LOG(LogDataCollection, Log, TEXT("%s (value=%.2f)"), *EventName, Value);
	}
}
