// Fill out your copyright notice in the Description page of Project Settings.


#include "PvPGameMode.h"

#include "GodsUnited/GameLogic/Managers/ActionManager/Waypoint.h"
#include "GodsUnited/Player/Character/BaseCharacter.h"
#include "GodsUnited/Player/Components/WaypointMovementComponent.h"
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
        
        int32 ItemWayPoints = 0;
        for (AActor* Actor : FoundWaypoints)
        {
            AWaypoint* Waypoint = Cast<AWaypoint>(Actor);
            if (Waypoint && Waypoint->HasItem())
            {
                ItemWayPoints++;
            }
        }
        
        GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Cyan, 
            FString::Printf(TEXT("Total Waypoints: %d (Right-Click: %d)"), 
            FoundWaypoints.Num(), ItemWayPoints));
        
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
    if (CurrentPhase == EPvPGamePhase::Action) return;

    // Clear IdleFinishTimers
    for (FTimerHandle& Timer : IdleFinishTimers)
    {
        GetWorld()->GetTimerManager().ClearTimer(Timer);
    }
    IdleFinishTimers.Empty();
        
    // Find all player characters and tell them to start following their paths
    TArray<AActor*> FoundCharacters;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseCharacter::StaticClass(), FoundCharacters);
        
    for (AActor* Actor : FoundCharacters)
    {
        if (ABaseCharacter* Character = Cast<ABaseCharacter>(Actor))
        {
            // it means the character is idle, so we complete the movement process and broadcast
            if(!Character->GetLastWaypoint())
            {
                FTimerHandle NewTimer;
                GetWorld()->GetTimerManager().SetTimer(
                    NewTimer,
                    FTimerDelegate::CreateLambda([Character, this]()
                    {
                        if (IsValid(Character))
                        {
                            UE_LOG(LogTemp, Warning, TEXT("Broadcasting MovementCompleted for idle character: %s"), *Character->GetName());

                            bPlayerLastCardFinished = true;
                            bPlayerMovementFinished = true;
                            if (UWaypointMovementComponent* MovementComp = Character->GetWaypointMovementComponent())
                            {
                                MovementComp->OnMovementCompleted.Broadcast(Character);
                            }
                        }
                    }),
                    5.0f,
                    false
                );
                
                IdleFinishTimers.Add(NewTimer);
                continue;
            }
            
            // Start following the path
            Character->StartFollowingPath();
        }
    }

    CurrentPhase = EPvPGamePhase::Action;
    
    // Call the blueprint event
    OnGamePhaseChanged(CurrentPhase);
        
    UE_LOG(LogTemp, Display, TEXT("Action Phase Started"));
}

void APvPGameMode::StartPreparationPhase()
{
    if (CurrentPhase == EPvPGamePhase::Preparation) return;

    CurrentPhase = EPvPGamePhase::Preparation;

    bPlayerLastCardFinished = false;
    bPlayerMovementFinished = false;
        
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
            //Character->SetActorLocation(FVector(CurrentLocation.X, CurrentLocation.Y, Character->PlayerGroundOffset));
                
            // Reset the path
            SetCharacterNewRoundEnergy(Character);
        }
    }
        
    // Call the blueprint event
    OnGamePhaseChanged(CurrentPhase);
        
    UE_LOG(LogTemp, Display, TEXT("Preparation Phase Started"));
}

void APvPGameMode::DecideSwitchingToPreparationPhase()
{
    if (bPlayerMovementFinished && bPlayerLastCardFinished)
    {
        StartPreparationPhase();
    }
}

void APvPGameMode::SetCharacterNewRoundEnergy(ABaseCharacter* Character)
{
    
}
