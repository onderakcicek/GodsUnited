#pragma once

#include "CoreMinimal.h"

// Player Movement Constants
namespace PlayerMovementConstants
{
    static constexpr float DEFAULT_MOVEMENT_TOLERANCE = 10.0f;
    static constexpr float DEFAULT_BRAKING_DECELERATION = 2048.0f;
    static constexpr float DEFAULT_GROUND_FRICTION = 8.0f;
    
    // Turn Detection Constants
    static constexpr float U_TURN_THRESHOLD_RADIANS = PI * 0.8f; // ~144 degrees
    static constexpr float SLOW_TURN_THRESHOLD_RADIANS = PI * 0.7f; // ~126 degrees
    static constexpr float SLOW_DISTANCE_MULTIPLIER = 10.0f;
    
    // Energy System Constants
    static constexpr int32 BONUS_ENERGY_MULTIPLIER = 2;
    
    // Input Scale Constants
    static constexpr float MIN_INPUT_SCALE = 0.1f;
    static constexpr float MAX_INPUT_SCALE = 1.0f;
}

// Energy System Constants
namespace EnergySystemConstants
{
    static constexpr int32 DEFAULT_STARTING_ENERGY = 4;
    static constexpr int32 DEFAULT_STARTING_BUFFER_ENERGY = 0;
    static constexpr int32 BUFFER_ENERGY_GAIN_MULTIPLIER = 2;
    static constexpr int32 MAX_BUFFER_ENERGY_GAIN_PER_TURN = 4;
    static constexpr float CIRCLE_RADIUS_FOR_COST_CALCULATION = 200.0f;
    static constexpr float MIN_MOVEMENT_DISTANCE = 1.0f;
    static constexpr float ENERGY_COST_PER_UNIT_DISTANCE = 1.0f;
}

// Debug Visualization Constants
namespace PlayerDebugConstants
{
    static constexpr float ARROW_LENGTH = 200.0f;
    static constexpr float ARROW_START_HEIGHT_OFFSET = 50.0f;
    static constexpr float ARROW_SIZE = 100.0f;
    static constexpr float SPHERE_RADIUS = 60.0f;
    static constexpr int32 SPHERE_SEGMENTS = 12;
    static constexpr float LINE_THICKNESS = 2.0f;
    static constexpr float ARROW_THICKNESS = 10.0f;
    static constexpr float DEBUG_DURATION_PERSISTENT = -1.0f;
}

// Card System Constants  
namespace CardSystemConstants
{
    static constexpr float DRAG_CIRCLE_RADIUS_MULTIPLIER = 10.0f;
    static constexpr float PLACEMENT_RADIUS = 200.0f;
    static constexpr float DEBUG_SPHERE_SIZE = 20.0f;
    static constexpr float DEBUG_SPHERE_HEIGHT_OFFSET = 10.0f;
    static constexpr int32 DEBUG_CIRCLE_SEGMENTS = 32;
    static constexpr float DEBUG_CIRCLE_THICKNESS = 3.0f;
    static constexpr float DEBUG_DURATION_SHORT = 0.03f;
    static constexpr float DEBUG_DURATION_LONG = 3.0f;
}

// Energy Calculation Functions
namespace EnergyCalculations
{
    /**
     * Calculate energy cost based on ring system
     * @param Center Starting position (usually last waypoint or character position)
     * @param Point Target position where player wants to move/place
     * @param Radius Free movement radius (PLACEMENT_RADIUS)
     * @return Number of energy rings required (0 for within free radius, 1-N for outside)
     */
    FORCEINLINE int32 CalculateEnergyRings(const FVector& Center, const FVector& Point, float Radius)
    {
        const float Distance2D = FVector::Dist2D(Center, Point);
        
        // Within free radius costs no energy
        if (Distance2D <= Radius) 
        {
            return 0;
        }
        
        // Calculate how many additional radius units we're going beyond the free zone
        const float ExcessDistance = Distance2D - Radius;
        return FMath::CeilToInt(ExcessDistance / Radius);
    }

    /**
     * Calculate buffer energy gain from movement distance
     * @param TotalDistance Total distance traveled during movement
     * @param PlacementRadius Base radius for energy calculations
     * @return Amount of buffer energy to award
     */
    FORCEINLINE int32 CalculateBufferEnergyFromDistance(float TotalDistance, float PlacementRadius)
    {
        if (TotalDistance <= 0.0f)
            return 0;
        
        // Convert distance to energy units
        float BufferGain = TotalDistance / PlacementRadius;
    
        // Apply multiplier
        BufferGain *= EnergySystemConstants::BUFFER_ENERGY_GAIN_MULTIPLIER;
    
        // Cap maximum gain per turn and convert to integer
        int32 FinalGain = FMath::FloorToInt(FMath::Min(BufferGain, static_cast<float>(EnergySystemConstants::MAX_BUFFER_ENERGY_GAIN_PER_TURN)));
    
        return FinalGain;
    }

    /**
     * Calculate total available energy including bonus energy with multiplier
     * @param RegularEnergy Base energy amount
     * @param BufferEnergy Buffer energy amount
     * @return Total usable energy
     */
    FORCEINLINE int32 CalculateTotalAvailableEnergy(int32 RegularEnergy, int32 BufferEnergy)
    {
        return RegularEnergy + (BufferEnergy * PlayerMovementConstants::BONUS_ENERGY_MULTIPLIER);
    }
}