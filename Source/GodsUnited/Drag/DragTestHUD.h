// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CardWidget.h"
#include "CardDragDropOperation.h"
#include "DragTestHUD.generated.h"

/**
 * 
 */
UCLASS()
class GODSUNITED_API UDragTestHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// Card'ları dinamik olarak ekleme
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void AddCard(TSubclassOf<AActor> DraggingActorClass, TSubclassOf<AActor> FinalActorClass);

	// Tüm card'ları temizle
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ClearAllCards();

protected:
	virtual void NativeConstruct() override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	// Card container panel (Blueprint'te tanımlanmalı)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UPanelWidget* CardContainer;

	// Card widget class'ı
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD Settings")
	TSubclassOf<UCardWidget> CardWidgetClass;

private:
	// Aktif card'ların listesi
	UPROPERTY()
	TArray<UCardWidget*> ActiveCards;
};
