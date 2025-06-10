#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ItemDefinitions.generated.h"

//////////////////////////////////////////////////////////////////////////////////////
// LOGS
//////////////////////////////////////////////////////////////////////////////////////

GODSUNITED_API DECLARE_LOG_CATEGORY_EXTERN(LogItemSubsystem, Log, All);

#define ITEM_LOG_ERR(MessageFmt, ...) \
UE_LOG(LogItemSubsystem, Error, TEXT("%hs - " MessageFmt), __FUNCTION__, ##__VA_ARGS__)

//////////////////////////////////////////////////////////////////////////////////////
// CONSTANTS
//////////////////////////////////////////////////////////////////////////////////////

namespace Item
{
	const FString DataTablePath = TEXT("/Script/Engine.DataTable'/Game/DataTables/DT_Items.DT_Items'");
}

//////////////////////////////////////////////////////////////////////////////////////
// TYPES
//////////////////////////////////////////////////////////////////////////////////////
UENUM(BlueprintType)
enum class EItemType : uint8
{
	None,
	Gadget,
	Weapon
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	None,
	FlakCannon,
	LightningGun,
	LinkGun,
	RocketLauncher,
	ShockRifle,
	SniperRifle,

	OnePastLast UMETA(Hidden),
	First = None + 1 UMETA(Hidden),
	Last = OnePastLast - 1 UMETA(Hidden),
};

UENUM(BlueprintType)
enum class EGadgetType : uint8
{
	None,
	BlinkDisruptor,
	NanoShield,
	RegenStimulant,
	HyperReflex,
	OverclockModule,

	OnePastLast UMETA(Hidden),
	First = None + 1 UMETA(Hidden),
	Last = OnePastLast - 1 UMETA(Hidden),
};

// Aşağıdaki type'ların tanımlı olması gerektiğini düşünmüyorum. Her item ayrı bir class ile temsil edilecek ve
// her item'ın davranışı spesifik. Eğer "attack type'ı standard olanlardan rastgele birini getir" ya da "projectile"
// silahları ignore et" gibi kurallar belirlenmeyecekse aşağıdakilere gerek yok.

UENUM(BlueprintType)
enum class EWeaponAttackType : uint8
{
	Standard,
	Splash,
	Buff,
	Fortify,
	Piercer,
};

UENUM(BlueprintType)
enum class EWeaponDeliveryType : uint8
{
	Projectile,
	Self,
	HitScan,
};

UENUM(BlueprintType)
enum class EEffectType : uint8
{
	Standard,
	Movement,
	Buff,
};

UENUM(BlueprintType)
enum class EUsageType : uint8
{
	Self,
	Opponent,
};
