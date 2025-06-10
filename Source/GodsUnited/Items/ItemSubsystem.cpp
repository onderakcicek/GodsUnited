// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemSubsystem.h"
#include "ItemDefinitions.h"


void UItemSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogItemSubsystem, Log, TEXT("InventorySubsystem initialized"));

	// Load the data table into memory
	FSoftObjectPath const Path = Item::DataTablePath;
	UDataTable* DataTable = Cast<UDataTable>(Path.TryLoad());

	if (!DataTable)
	{
		UE_LOG(LogItemSubsystem, Error, TEXT("Failed to locate the regular mission templates data table."));
		return;
	}
}

void UItemSubsystem::Deinitialize()
{
	UE_LOG(LogItemSubsystem, Log, TEXT("InventorySubsystem deinitialized"));
	Super::Deinitialize();
}

/*bool UItemSubsystem::AddItem(UItemData* Item)
{
	if (!Item) return false;
	Items.Add(Item);
	OnInventoryChanged.Broadcast();
	return true;
}

bool UItemSubsystem::RemoveItem(UItemData* Item)
{
	if (!Item) return false;
	const int32 Removed = Items.Remove(Item);
	if (Removed > 0) OnInventoryChanged.Broadcast();
	return Removed > 0;
}*/