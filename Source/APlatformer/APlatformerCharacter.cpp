#include "APlatformerCharacter.h"
// Copyright Epic Games, Inc. All Rights Reserved.
#include "APlatformerProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MySaveGame.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "PCThing.h"
#include "MainGI.h"


//////////////////////////////////////////////////////////////////////////
// AAPlatformerCharacter

void AAPlatformerCharacter::EnableMoveInput(const FHitResult& Hit)
{
  PCRef->SetIgnoreMoveInput(false);
  LandedDelegate.RemoveDynamic(this, &AAPlatformerCharacter::EnableMoveInput);
}

void AAPlatformerCharacter::ResetClimbCooldown()
{
  UKismetSystemLibrary::PrintString(this, "resetting climbing timer");
  GetWorld()->GetTimerManager().ClearTimer(ClimbingTimer);
}

void AAPlatformerCharacter::Climb(const FInputActionValue &InputActionValue)
{
  if (!bHasClimbingGear)
  {
    return;
  }
  //climbing shouldn't happen when movement is disabled
  if (PCRef->IsMoveInputIgnored())
  {
    return;
  }
  if (!GetCharacterMovement()->IsFalling())
  {
    return;
  }
  FVector2D MovementVector = InputActionValue.Get<FVector2D>();
  LastMoveInput = MovementVector;

  bool bForwardInput = (MovementVector.Y > 0.f);
  bool bBackwardInput = (MovementVector.Y < 0.f);

  if (bBackwardInput)
  {
    StopLedgeHangAndClimbing(FInputActionValue(true));
    return;
  }
  else if (!bForwardInput)
  {
    return;
  }

  //bypass if on ledge
  if (bIsOnLedge)
  {
    //checks if the difference in yaw in degrees is greater than 30 and returns if true
    //don't wnat people to climb up ledge while facing away from the ledge
    FVector ForwardVector = GetActorForwardVector();
    FVector LedgeVector = LedgePos - GetActorLocation();
    LedgeVector.Z = 0.f; // Set Z component to zero to get the XY component
    LedgeVector.Normalize();

    float DotProduct = FVector::DotProduct(ForwardVector, LedgeVector);
    float AngleInDegrees = UKismetMathLibrary::DegAcos(DotProduct);

    if (AngleInDegrees > 90.f)
    {
      StopLedgeHangAndClimbing(FInputActionValue(true));
      return;
    }
    else if (AngleInDegrees > 15.f)
    {
      return;
    }

    //bUt DrY pRogRaMmInG!1!
    if (GetCharacterMovement()->Velocity.Z <= MaxClimbVelocityZ)
    {
      GetCharacterMovement()->AddForce(FVector(0.f, 0.f, 170000.f));
    }
    return;
  }


  double RemainingTime = GetWorld()->GetTimerManager().GetTimerRemaining(ClimbingTimer);
  if (!bIsClimbing)
  {
    if (RemainingTime==69420.f)
    {
      return;
    }
    else
    {
      UKismetSystemLibrary::PrintString(this, FString::SanitizeFloat(RemainingTime));
    }
    if (!ForwardWallDetect())
    {
      return;
    }

    bIsClimbing = true;
    //sprinting has no effect on climbing
    GetCharacterMovement()->MaxWalkSpeed = MaxSpeedDefault * 0.75;

    //disable gravity
    GetCharacterMovement()->GravityScale = 0;

    //also increase the air friction to allow for better lateral control while climbing (but slower max speed)
    GetCharacterMovement()->FallingLateralFriction = 10.f;

    //also detect if on ledge
    if (LedgeDetect())
    {
      bIsOnLedge = true;
      //disable pesky timers
      GetWorld()->GetTimerManager().ClearTimer(ClimbingTimer);
    }
    else
    {
      //start climbing timer to stop climbing if timer does not exist, else if timer is above zero unpause, else return
      if (GetWorld()->GetTimerManager().GetTimerRemaining(ClimbingTimer)>0.f)
      {
        GetWorld()->GetTimerManager().UnPauseTimer(ClimbingTimer);
        UKismetSystemLibrary::PrintString(this, "unpausing climbing timer");
      }
      else
      {
        GetWorld()->GetTimerManager().SetTimer(ClimbingTimer, this, &AAPlatformerCharacter::StopClimbing, MaxClimbLength, false);
      }
    }
  }
  if (LedgeDetect())
  {
    bIsOnLedge = true;
    //disable pesky timers
    GetWorld()->GetTimerManager().ClearTimer(ClimbingTimer);
    //bind mantle delegate
    TickDelegate.AddDynamic(this, &AAPlatformerCharacter::LedgeMantle);
  }
  if (GetCharacterMovement()->Velocity.Z <= MaxClimbVelocityZ)
  {
    GetCharacterMovement()->AddForce(FVector(0.f, 0.f, 170000.f));
  }
}

void AAPlatformerCharacter::StopClimbing()
{
  GetCharacterMovement()->GravityScale = 1.f;
  if (bIsOnLedge)
  {
    if (!TickDelegate.Contains(this, FName("LedgeHang")))
    {
      TickDelegate.AddDynamic(this, &AAPlatformerCharacter::LedgeHang);
    }
    return;
  }
  bIsClimbing = false;
  GetCharacterMovement()->FallingLateralFriction = 0.f;
  if (bIsSprinting)
  {
    GetCharacterMovement()->MaxWalkSpeed = MaxSpeedDefault * 1.5;
  }
  else
  {
    GetCharacterMovement()->MaxWalkSpeed = MaxSpeedDefault;
  }
  //set it to a specific number to show that it is completed
  if (GetWorld()->GetTimerManager().GetTimerRemaining(ClimbingTimer) <= 0.f)
  {
    UKismetSystemLibrary::PrintString(this, "setting the timer to 69420");
    GetWorld()->GetTimerManager().SetTimer(ClimbingTimer, this, &AAPlatformerCharacter::StopClimbing, 69420.f, false);
  }
  GetWorld()->GetTimerManager().PauseTimer(ClimbingTimer);
  GetWorld()->GetTimerManager().ClearTimer(ClimbCooldownTimer);
  if (!GetCharacterMovement()->IsFalling())
  {
    ResetClimbCooldown();
    return;
  }
  //start a timer to reset the climb timer
  GetWorld()->GetTimerManager().SetTimer(ClimbCooldownTimer, this, &AAPlatformerCharacter::ResetClimbCooldown, ClimbCooldown, false);
}

void AAPlatformerCharacter::StopClimbingActionMethod()
{
  UKismetSystemLibrary::PrintString(this, FString::SanitizeFloat(LastMoveInput.Y));
  //stopclimbing will only be called on forward input end
  if (LastMoveInput.Y > 0)
  {
    StopClimbing();
  }
}

bool AAPlatformerCharacter::ForwardWallDetect()
{
  TArray<FHitResult> HitResults;
  FVector StartLocation = GetActorLocation();
  FVector EndLocation = StartLocation + GetActorForwardVector()*40.f;

  FCollisionShape MyCapsule = FCollisionShape::MakeCapsule(0.5, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

  // Perform the capsule trace
  bool bHit = GetWorld()->SweepMultiByChannel(HitResults, StartLocation, EndLocation, FQuat::Identity, ECC_Visibility, MyCapsule);

  if (bHit)
  {
    bool bHasHitActor;
    for (auto& Hit : HitResults)
    {
      AActor* HitActor = Hit.GetActor();
      if (HitActor == nullptr)
      {
        continue;
      }
      bHasHitActor = true;
      if (HitActor->ActorHasTag(FName("NotClimbable")))
      {
        return false;
      }
    }
    if (bHasHitActor)
    {
      return true;
    }
  }

  return false;
}

bool AAPlatformerCharacter::LedgeDetect()
{
  FHitResult HitResult = FHitResult();
  TArray<AActor*> EmptyActorArray;
  FVector StartVector = GetActorLocation()+FVector(0.f,0.f,96.f);
  FVector EndVector = StartVector+UKismetMathLibrary::Multiply_VectorFloat(GetActorForwardVector(), 41.f);
  //start it a little above camera, go for same 41cm length forward, go to wall detect test if nothing hit
  if(UKismetSystemLibrary::LineTraceSingle(this, StartVector, EndVector, ETraceTypeQuery::TraceTypeQuery1, false, EmptyActorArray, EDrawDebugTrace::ForOneFrame, HitResult, true))
  {
    return false;
  }
  if (ForwardWallDetect())
  {
    LedgePos = EndVector;
    return true;
  }
  //this means you are no longer facing the wall properly
  StopClimbing();
  return false;
}

void AAPlatformerCharacter::LedgeHang()
{
  UKismetSystemLibrary::PrintString(this, "seeing if can hang on ledge");
  if (GetActorLocation().Z < LedgePos.Z-96.f)
  {
    GetCharacterMovement()->GravityScale = 0.f;
    GetCharacterMovement()->StopMovementImmediately();
    SetActorLocation(FVector(GetActorLocation().X, GetActorLocation().Y, LedgePos.Z-96.f));
    SetIgnoreStrafing(true);
    SetIgnoreForward(true);
    TickDelegate.RemoveDynamic(this, &AAPlatformerCharacter::LedgeHang);
  }
}

void AAPlatformerCharacter::LedgeMantle()
{
  UKismetSystemLibrary::PrintString(this, "seeing if can hang mantle");
  //the point of no return (can't cancel mantling after reaching this height)
  if (GetActorLocation().Z >= LedgePos.Z + 20.f)
  {
    //unbind self from tick delegate
    TickDelegate.RemoveDynamic(this, &AAPlatformerCharacter::LedgeMantle);
    TickDelegate.RemoveDynamic(this, &AAPlatformerCharacter::LedgeHang);
    bIsOnLedge = false;
    //disable wasd player input until mantle is complete (also call stopclimbing)
    StopClimbing();
    PCRef->SetIgnoreMoveInput(true);
    SetIgnoreStrafing(false);
    SetIgnoreForward(false);
    //bind re-enabling input to landing delegate
    LandedDelegate.AddDynamic(this, &AAPlatformerCharacter::EnableMoveInput);
    //move root component i.e. capsule component to correct landing position (ignore some minor clipping)
    FLatentActionInfo LatentInfo;
    LatentInfo.CallbackTarget = this;
    LatentInfo.ExecutionFunction = "MoveCapsule";
    LatentInfo.Linkage = 0;
    LatentInfo.UUID = 1;
    FVector NewLocation = LedgePos + (GetActorForwardVector()*50.f) + FVector(0.f, 0.f, 98.f);
    UKismetSystemLibrary::MoveComponentTo(GetCapsuleComponent(), NewLocation, FRotator::ZeroRotator, false, false, 0.5, false, EMoveComponentAction::Type::Move, LatentInfo);
  }
}

void AAPlatformerCharacter::StopLedgeHangAndClimbing(const FInputActionValue &InputActionValue)
{
  if (!bIsClimbing)
  {
    return;
  }
  if (!bIsOnLedge)
  {
    return;
  }
  UKismetSystemLibrary::PrintString(this, "trying stopledgehandandclimbing");
  if (InputActionValue.GetValueType() == EInputActionValueType::Axis2D)
  {
    FVector2D MovementVector = InputActionValue.Get<FVector2D>();

    if (MovementVector.Y >= 0.f)
    {
      UKismetSystemLibrary::PrintString(this, "not backward");
      return;
    }
  }
  TickDelegate.RemoveDynamic(this, &AAPlatformerCharacter::LedgeHang);
  TickDelegate.RemoveDynamic(this, &AAPlatformerCharacter::LedgeMantle);
  SetIgnoreStrafing(false);
  SetIgnoreForward(false);
  bIsOnLedge = false;
  StopClimbing();
}

void AAPlatformerCharacter::SetIgnoreStrafing(bool ShouldDisable)
{
  if (ShouldDisable)
  {
    StrafeMultiplier = 0.f;
  }
  else
  {
    StrafeMultiplier = 1.f;
  }
}

void AAPlatformerCharacter::SetIgnoreForward(bool ShouldDisable)
{
  if (ShouldDisable)
  {
    ForwardMultiplier = 0.f;
  }
  else
  {
    ForwardMultiplier = 1.f;
  }
}

void AAPlatformerCharacter::ResetSlideCooldown()
{
}

AAPlatformerCharacter::AAPlatformerCharacter()
{
	// Character doesnt have a rifle at start
	bHasRifle = false;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

  //Define the player controller reference
  PCRef = Cast<APCThing>(UGameplayStatics::GetPlayerController(this, 0));

  //Set the MaxWalkSpeed and JumpZVelocity in case from save
  GetCharacterMovement()->MaxWalkSpeed = MaxSpeedDefault;
  GetCharacterMovement()->JumpZVelocity = JumpVelocity;
  GetCharacterMovement()->AirControl = AirControl;
}
//destructor
void AAPlatformerCharacter::Destroyed()
{
  DetachFromControllerPendingDestroy();
	Super::Destroyed();
}

void AAPlatformerCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

  //Define the game instance reference
  GIRef = Cast<UMainGI>(GetGameInstance());
  
	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
  //there is a small delay before saving since the save relies on other variables in other objects set in begin play
  
  GetWorldTimerManager().SetTimer(UnusedHandle, this, &AAPlatformerCharacter::LoadSave, 0.001f, false);
  
  //Add our landing function to the delegate
  LandedDelegate.AddDynamic(this, &AAPlatformerCharacter::Landing);
}

void AAPlatformerCharacter::Tick(float DeltaSeconds)
{
  TickDelegate.Broadcast();
}

//////////////////////////////////////////////////////////////////////////// Input

void AAPlatformerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Ongoing, this, &ACharacter::StopJumping);
    //Crouching and sliding
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AAPlatformerCharacter::CrouchE);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AAPlatformerCharacter::UnCrouchE);

    EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AAPlatformerCharacter::CheckSlideSpeed);


		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAPlatformerCharacter::Move);

    //for climbing
    EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAPlatformerCharacter::Climb);
    EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AAPlatformerCharacter::StopClimbingActionMethod);

    //ledge cancel
    EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Started, this, &AAPlatformerCharacter::StopLedgeHangAndClimbing);
    EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AAPlatformerCharacter::StopLedgeHangAndClimbing);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAPlatformerCharacter::Look);

    //Sprinting
    EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AAPlatformerCharacter::Sprint);

    //Open the Paused Menu
    EnhancedInputComponent->BindAction(PausedMenuAction, ETriggerEvent::Started, this, &AAPlatformerCharacter::TriggerPausedMenu);
	}
}


void AAPlatformerCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
    if (MovementVector.Y>0)
    {
		  AddMovementInput(GetActorForwardVector(), MovementVector.Y * ForwardMultiplier);
    }
    else
    {
      AddMovementInput(GetActorForwardVector(), MovementVector.Y);
    }

		AddMovementInput(GetActorRightVector(), MovementVector.X * StrafeMultiplier);
	}
}

void AAPlatformerCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AAPlatformerCharacter::CrouchE()
{
  //check for sprinting to slide and falling to queueing to slide
  if (bIsSprinting && bHasSlidingGear)
  {
    if (GetCharacterMovement()->IsFalling())
    {
      if (!LandedDelegate.Contains(this, FName("BeginSlide")))
      {
        LandedDelegate.AddDynamic(this, &AAPlatformerCharacter::BeginSlide);
        UKismetSystemLibrary::PrintString(this, "binded landeddelegate with beginslide");
      }
    }
    else
    {
      BeginSlide();
    }
  }
  if (!bIsSliding)
  {
    Crouch();
  }
}

void AAPlatformerCharacter::UnCrouchE()
{
  if (!bHasSlidingGear)
  {
    UnCrouch();
    return;
  }
  if (bIsSliding)
  {
    EndSlideCompletely();
    return;
  }
  else
  {
    while (JumpDelegate.Contains(this, FName("EndSlideInAir")))
    {
      JumpDelegate.RemoveDynamic(this, &AAPlatformerCharacter::EndSlideInAir);
    }
    while (LandedDelegate.Contains(this, FName("BeginSlide")))
    {
      LandedDelegate.RemoveDynamic(this, &AAPlatformerCharacter::BeginSlide);
    }
  }
  UnCrouch();
}

void AAPlatformerCharacter::Sprint()
{
  if (!bHasShoes) 
  {
    return;
  }
  if (GetCharacterMovement()->MaxWalkSpeed == MaxSpeedDefault) 
  {
    GetCharacterMovement()->MaxWalkSpeed = MaxSpeedDefault * 1.5;
    bIsSprinting = true;
  } 
  else
  {
    GetCharacterMovement()->MaxWalkSpeed = MaxSpeedDefault;
    bIsSprinting = false;
  }
}
//TODO make sliidinggear a requirement
void AAPlatformerCharacter::BeginSlide(const FHitResult& Hit)
{
  //unbind self from landed delegate
  if (LandedDelegate.Contains(this, FName("BeginSlide")))
  {
    LandedDelegate.RemoveDynamic(this, &AAPlatformerCharacter::BeginSlide);
  }
  UnCrouch();
  UKismetSystemLibrary::PrintString(this, "Attempting beginslide velocity check");
  //check xy velocity with this vector
  FVector LastVelocityVector = GetVelocity();
  LastVelocityVector.Z = 0;

  if (LastVelocityVector.Size() > 650.f && LastVelocityVector.Size() < 1100.f)
  {
    bIsSliding = true;
    FTimerHandle SlideHeightChangeTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(SlideHeightChangeTimerHandle, this, &AAPlatformerCharacter::SlideHeightChangeTrue, 0.01, false);
    PCRef->SetIgnoreMoveInput(true);
    //turn ground friction lower
    GetCharacterMovement()->GroundFriction = 0.25;
    GetCharacterMovement()->BrakingDecelerationWalking = 300.f;

    //boost the character in the direction of SlideForceVector over 0.2s (can cancel with EndSlideCompletely or EndSlideInAir)
    if (GetWorld()->GetTimerManager().GetTimerRemaining(SlideCooldownTimer)<=0.f)
    {
      //set the slideforcevector
      FHitResult HitResult;
      FVector StartLocation = GetActorLocation();
      FVector EndLocation = StartLocation - FVector(0, 0, 20.f);
      GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility);
      FVector GroundNormal = HitResult.Normal;
      SlideForceVector = LastVelocityVector - (FVector::DotProduct(LastVelocityVector, GroundNormal) * GroundNormal);
      SlideForceVector.Normalize();
      
      UKismetSystemLibrary::PrintString(this, "boosting with this vector: "+ SlideForceVector.ToString());
      GetWorldTimerManager().SetTimer(SlideForceTimerHandle, this, &AAPlatformerCharacter::ApplySlideForce, 0.02, true, 0.f);
    }
    //trigger slide cooldown
    GetWorld()->GetTimerManager().SetTimer(SlideCooldownTimer, this, &AAPlatformerCharacter::ResetSlideCooldown, SlideCooldown, false);
    //bind end slide to jumpdelegate if not already binded
    if (!JumpDelegate.Contains(this, FName("EndSlideInAir")))
    {
      JumpDelegate.AddDynamic(this, &AAPlatformerCharacter::EndSlideInAir);
    }
  }
}

void AAPlatformerCharacter::ApplySlideForce()
{
  FVector FrameLaunchForce = SlideForceVector * 2000000.f * 0.02f / 0.2f;
  GetCharacterMovement()->AddForce(FrameLaunchForce);

  if (SlideForceTimerCount == 9)
  {
    GetWorldTimerManager().ClearTimer(SlideForceTimerHandle);
    SlideForceTimerCount = 0;
  }
  else
  {
    SlideForceTimerCount++;
  }
}

void AAPlatformerCharacter::EndSlideInAir()
{
  UKismetSystemLibrary::PrintString(this, "Attempting to end slide in air");
  if (!bIsSliding)
  {
    return;
  }
  UKismetSystemLibrary::PrintString(this, "ending slide in air");
  PCRef->SetIgnoreMoveInput(false);
  GetWorld()->GetTimerManager().ClearTimer(SlideForceTimerHandle);
  GetCharacterMovement()->GroundFriction = 6.f;
  GetCharacterMovement()->BrakingDecelerationWalking = 2048.f;

  LandedDelegate.AddDynamic(this, &AAPlatformerCharacter::BeginSlide);
}

void AAPlatformerCharacter::CheckSlideSpeed()
{
  if (!bIsSliding)
  {
    return;
  }
  UKismetSystemLibrary::PrintString(this, "checking slide speed ongoing");
  if (GetVelocity().Size() < 200.f)
  {
    EndSlideCompletely();
    Crouch();
  }
}

void AAPlatformerCharacter::EndSlideCompletely()
{
  PCRef->SetIgnoreMoveInput(false);
  GetWorld()->GetTimerManager().ClearTimer(SlideForceTimerHandle);
  GetCharacterMovement()->GroundFriction = 6.f;
  GetCharacterMovement()->BrakingDecelerationWalking = 2048.f;
  SlideHeightChange(false);
  bIsSliding = false;

  while (JumpDelegate.Contains(this, FName("EndSlideInAir")))
  {
    JumpDelegate.RemoveDynamic(this, &AAPlatformerCharacter::EndSlideInAir);
  }
  while (LandedDelegate.Contains(this, FName("BeginSlide")))
  {
    LandedDelegate.RemoveDynamic(this, &AAPlatformerCharacter::BeginSlide);
  }
}

void AAPlatformerCharacter::SlideHeightChange(bool ShouldBeLow)
{
  if (ShouldBeLow)
  {
    GetCapsuleComponent()->SetCapsuleHalfHeight(40.f);
    FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 32.f));
    SetActorLocation(GetActorLocation() - FVector(0, 0, 96.f - 40.f));
  }
  else
  {
    GetCapsuleComponent()->SetCapsuleHalfHeight(96.f);
    FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f));
  }
}

void AAPlatformerCharacter::SlideHeightChangeTrue()
{
  SlideHeightChange(true);
}

void AAPlatformerCharacter::Landing_Implementation(const FHitResult& Hit)
{
  //calc the jump height
  GetCharacterMovement()->JumpZVelocity = JumpVelocityCalc(JumpFatigueCount, JumpFatigueTimer);
  //start the recovery timer
  JumpFatigueTimerHandle = UKismetSystemLibrary::K2_SetTimer(this, "JumpFatigueTimerReset", JumpFatigueTimer, false);
  UKismetSystemLibrary::PrintString(this, UKismetStringLibrary::Concat_StrStr("This is the Jump Fatigue Timer: ", UKismetStringLibrary::Conv_DoubleToString(JumpFatigueTimer)));
  //climbing
  StopClimbing();
  ResetClimbCooldown();
  GetWorld()->GetTimerManager().ClearTimer(ClimbCooldownTimer);
}

void AAPlatformerCharacter::JumpPadBoost_Implementation(double VelocityZ)
{
  UKismetSystemLibrary::PrintString(this, "boostinig");
  LaunchCharacter(FVector(0.f, 0.f, VelocityZ), false, false);
}

void AAPlatformerCharacter::OnJumped_Implementation()
{
  UKismetSystemLibrary::PrintString(this, "timer invalidated");
  UKismetSystemLibrary::K2_ClearAndInvalidateTimerHandle(this, JumpFatigueTimerHandle);
  JumpDelegate.Broadcast();
}

void AAPlatformerCharacter::OnWalkingOffLedge_Implementation(const FVector &PreviousFloorImpactNormal, const FVector &PreviousFloorContactNormal, const FVector &PreviousLocation, float TimeDelta)
{
  if (bIsSliding)
  {
    EndSlideInAir();
  }
}

void AAPlatformerCharacter::NotifyHit(UPrimitiveComponent *MyComp, AActor *Other, UPrimitiveComponent *OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult &Hit)
{
  TMap<FName, int> TagsMap;
  //Add all of the possible tags
  TagsMap.Add("Hazard", 1);
  TagsMap.Add("Checkpoint", 2);

  for (FName Name : Other->Tags)
  {
    switch (TagsMap.FindRef(Name))
    {
    case 1:
      //Hazard
      KillPlayer();
      break;
    case 2:
      //Checkpoint
      if (Other != LastCheckpointRef)
      {
        LastCheckpointPos = UKismetMathLibrary::Add_VectorVector(Other->GetActorLocation(), FVector(100.f, 100.f, 0.f));
        LastCheckpointRef = Other;
        PCRef->CreateCheckpointReached();
        LastCheckpointLevel = GIRef->LevelNumber;
      }
      break;
    default:

      break;
    }
  }

  ReceiveHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
}

double AAPlatformerCharacter::JumpVelocityCalc(int & Count, double & Timer)
{
  double VelocityZ = GetCharacterMovement()->Velocity.Z;
  UKismetSystemLibrary::PrintString(this, UKismetStringLibrary::Conv_DoubleToString(VelocityZ));
  //velocity based jump fatigue is based on kinetic energy (except not mass)
  switch (UKismetMathLibrary::Clamp(UKismetMathLibrary::Round(pow(VelocityZ/810, 2)), 0, 5))
  {
  case 1:
    Count = UKismetMathLibrary::Clamp(Count + 2, 0, 5);
    break;
  case 2:
    Count = UKismetMathLibrary::Clamp(Count + 3, 0, 5);
    break;
  case 3:
    Count = UKismetMathLibrary::Clamp(Count + 4, 0, 5);
    break;
  case 4:
    Count = UKismetMathLibrary::Clamp(Count + 4, 0, 5);
    break;
  case 5:
    Count = UKismetMathLibrary::Clamp(Count + 5, 0, 5);
    break;
  default:
    //0
    if (Count<5)
    {
      Count++;
    }
    break;
  }
  switch (Count)
  {
  case 1:
    Timer = 0.3;
    return JumpVelocity * 0.85;
  case 2:
    Timer = 0.45;
    return JumpVelocity * 0.75;
  case 3:
  Timer = 0.55;
    return JumpVelocity * 0.6;
  case 4:
  Timer = 0.6;
    return JumpVelocity * 0.5;
  case 5:
    Timer = 0.65;
    return JumpVelocity * 0.4;
  default:
    return JumpVelocity;
  }
}
void AAPlatformerCharacter::JumpFatigueTimerReset()
{
  UKismetSystemLibrary::PrintString(this, "jump fatigue reset called");
  JumpFatigueTimer = 0.f;
  JumpFatigueCount = 0;
  GetCharacterMovement()->JumpZVelocity = JumpVelocity;
}
void AAPlatformerCharacter::RespawnPlayer_Implementation()
{
  bIsDead = false;
  GetCharacterMovement()->GravityScale = 1.f;
  EnableInput(PCRef);
  UnCrouch();
  EnableMoveInput(FHitResult());
  SlideHeightChange(false);
  PCRef->bShowMouseCursor = false;
  SetActorLocation(LastCheckpointPos);
  UKismetSystemLibrary::K2_SetTimer(this, "JumpFatigueTimerReset", 0.1, false);
}
void AAPlatformerCharacter::KillPlayer_Implementation()
{
  if (!bIsDead)
  {
  bIsDead = true;
  UCharacterMovementComponent *CharMovement = GetCharacterMovement();
  CharMovement->StopMovementImmediately();
  CharMovement->GravityScale = 0.f;
  DisableInput(PCRef);
  PCRef->CreateRespawnScreen();
  }
}
void AAPlatformerCharacter::SetHasRifle(bool bNewHasRifle)
{
	bHasRifle = bNewHasRifle;
}

bool AAPlatformerCharacter::GetHasRifle()
{
	return bHasRifle;
}

void AAPlatformerCharacter::SetShoesTrue()
{
  bHasShoes = true;
}

void AAPlatformerCharacter::SetClimbingTrue()
{
  bHasClimbingGear = true;
}

void AAPlatformerCharacter::SetSlidingTrue()
{
  bHasSlidingGear = true;
}

APCThing *AAPlatformerCharacter::GetPCRef()
{
  return PCRef;
}

UMainGI *AAPlatformerCharacter::GetGIRef()
{
  return GIRef;
}

void AAPlatformerCharacter::NotifyActorBeginOverlap(AActor* OtherActor)
{
  NumActorOverlapEventsCounter++;
	Super::NotifyActorBeginOverlap(OtherActor);

  if (Cast<IOverlapInterface>(OtherActor))
  {
    Cast<IOverlapInterface>(OtherActor)->OverlapAction(this);
  }
}

void AAPlatformerCharacter::NotifyActorEndOverlap(AActor *OtherActor)
{
  NumActorOverlapEventsCounter++;
	Super::NotifyActorEndOverlap(OtherActor);

  if (Cast<IOverlapInterface>(OtherActor))
  {
    Cast<IOverlapInterface>(OtherActor)->EndOverlapAction(this);
  }
  
}

void AAPlatformerCharacter::LoadSave()
{
  //just in case some dumb stuff happened with LoadedSave
  if (UMySaveGame* SaveGameInstance = GIRef->LoadedSave)
  {
    bHasShoes = SaveGameInstance->bHasShoes;
    bHasClimbingGear = SaveGameInstance->bHasClimbingGear;
    bHasSlidingGear = SaveGameInstance->bHasSlidingGear;
    LastCheckpointPos = SaveGameInstance->LastCheckpointPos;
    MaxSpeedDefault = SaveGameInstance->MaxSpeedDefault;
    JumpVelocity = SaveGameInstance->JumpVelocity;
    GIRef->LevelUnlocked = SaveGameInstance->LevelUnlocked;
    //if LastCheckpointPos is defined and the level numbers match, set the actor location to LastCheckpointPos
    if (LastCheckpointPos != FVector(0,0,0) && GIRef->LevelNumber == SaveGameInstance->LastCheckpointLevel)
    {
      SetActorLocation(LastCheckpointPos, false);
    }
    else
    {
      //set last checkpoint pos to spawn location
      LastCheckpointPos = GetActorLocation();
      LastCheckpointLevel = GIRef->LevelNumber;
    }
  }
  else
  {
    UKismetSystemLibrary::PrintString(this, "Failed to load save (Character)");
  }
}

void AAPlatformerCharacter::CreateSave(FString SaveSlot, int SaveIndex, FString CustomName, bool async)
{
  if (UMySaveGame* SaveGameInstance = Cast<UMySaveGame>(UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass())))
    {
      //delegate mainly used to create saved game notification
      FAsyncSaveGameToSlotDelegate SavedDelegate;
      SavedDelegate.BindUObject(PCRef, &APCThing::CreateSavedGame);

      //Set all of the save data
      SaveGameInstance->bHasShoes = bHasShoes;
      SaveGameInstance->bHasClimbingGear = bHasClimbingGear;
      SaveGameInstance->bHasSlidingGear = bHasSlidingGear;
      if (LastCheckpointRef)
      {
        SaveGameInstance->LastCheckpointPos = LastCheckpointPos;
        SaveGameInstance->LastCheckpointLevel = LastCheckpointLevel;
      }
      SaveGameInstance->MaxSpeedDefault = MaxSpeedDefault;
      SaveGameInstance->JumpVelocity = JumpVelocity;

      SaveGameInstance->CustomName = CustomName;
      if (GIRef)
      {
        SaveGameInstance->LevelUnlocked = GIRef->LevelUnlocked;
        SaveGameInstance->LevelNumber = GIRef->LevelNumber;
      }
      else
      {
        UMainGI *NewGIRef = Cast<UMainGI>(GetGameInstance());
        SaveGameInstance->LevelUnlocked = NewGIRef->LevelUnlocked;
        SaveGameInstance->LevelNumber = NewGIRef->LevelNumber;
      }
      if (async)
      {
        //asynchronous save
        UGameplayStatics::AsyncSaveGameToSlot(SaveGameInstance, SaveSlot, SaveIndex, SavedDelegate);
      }
      else
      {
        //synchronous save
        UKismetSystemLibrary::PrintString(this, "Synchronously saving"+SaveSlot+FString::FromInt(SaveIndex));
        UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveSlot, SaveIndex);
        //trigger the delegate to load when the synch load is complete
        if (GIRef)
        {
          GIRef->LoadSaveDelegate.ExecuteIfBound();
        }
        else
        {
          UKismetSystemLibrary::PrintString(this, "GIRef is gone");
        }
      }
    }
}

void AAPlatformerCharacter::TriggerPausedMenu()
{
  if (PCRef->bIsPaused)
  {
    PCRef->RemovePausedMenu();
    PCRef->bIsPaused = false;
  }
  else
  {
    PCRef->CreatePausedMenu();
    PCRef->bIsPaused = true;
  }
}
