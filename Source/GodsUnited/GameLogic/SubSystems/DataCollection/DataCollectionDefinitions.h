#pragma once

#include "UObject/UnrealType.h"

#include "DataCollectionDefinitions.generated.h"


GODSUNITED_API DECLARE_LOG_CATEGORY_EXTERN(LogDataCollection, Log, All);

namespace DataCollection
{
	template <typename TEnum>
	FORCEINLINE FString EnumToToken(TEnum Value)
	{
		static_assert(TIsEnum<TEnum>::Value, "EnumToToken can only be used with enum types");

		const UEnum* Enum = StaticEnum<TEnum>();
		check(Enum);

		return Enum
			->GetNameStringByValue(static_cast<int64>(Value))
			.ToLower();
	}
}

UENUM(BlueprintType)
enum class EMatchConnectivity : uint8
{
	Online,
	Offline,
};

UENUM(BlueprintType, meta = (ScriptName = "EMatchResult1"))
enum EMatchResult
{
	Win,
	Lose,
	Terminated,
};

UENUM(BlueprintType, meta = (ScriptName = "EPostMatchAction1"))
enum EPostMatchAction
{
	Rematch UMETA(DisplayName = "Rematch"),
	ReturnToLoadout UMETA(DisplayName = "Return To Loadout"),
};

UENUM(BlueprintType, meta = (ScriptName = "EMatchTerminateReason1"))
enum EMatchTerminateReason
{
	Quit,
	NetworkTimedOut,
	Disconnected,
};
