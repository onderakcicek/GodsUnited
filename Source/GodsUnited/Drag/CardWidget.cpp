// Fill out your copyright notice in the Description page of Project Settings.

#include "CardWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
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

	// Create custom drag drop operation
	UCardDragDropOperation* DragOp = NewObject<UCardDragDropOperation>();
	if (!DragOp)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create CardDragDropOperation"));
		return;
	}

	// Cache player controller reference
	if (UWorld* World = GetWorld())
	{
		DragOp->CachedPC = World->GetFirstPlayerController();
		if (!DragOp->CachedPC.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to get PlayerController reference"));
		}
	}
	
	// Configure drag operation properties
	DragOp->DefaultDragVisual = nullptr;
	DragOp->Pivot = EDragPivot::MouseDown;
    
	OutOperation = DragOp;

	// Start the drag operation with actor classes
	DragOp->StartDragOperation(DraggingActorClass, FinalActorClass, InMouseEvent);
	
	UE_LOG(LogTemp, Display, TEXT("Drag operation started successfully"));
}

bool UCardWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	// Handle special behavior when dropped on another card if needed
	if (UCardDragDropOperation* CardDragOp = Cast<UCardDragDropOperation>(InOperation))
	{
		UE_LOG(LogTemp, Display, TEXT("Card dropped on another card"));
		// Custom logic for card-to-card interaction can be added here
		return true;
	}
	
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}