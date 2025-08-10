// Fill out your copyright notice in the Description page of Project Settings.


#include "LooterGameModeBase.h"
#include "PlayerCharacter.h"

ALooterGameModeBase::ALooterGameModeBase()
{
	// Set the default pawn class to our player character
	DefaultPawnClass = APlayerCharacter::StaticClass();
	
	// Optionally, you can set other properties such as HUD class, PlayerController class, etc.
	// HUDClass = AMyHUD::StaticClass();
	// PlayerControllerClass = AMyPlayerController::StaticClass();
}
