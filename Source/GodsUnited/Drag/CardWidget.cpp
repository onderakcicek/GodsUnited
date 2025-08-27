// Fill out your copyright notice in the Description page of Project Settings.


#include "CardWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Input/Reply.h"

FReply UCardWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsDraggable && InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		// Drag detection başlat
		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}
    
	return FReply::Unhandled();
}

void UCardWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	// Custom drag drop operation oluştur
	UCardDragDropOperation* DragOp = NewObject<UCardDragDropOperation>();
	if (UWorld* World = GetWorld())
	{
		DragOp->CachedPC = World->GetFirstPlayerController();
	}
	
	DragOp->DefaultDragVisual = nullptr;
	DragOp->Pivot = EDragPivot::MouseDown;
    
	OutOperation = DragOp;

	DragOp->StartDragOperation(DraggingActorClass, FinalActorClass, InMouseEvent);
}

/*void UCardWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (!bIsDraggable) return;

	// Custom drag drop operation oluştur
	UCardDragDropOperation* DragDropOp = NewObject<UCardDragDropOperation>(this);
    
	// Pivot point ve offset ayarla
	DragDropOp->Pivot = EDragPivot::MouseDown;
	DragDropOp->Offset = FVector2D::ZeroVector;
    
	// Drag visual (opsiyonel - card'ın kendisini göster)
	DragDropOp->DefaultDragVisual = nullptr;

	// Drag operation'ı başlat
	DragDropOp->StartDragOperation(DraggingActorClass, FinalActorClass, InMouseEvent);
    
	OutOperation = DragDropOp;
}*/

bool UCardWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	// Card üzerine drop edildiğinde özel davranış istersek burada implement ederiz
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}