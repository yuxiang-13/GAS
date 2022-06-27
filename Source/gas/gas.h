// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"


UENUM(BlueprintType)
enum class EGAS0AbilityInputID : uint8
{
	None	UMETA(DisplayName = "None"),
	Jump	UMETA(DisplayName ="Jump"),
	Sprint	UMETA(DisplayName = "Sprint"), //冲刺
};

