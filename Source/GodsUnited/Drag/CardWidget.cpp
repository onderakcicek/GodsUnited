// CardWidget.cpp - Update implementation

#include "CardWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GodsUnited/Player/Character/BaseCharacter.h"
#include "Input/Reply.h"

FReply UCardWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsDraggable && InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		// Start drag detection
		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}
    
	return FReply::Unhandled();
}

void UCardWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	if (!bIsDraggable) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Card is not draggable"));
		return;
	}

	// Validate required classes
	if (!DraggingActorClass || !FinalActorClass)
	{
		UE_LOG(LogTemp, Error, TEXT("DraggingActorClass or FinalActorClass is null"));
		return;
	}

	// Validate card item ID
	if (CardItemId.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("CardItemId is empty - this will be treated as movement drag"));
	}

	// Create generic drag drop operation
	UGenericDragDropOperation* DragOp = NewObject<UGenericDragDropOperation>();
	if (!DragOp)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create GenericDragDropOperation"));
		return;
	}

	// Cache player controller and character references
	if (UWorld* World = GetWorld())
	{
		DragOp->CachedPC = World->GetFirstPlayerController();
		if (!DragOp->CachedPC.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to get PlayerController reference"));
		}
		else
		{
			// Set PlayerCharacter reference
			DragOp->PlayerCharacter = Cast<ABaseCharacter>(DragOp->CachedPC->GetPawn());
			if (!DragOp->PlayerCharacter)
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to get PlayerCharacter reference"));
			}
		}
	}
	
	// Configure drag operation properties
	DragOp->DefaultDragVisual = nullptr;
	DragOp->Pivot = EDragPivot::MouseDown;
    
	OutOperation = DragOp;

	// Start the drag operation with card-specific parameters
	DragOp->StartDragOperation(DraggingActorClass, FinalActorClass, CardItemId, InMouseEvent);
	
	UE_LOG(LogTemp, Display, TEXT("Card drag operation started successfully for item: %s"), *CardItemId);
}

bool UCardWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	// Handle special behavior when dropped on another card if needed
	if (UGenericDragDropOperation* GenericDragOp = Cast<UGenericDragDropOperation>(InOperation))
	{
		UE_LOG(LogTemp, Display, TEXT("Card dropped on another card"));
		// Custom logic for card-to-card interaction can be added here
		return true;
	}
	
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}