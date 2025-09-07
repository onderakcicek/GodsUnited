#include "BasePlayerController.h"
#include "../../Drag/GenericDragDropOperation.h"

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
    
    // Initialize drag state
    bIsDragTracking = false;
    InitialMousePosition = FVector2D::ZeroVector;
    CurrentDragOperation = nullptr;
    MinimumDragDistance = 50.0f;
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
    
    // Handle drag tracking
    if (bIsDragTracking)
    {
        UE_LOG(LogTemp, Error, TEXT("TICK: Drag tracking active, checking distance..."));
        FVector2D CurrentMousePosition;
        GetMousePosition(CurrentMousePosition.X, CurrentMousePosition.Y);
        
        if (ShouldStartDrag(CurrentMousePosition))
        {
            UE_LOG(LogTemp, Error, TEXT("TICK: Starting movement drag!"));
            StartMovementDrag(InitialMousePosition);
        }
    }
}

void ABasePlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // Bind mouse events for drag detection (for non-touch platforms)
    InputComponent->BindAction("LeftClick", IE_Pressed, this, &ABasePlayerController::OnLeftMouseDown);
    InputComponent->BindAction("LeftClick", IE_Released, this, &ABasePlayerController::OnLeftMouseUp);
    
    // Bind touch events for touch platforms (Use Mouse for Touch)
    InputComponent->BindTouch(IE_Pressed, this, &ABasePlayerController::OnTouchStarted);
    InputComponent->BindTouch(IE_Repeat, this, &ABasePlayerController::OnTouchMoved);
    InputComponent->BindTouch(IE_Released, this, &ABasePlayerController::OnTouchEnded);
    
    // Bind phase toggle to a key (e.g., Space)
    //InputComponent->BindAction("TogglePhase", IE_Pressed, this, &ABasePlayerController::ToggleGamePhase);
    
    UE_LOG(LogTemp, Display, TEXT("Input component setup completed"));
}

void ABasePlayerController::OnLeftMouseDown()
{
    UE_LOG(LogTemp, Warning, TEXT("OnLeftMouseDown called"));
    
    // drag tracking 
    FVector2D MousePosition;
    GetMousePosition(MousePosition.X, MousePosition.Y);
    StartDragTracking(MousePosition);
    UE_LOG(LogTemp, Warning, TEXT("Started drag tracking (GameMode independent)"));
}

void ABasePlayerController::OnLeftMouseUp()
{
    if (bIsDragTracking)
    {
        // Mouse up without drag - treat as simple click
        if (!CurrentDragOperation)
        {
            ProcessDrop(""); // Empty ItemId for movement waypoint
        }
        
        StopDragTracking();
    }
}

void ABasePlayerController::OnMouseMove()
{
    // This method is for potential future use
    // Currently drag detection happens in Tick
}

//-------------------------------------------
// Touch Input Events
//-------------------------------------------

void ABasePlayerController::OnTouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
    UE_LOG(LogTemp, Error, TEXT("=== TOUCH DEBUG ==="));
    UE_LOG(LogTemp, Error, TEXT("Touch Location: %s"), *Location.ToString());
    
    // Only handle primary finger for simplicity
    if (FingerIndex != ETouchIndex::Touch1)
        return;
    
    // Get screen position and start drag tracking
    FVector2D MousePos;
    GetMousePosition(MousePos.X, MousePos.Y);
    UE_LOG(LogTemp, Error, TEXT("Mouse Position during touch: %s"), *MousePos.ToString());
    
    // Start drag tracking like mouse does
    StartDragTracking(MousePos);
}

void ABasePlayerController::OnTouchMoved(ETouchIndex::Type FingerIndex, FVector Location)
{
    // Only handle primary finger
    if (FingerIndex != ETouchIndex::Touch1)
        return;
    
    // Touch movement is handled in Tick for consistency with mouse drag
    if (bIsDragTracking)
    {
        UE_LOG(LogTemp, Warning, TEXT("Touch moved during drag tracking"));
    }
}

void ABasePlayerController::OnTouchEnded(ETouchIndex::Type FingerIndex, FVector Location)
{
    UE_LOG(LogTemp, Warning, TEXT("Touch ended at location: %s"), *Location.ToString());
    
    // Only handle primary finger
    if (FingerIndex != ETouchIndex::Touch1)
        return;
    
    if (bIsDragTracking)
    {
        // Touch up without drag - treat as simple click
        if (!CurrentDragOperation)
        {
            UE_LOG(LogTemp, Warning, TEXT("Touch ended without drag - processing as click"));
            ProcessTouchClick(Location);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Touch ended with active drag operation"));
        }
        
        StopDragTracking();
    }
}

//-------------------------------------------
// Drag&Drop Lifecycle
//-------------------------------------------

void ABasePlayerController::StartDragTracking(const FVector2D& MousePosition)
{
    UE_LOG(LogTemp, Error, TEXT("StartDragTracking called with position: %s"), *MousePosition.ToString());
    
    bIsDragTracking = true;
    InitialMousePosition = MousePosition;
    CurrentDragOperation = nullptr;
    
    UE_LOG(LogTemp, Error, TEXT("Drag tracking state set: bIsDragTracking=%s, InitialPos=%s"), 
        bIsDragTracking ? TEXT("true") : TEXT("false"), 
        *InitialMousePosition.ToString());
}

bool ABasePlayerController::ShouldStartDrag(const FVector2D& CurrentMousePosition) const
{
    return true;
    /*float DragDistance = FVector2D::Distance(InitialMousePosition, CurrentMousePosition);
    UE_LOG(LogTemp, Error, TEXT("Drag Distance Check: Initial=%s, Current=%s, Distance=%.2f, Threshold=%.2f"), 
        *InitialMousePosition.ToString(), *CurrentMousePosition.ToString(), DragDistance, MinimumDragDistance);
    
    bool bShouldStart = DragDistance >= MinimumDragDistance;
    UE_LOG(LogTemp, Error, TEXT("Should Start Drag: %s"), bShouldStart ? TEXT("YES") : TEXT("NO"));
    
    return bShouldStart;*/
}

void ABasePlayerController::StartMovementDrag(const FVector2D& StartPosition)
{
    // Debug coordinate'ları
   UE_LOG(LogTemp, Error, TEXT("=== MOVEMENT DRAG DEBUG ==="));
   UE_LOG(LogTemp, Error, TEXT("StartPosition: %s"), *StartPosition.ToString());
   
   // Mouse position'ı da kontrol et
   FVector2D CurrentMousePos;
   GetMousePosition(CurrentMousePos.X, CurrentMousePos.Y);
   UE_LOG(LogTemp, Error, TEXT("Current Mouse Position: %s"), *CurrentMousePos.ToString());
   
   // Viewport size'ı kontrol et
   int32 ViewportSizeX, ViewportSizeY;
   GetViewportSize(ViewportSizeX, ViewportSizeY);
   UE_LOG(LogTemp, Error, TEXT("Viewport Size: %d x %d"), ViewportSizeX, ViewportSizeY);
   
   // STOP DRAG TRACKING IMMEDIATELY to prevent infinite loop
   StopDragTracking();
   
   // Validate drag actor classes
   if (!MovementDraggingActorClass)
   {
       UE_LOG(LogTemp, Warning, TEXT("MovementDraggingActorClass not set - cannot start movement drag"));
       return;
   }

   // Create movement drag operation
   CurrentDragOperation = NewObject<UGenericDragDropOperation>();
   if (!CurrentDragOperation)
   {
       UE_LOG(LogTemp, Error, TEXT("Failed to create movement drag operation"));
       return;
   }

   // Configure drag operation
   CurrentDragOperation->CachedPC = this;
   CurrentDragOperation->PlayerCharacter = PlayerCharacter;

   // Use current mouse position instead of start position for better coordinate accuracy
   FPointerEvent SyntheticEvent(
       0,                              // InPointerIndex
       CurrentMousePos,                // InScreenSpacePosition - use current, not start
       CurrentMousePos,                // InLastScreenSpacePosition
       TSet<FKey>(),                   // InPressedButtons
       FKey(),                         // InEffectingButton
       0.0f,                          // InWheelDelta
       FModifierKeysState()           // InModifierKeysState
   );

   // Start movement drag (empty ItemId for movement)
   CurrentDragOperation->StartDragOperation(
       MovementDraggingActorClass,
       MovementFinalActorClass,
       TEXT(""), // Empty ItemId indicates movement waypoint
       SyntheticEvent
   );

   UE_LOG(LogTemp, Display, TEXT("Started movement drag operation"));
}

void ABasePlayerController::StopDragTracking()
{
    bIsDragTracking = false;
    InitialMousePosition = FVector2D::ZeroVector;
    CurrentDragOperation = nullptr;
}

void ABasePlayerController::CancelCurrentDrag()
{
    if (CurrentDragOperation && IsValid(CurrentDragOperation))
    {
        // Create synthetic pointer event for drag cancel (UE 5.4)
        FPointerEvent SyntheticEvent(
            0,                              // InPointerIndex
            FVector2D::ZeroVector,          // InScreenSpacePosition
            FVector2D::ZeroVector,          // InLastScreenSpacePosition
            TSet<FKey>(),                   // InPressedButtons
            FKey(),                         // InEffectingButton
            0.0f,                          // InWheelDelta
            FModifierKeysState()           // InModifierKeysState
        );
        
        CurrentDragOperation->DragCancelled_Implementation(SyntheticEvent);
    }
    
    StopDragTracking();
}

//-------------------------------------------
// Utility Methods
//-------------------------------------------

bool ABasePlayerController::GetHitResultUnderCursor(FHitResult& HitResult, const FVector2D* ScreenPosition) const
{
    if (ScreenPosition)
    {
        UE_LOG(LogTemp, Warning, TEXT("Tracing at screen position: %s"), *ScreenPosition->ToString());
        bool bHit = GetHitResultAtScreenPosition(*ScreenPosition, ECC_Visibility, false, HitResult);
        
        if (bHit)
        {
            UE_LOG(LogTemp, Warning, TEXT("Hit Location: %s"), *HitResult.Location.ToString());
            UE_LOG(LogTemp, Warning, TEXT("Hit Normal: %s"), *HitResult.Normal.ToString());
            UE_LOG(LogTemp, Warning, TEXT("Hit Component: %s"), HitResult.Component.IsValid() ? *HitResult.Component->GetName() : TEXT("None"));
            
            // Check for NaN values
            if (HitResult.Location.ContainsNaN())
            {
                UE_LOG(LogTemp, Error, TEXT("HIT RESULT CONTAINS NaN!"));
                return false;
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Trace result: %s"), bHit ? TEXT("HIT") : TEXT("MISS"));
        return bHit;
    }
    
    // Use current cursor position
    FVector WorldLocation, WorldDirection;
    if (DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
    {
        return GetWorld()->LineTraceSingleByChannel(
            HitResult,
            WorldLocation,
            WorldLocation + WorldDirection * 10000.0f,
            ECC_Visibility
        );
    }
    
    return false;
}

void ABasePlayerController::ProcessDrop(FString ItemId)
{
    // GameMode independent click processing for testing
    FHitResult HitResult;
    if (GetHitResultUnderCursor(HitResult))
    {
        if (PlayerCharacter)
        {
            // Forward the click to the character's waypoint component
            PlayerCharacter->OnMouseClick(HitResult, ItemId);
            
            UE_LOG(LogTemp, Display, TEXT("Processed %s click at location: %s"), 
                ItemId.IsEmpty() ? TEXT("Movement") : TEXT("Card"),
                *HitResult.Location.ToString());
        }
    }
}

void ABasePlayerController::ProcessTouchClick(const FVector& TouchWorldLocation)
{
    UE_LOG(LogTemp, Error, TEXT("ProcessTouchClick called with: %s"), *TouchWorldLocation.ToString());
    
    // Don't use GetMousePosition - it returns (0,0) during touch
    // Use TouchWorldLocation.X and TouchWorldLocation.Y as screen coordinates
    FVector2D TouchScreenPos(TouchWorldLocation.X, TouchWorldLocation.Y);
    
    UE_LOG(LogTemp, Warning, TEXT("Using touch screen position: %s"), *TouchScreenPos.ToString());
    
    FHitResult HitResult;
    if (GetHitResultUnderCursor(HitResult, &TouchScreenPos))
    {
        if (PlayerCharacter)
        {
            PlayerCharacter->OnMouseClick(HitResult, TEXT(""));
            UE_LOG(LogTemp, Display, TEXT("Processed touch click via screen trace at: %s"), *HitResult.Location.ToString());
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to get hit result for touch click at screen pos: %s"), *TouchScreenPos.ToString());
    }
}

void ABasePlayerController::ToggleGamePhase()
{
    if (GameMode)
    {
        // Cancel any active drag when switching phases
        if (bIsDragTracking || CurrentDragOperation)
        {
            CancelCurrentDrag();
        }
        
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