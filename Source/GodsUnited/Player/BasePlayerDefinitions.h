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
	static constexpr float BONUS_ENERGY_MULTIPLIER = 2.0f;
    
	// Input Scale Constants
	static constexpr float MIN_INPUT_SCALE = 0.1f;
	static constexpr float MAX_INPUT_SCALE = 1.0f;
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