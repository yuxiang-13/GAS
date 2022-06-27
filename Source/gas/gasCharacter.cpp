// Copyright Epic Games, Inc. All Rights Reserved.

#include "gasCharacter.h"

#include "AbilitySystemComponent.h"
#include "gas.h"
#include "Abilities/GameplayAbility_CharacterJump.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

//////////////////////////////////////////////////////////////////////////
// AgasCharacter

AgasCharacter::AgasCharacter()
{
	// 初始化 能力组件
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rate for input
	TurnRateGamepad = 50.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

UAbilitySystemComponent* AgasCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AgasCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (GetLocalRole() != ROLE_Authority || !IsValid(AbilitySystemComponent))
	{
		return;
	}

	AbilitySystemComponent->GiveAbility(
		FGameplayAbilitySpec(UGameplayAbility_CharacterJump::StaticClass(), 1, static_cast<int32>(EGAS0AbilityInputID::Jump), this));

	if (SprintAbility)
	{
		AbilitySystemComponent->GiveAbility(
			FGameplayAbilitySpec(SprintAbility, 1, static_cast<int32>(EGAS0AbilityInputID::Sprint), this));
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AgasCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	// PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	// PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);


	if (IsValid(AbilitySystemComponent))
	{
		// 能力組件 綁定
		AbilitySystemComponent->BindAbilityActivationToInputComponent(PlayerInputComponent,
			FGameplayAbilityInputBinds(
				FString(),
				FString(),
				FString(TEXT("EGAS0AbilityInputID")), // 定義的 技能枚舉名字
				static_cast<int32>(EGAS0AbilityInputID::None),
				static_cast<int32>(EGAS0AbilityInputID::None)
			)
		);
	}

	

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AgasCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &AgasCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &AgasCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &AgasCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AgasCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AgasCharacter::TouchStopped);
}

void AgasCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void AgasCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void AgasCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AgasCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AgasCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AgasCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
