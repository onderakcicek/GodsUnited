// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CardDragDropOperation.h"
#include "CardWidget.generated.h"

/**
 * 
 */
UCLASS()
class GODSUNITED_API UCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Drag edilecek actor class'ları
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Settings")
	TSubclassOf<AActor> DraggingActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Settings")
	TSubclassOf<AActor> FinalActorClass;

	// Drag edilebilir mi?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Settings")
	bool bIsDraggable = true;

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

};
