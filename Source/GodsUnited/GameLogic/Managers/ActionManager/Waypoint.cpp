// Fill out your copyright notice in the Description page of Project Settings.


#include "Waypoint.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/SphereComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

AWaypoint::AWaypoint()
{
    // Set this actor to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;

    // Create collision sphere
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    RootComponent = CollisionSphere;
    CollisionSphere->SetSphereRadius(50.0f);
    CollisionSphere->SetCollisionProfileName(TEXT("Trigger"));
    CollisionSphere->SetVisibility(true);
    
    // Create and set up the mesh component
    WaypointMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaypointMesh"));
    WaypointMesh->SetupAttachment(RootComponent);
    WaypointMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    // Try to load a default sphere mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereMeshAsset.Succeeded())
    {
        WaypointMesh->SetStaticMesh(SphereMeshAsset.Object);
        WaypointMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.1f)); // Flatter sphere for top-down view
    }
    
    // Create text component for displaying waypoint index
    IndexText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IndexText"));
    IndexText->SetupAttachment(RootComponent);
    IndexText->SetRelativeLocation(FVector(0, 0, 60.0f));
    IndexText->SetHorizontalAlignment(EHTA_Center);
    IndexText->SetTextRenderColor(FColor::Green);
    IndexText->SetWorldSize(50.0f);
    // Make text face upward for top-down view
    IndexText->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
    
    // Initialize properties
    bIsRightClick = false;
    PathIndex = 0;
    OwningPlayer = nullptr;
    LastKnownPathIndex = -1; // Initialize track variable
}

void AWaypoint::BeginPlay()
{
    Super::BeginPlay();
    
    // Create dynamic material to change color based on right/left click
    UMaterialInstanceDynamic* DynamicMaterial = WaypointMesh->CreateAndSetMaterialInstanceDynamic(0);
    
    if (DynamicMaterial)
    {
        // Set color based on click type (red for right click, blue for left click)
        FLinearColor WaypointColor = bIsRightClick ? FLinearColor::Red : FLinearColor::Blue;
        DynamicMaterial->SetVectorParameterValue(TEXT("Color"), WaypointColor);
    }
    
    // Force update text immediately
    LastKnownPathIndex = -1; // Ensure it's different than PathIndex to trigger update
    
    // Update text to show waypoint index
    FString IndexString = FString::Printf(TEXT("%d"), PathIndex);
    
    // Add additional text for right-click waypoints
    if (bIsRightClick)
    {
        IndexString += TEXT(" (R)");
        IndexText->SetTextRenderColor(FColor::Red);
    }
    
    IndexText->SetText(FText::FromString(IndexString));
    
    // Log the index this waypoint is being created with
    UE_LOG(LogTemp, Warning, TEXT("Waypoint BeginPlay with index: %d"), PathIndex);
    
    // Make the sphere semi-transparent to better visualize overlapping
    CollisionSphere->SetVisibility(true);
}

void AWaypoint::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Waypoint index değişirse text'i güncelle
    if (LastKnownPathIndex != PathIndex)
    {
        LastKnownPathIndex = PathIndex;
        
        // Update text to show current waypoint index
        FString IndexString = FString::Printf(TEXT("%d"), PathIndex);
        
        // Add additional text for right-click waypoints
        if (bIsRightClick)
        {
            IndexString += TEXT(" (R)");
            IndexText->SetTextRenderColor(FColor::Red);
        }
        else
        {
            IndexText->SetTextRenderColor(FColor::Green);
        }
        
        IndexText->SetText(FText::FromString(IndexString));
        
        UE_LOG(LogTemp, Display, TEXT("Waypoint %p updated text to show index: %d"), this, PathIndex);
    }
}