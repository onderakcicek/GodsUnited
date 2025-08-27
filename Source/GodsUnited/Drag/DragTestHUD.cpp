// Fill out your copyright notice in the Description page of Project Settings.


// DragTestHUD.cpp

#include "DragTestHUD.h"
#include "Components/PanelWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanelSlot.h"

void UDragTestHUD::NativeConstruct()
{
	Super::NativeConstruct();
    
	// Default card widget class'ı set et
	if (!CardWidgetClass)
	{
		CardWidgetClass = UCardWidget::StaticClass();
	}
}

void UDragTestHUD::AddCard(TSubclassOf<AActor> DraggingActorClass, TSubclassOf<AActor> FinalActorClass)
{
	if (!CardWidgetClass || !CardContainer) return;

	// Yeni card widget oluştur
	UCardWidget* NewCard = CreateWidget<UCardWidget>(this, CardWidgetClass);
	if (NewCard)
	{
		// Card ayarlarını set et
		NewCard->DraggingActorClass = DraggingActorClass;
		NewCard->FinalActorClass = FinalActorClass;
        
		// Container'a ekle
		if (UPanelSlot* S = CardContainer->AddChild(NewCard))
		{
			if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(S))
			{
				CS->SetAutoSize(false);
				// Sol-alt köşeye sabitle
				CS->SetAnchors(FAnchors(0.f, 1.f, 0.f, 1.f)); // min=(0,1) max=(0,1)
				CS->SetAlignment(FVector2D(0.f, 1.f));        // sol-alt referans
				CS->SetPosition(FVector2D(16.f, -16.f));      // sol-alttan 16px içeri (Y negatif = yukarı)
				CS->SetSize(FVector2D(240.f, 360.f));         // biraz büyük
				CS->SetZOrder(10);
			}
		}
		ActiveCards.Add(NewCard);
	}
}

void UDragTestHUD::ClearAllCards()
{
	if (!CardContainer) return;

	CardContainer->ClearChildren();
	ActiveCards.Empty();
}

bool UDragTestHUD::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	// HUD üzerine drop edildiğinde özel davranış
	if (UDragDropOperation* DragDropOp = Cast<UDragDropOperation>(InOperation))
	{
		// Drop işlemi HUD tarafından handle ediliyor
		return true;
	}
    
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}