
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GenericDragDropOperation.h"  // Changed from CardDragDropOperation
#include "CardWidget.generated.h"

/**
 * Card widget that supports drag and drop operations
 */
UCLASS()
class GODSUNITED_API UCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Actor classes for drag and drop
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Settings")
	TSubclassOf<AActor> DraggingActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Settings")
	TSubclassOf<AActor> FinalActorClass;

	// Card item identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Settings")
	FString CardItemId;

	// Whether this card can be dragged
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Settings")
	bool bIsDraggable = true;

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};