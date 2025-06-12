// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlayerController.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

#include "GodsUnited/GameLogic/GameModes/PvPGameMode/Definitions.h"
#include "GodsUnited/GameLogic/GameModes/PvPGameMode/PvPGameMode.h"
#include "GodsUnited/Player/Character/BaseCharacter.h"

ABasePlayerController::ABasePlayerController()
{
    // Enable mouse cursor by default
    bShowMouseCursor = true;
    DefaultMouseCursor = EMouseCursor::Default;
}

void ABasePlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    // Get reference to the game mode
    GameMode = Cast<APvPGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    
    // Get reference to the player character
    PlayerCharacter = Cast<ABaseCharacter>(GetPawn());
    
    if (!PlayerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerController couldn't find a valid PlayerCharacter"));
    }
}

void ABasePlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Additional per-frame logic can go here
}

void ABasePlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // Bind mouse click events
    /*InputComponent->BindAction("LeftClick", IE_Pressed, this, &ABasePlayerController::OnLeftMouseClick);
    InputComponent->BindAction("RightClick", IE_Pressed, this, &ABasePlayerController::OnRightMouseClick);
    
    // Bind phase toggle to a key (e.g., Space)
    InputComponent->BindAction("TogglePhase", IE_Pressed, this, &ABasePlayerController::ToggleGamePhase);*/
}

void ABasePlayerController::OnLeftMouseClick()
{
    ProcessDrop("");
}

void ABasePlayerController::OnRightMouseClick()
{
    ProcessDrop("true");
}

void ABasePlayerController::ProcessDrop(FString ItemId)
{
    // Only process clicks in preparation phase
    if (GameMode && GameMode->GetCurrentPhase() == EPvPGamePhase::Preparation)
    {
        FHitResult HitResult;
        if (GetHitResultUnderCursor(HitResult))
        {
            if (PlayerCharacter)
            {
                // Forward the click to the character
                PlayerCharacter->OnMouseClick(HitResult, ItemId);
                
                // Log for debugging
                UE_LOG(LogTemp, Display, TEXT("%s detected at location: %s"), 
                    *ItemId,
                    *HitResult.Location.ToString());
            }
        }
    }
}

bool ABasePlayerController::GetHitResultUnderCursor(FHitResult& HitResult) const
{
    // Perform a trace from the cursor position
    FVector WorldLocation, WorldDirection;
    if (DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
    {
        // Perform a line trace from the deprojected position
        return GetWorld()->LineTraceSingleByChannel(
            HitResult,
            WorldLocation,
            WorldLocation + WorldDirection * 10000.0f,
            ECC_Visibility
        );
    }
    return false;
}

void ABasePlayerController::ToggleGamePhase()
{
    if (GameMode)
    {
        // Toggle between preparation and action phases
        if (GameMode->GetCurrentPhase() == EPvPGamePhase::Preparation)
        {
            GameMode->StartActionPhase();
        }
        else
        {
            GameMode->StartPreparationPhase();
        }
    }
}