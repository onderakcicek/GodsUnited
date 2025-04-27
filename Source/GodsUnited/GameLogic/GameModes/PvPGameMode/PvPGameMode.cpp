// Fill out your copyright notice in the Description page of Project Settings.


#include "PvPGameMode.h"

#include "GodsUnited/GameLogic/Managers/ActionManager/Waypoint.h"
#include "GodsUnited/Player/Character/BaseCharacter.h"
#include "Kismet/GameplayStatics.h"

APvPGameMode::APvPGameMode()
{
    // Default to preparation phase when game starts
    CurrentPhase = EPvPGamePhase::Preparation;
}

void APvPGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bShowDebugInfo)
    {
        // Display current phase on screen
        GEngine->AddOnScreenDebugMessage(0, 0.0f, FColor::Yellow, 
            FString::Printf(TEXT("Current Phase: %s"), 
            CurrentPhase == EPvPGamePhase::Preparation ? TEXT("Preparation") : TEXT("Action")));
        
        // Count waypoints in the world
        TArray<AActor*> FoundWaypoints;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWaypoint::StaticClass(), FoundWaypoints);
        
        int32 RightClickWaypoints = 0;
        for (AActor* Actor : FoundWaypoints)
        {
            AWaypoint* Waypoint = Cast<AWaypoint>(Actor);
            if (Waypoint && Waypoint->bIsRightClick)
            {
                RightClickWaypoints++;
            }
        }
        
        GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Cyan, 
            FString::Printf(TEXT("Total Waypoints: %d (Right-Click: %d)"), 
            FoundWaypoints.Num(), RightClickWaypoints));
        
        // Display controls hint
        GEngine->AddOnScreenDebugMessage(2, 0.0f, FColor::Green, 
            TEXT("Controls: Left/Right Click = Add Waypoint, Space = Toggle Phase"));
    }
}

void APvPGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Additional setup logic here
    
    // Start in preparation phase (this will trigger the blueprint event)
    StartPreparationPhase();
}

void APvPGameMode::StartActionPhase()
{
    if (CurrentPhase != EPvPGamePhase::Action)
    {
        CurrentPhase = EPvPGamePhase::Action;
        
        // Find all player characters and tell them to start following their paths
        TArray<AActor*> FoundCharacters;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseCharacter::StaticClass(), FoundCharacters);
        
        for (AActor* Actor : FoundCharacters)
        {
            ABaseCharacter* Character = Cast<ABaseCharacter>(Actor);
            if (Character)
            {
                // Move character to ground level for action phase
                FVector CurrentLocation = Character->GetActorLocation();
                Character->SetActorLocation(FVector(CurrentLocation.X, CurrentLocation.Y, Character->PlayerGroundOffset));
                
                // Enable gravity for action phase (character will follow ground)
                //Character->EnableGravity();
                
                // Start following the path
                Character->StartFollowingPath();
            }
        }
        
        // Call the blueprint event
        OnGamePhaseChanged(CurrentPhase);
        
        UE_LOG(LogTemp, Display, TEXT("Action Phase Started"));
    }
}

void APvPGameMode::StartPreparationPhase()
{
    if (CurrentPhase != EPvPGamePhase::Preparation)
    {
        CurrentPhase = EPvPGamePhase::Preparation;
        
        // Clear all existing waypoints when returning to preparation phase
        TArray<AActor*> FoundWaypoints;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWaypoint::StaticClass(), FoundWaypoints);
        
        for (AActor* Actor : FoundWaypoints)
        {
            Actor->Destroy();
        }
        
        // Reset all player characters
        TArray<AActor*> FoundCharacters;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseCharacter::StaticClass(), FoundCharacters);
        
        for (AActor* Actor : FoundCharacters)
        {
            ABaseCharacter* Character = Cast<ABaseCharacter>(Actor);
            if (Character)
            {
                // Move character back up for planning (bird's eye view)
                FVector CurrentLocation = Character->GetActorLocation();
                Character->SetActorLocation(FVector(CurrentLocation.X, CurrentLocation.Y, Character->PlayerGroundOffset));
                
                // Reset the path
                Character->ResetPath();
            }
        }
        
        // Call the blueprint event
        OnGamePhaseChanged(CurrentPhase);
        
        UE_LOG(LogTemp, Display, TEXT("Preparation Phase Started"));
    }
}