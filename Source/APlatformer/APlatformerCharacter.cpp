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

void AAPlatformerCharacter::BufferedJump(const FHitResult &Hit)
{
  LandedDelegate.RemoveDynamic(this, &AAPlatformerCharacter::BufferedJump);
  GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AAPlatformerCharacter::JumpE);
}

void AAPlatformerCharacter::SetIsMantlingFalse()
{
  bIsMantling = false;
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
  if (bClimbCooldown)
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

  FCollisionShape MyCapsule = FCollisionShape::MakeCapsule(0.5, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()*0.95);

  bool bHit = GetWorld()->SweepMultiByChannel(HitResults, StartLocation, EndLocation, FQuat::Identity, ECC_Visibility, MyCapsule);

  if (bHit)
  {
    if (HitResults.Num() != 1)
    {
      return false;
    }
    for (auto& Hit : HitResults)
    {
      AActor* HitActor = Hit.GetActor();
      if (HitActor == nullptr)
      {
        return false;
      }
      if (UKismetMathLibrary::Abs(Hit.Normal.Z)>0.4)
      {
        return false;
      }
      if (HitActor->ActorHasTag(FName("NotClimbable")))
      {
        return false;
      }
      LastClimbNormal = Hit.Normal;
    }
    return true;
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
    FHitResult HitResult2;
    if (UKismetSystemLibrary::LineTraceSingle(this, EndVector, EndVector - FVector(0.f,0.f,300.f), ETraceTypeQuery::TraceTypeQuery1, false, EmptyActorArray, EDrawDebugTrace::ForOneFrame, HitResult2, true))
    {
      LedgePos = HitResult2.ImpactPoint;
      return true;
    }
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
    UKismetSystemLibrary::PrintString(this, "mantling");
    //unbind self from tick delegate
    TickDelegate.RemoveDynamic(this, &AAPlatformerCharacter::LedgeMantle);
    TickDelegate.RemoveDynamic(this, &AAPlatformerCharacter::LedgeHang);
    bIsOnLedge = false;
    //(don't disable wasd) until mantle is complete (also call stopclimbing)
    StopClimbing();

    bClimbCooldown = true;
    SetIgnoreStrafing(false);
    SetIgnoreForward(false);

    //FLatentActionInfo LatentInfo;
    //LatentInfo.CallbackTarget = this;
    //LatentInfo.ExecutionFunction = "MoveCapsule";
    //LatentInfo.Linkage = 0;
    //LatentInfo.UUID = 1;
    MantleTarget = LedgePos + (LastClimbNormal*-10.f) + FVector(0.f, 0.f, 69.f);
    
    MantleStart = GetActorLocation();
    //previously: UKismetSystemLibrary::MoveComponentTo(GetCapsuleComponent(), MantleTarget, FRotator::ZeroRotator, false, false, 0.5, false, EMoveComponentAction::Type::Move, LatentInfo);
    
    //play blueprint defined mantle timeline since c++ timelines are bad
    MantleTimelinePlay();

    GetWorld()->GetTimerManager().SetTimer(UnusedHandle, this, &AAPlatformerCharacter::SetClimbCooldownFalse, 0.8, false);

    //Is mantling boolean stuff
    bIsMantling = true;
    GetWorld()->GetTimerManager().SetTimer(SetMantlingFalseHandle, this, &AAPlatformerCharacter::SetIsMantlingFalse, 0.5, false);

    //superglide true in the last 0.1s of mantle if sliding gear
    if (!bHasSlidingGear || !bIsSprinting) {return;}
    GetWorld()->GetTimerManager().SetTimer(SuperGlideWindowStartHandle, this, &AAPlatformerCharacter::SetCanSuperGlideTrue, 0.4, false);
  }
}

void AAPlatformerCharacter::UpdatePosition(float Value)
{
  FVector NewLocation = FMath::Lerp(MantleStart, MantleTarget, Value);
  SetActorLocation(NewLocation);
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

void AAPlatformerCharacter::GravShift(EGravShiftDirection Direction)
{
  
}

void AAPlatformerCharacter::DoubleJump()
{
  //instantly apply 1/3 jump velocity (cancel downward velocity)
  GetCharacterMovement()->Velocity.Z = FMath::Max<FVector::FReal>(GetCharacterMovement()->Velocity.Z, JumpVelocity / 2);
  bIsDoubleJumpHeld = true;
  //start the looping timer to apply double jump force, up to full extra jump velocity
  GetWorld()->GetTimerManager().SetTimer(DoubleJumpHandle, this, &AAPlatformerCharacter::ApplyDoubleJumpForce, 0.02, true);
}

void AAPlatformerCharacter::EndDoubleJump()
{
  bIsDoubleJumpHeld = false;
}

void AAPlatformerCharacter::ApplyDoubleJumpForce()
{
  if (!bIsDoubleJumpHeld)
  {
    GetWorld()->GetTimerManager().ClearTimer(DoubleJumpHandle);
  }
  if (DoubleJumpForceCounter > 0 && DoubleJumpForceCounter != 10)
  {
    GetCharacterMovement()->AddImpulse(FVector(0.f, 0.f, JumpVelocity * 0.075), true);
    DoubleJumpForceCounter++;
  }
  else if (DoubleJumpForceCounter == 0)
  {
    if (GetCharacterMovement()->Velocity.Z < 0.f)
    {
      GetCharacterMovement()->Velocity.Z = 0.f;
    }
    GetCharacterMovement()->AddImpulse(FVector(0.f, 0.f, JumpVelocity * 0.075), true);
    DoubleJumpForceCounter++;
  }
  else
  {
    DoubleJumpForceCounter = 0;
    GetWorld()->GetTimerManager().ClearTimer(DoubleJumpHandle);
  }
}

void AAPlatformerCharacter::DoubleJumpBootsPickup()
{
  DoubleJumpCount = 1;
  bHasDoubleJumpBoots = true;
  PCRef->CreateDoubleJump();
}

void AAPlatformerCharacter::DoubleJumpOrbOverlap()
{
  DoubleJumpCount++;
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
  DeltaTime = DeltaSeconds;
  TickDelegate.Broadcast();
}

//////////////////////////////////////////////////////////////////////////// Input

void AAPlatformerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AAPlatformerCharacter::JumpE);
    //end double jump
    EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AAPlatformerCharacter::EndDoubleJump);
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
  LastMoveInput = MovementVector;
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

void AAPlatformerCharacter::JumpE()
{
  Jump();
  if (bSuperGlideSlideQueued)
  {
    bSuperGlideSlideQueued = false;
    UKismetSystemLibrary::PrintString(this, "Superglide successful");
    SuperGlide();
  }
  if (bCoyoteTime)
  {
    bCoyoteTime = false;
    GetCharacterMovement()->Velocity.Z = FMath::Max<FVector::FReal>(GetCharacterMovement()->Velocity.Z, GetCharacterMovement()->JumpZVelocity);
		GetCharacterMovement()->SetMovementMode(MOVE_Falling);
    //copied from checkjumpinput
    JumpCurrentCount++;
		JumpForceTimeRemaining = GetJumpMaxHoldTime();
		OnJumped();
  }
  //double jumping
  else if (GetCharacterMovement()->IsFalling())
  {
    //to check if close to the ground
    TArray<AActor*> EmptyActorArray;
    FHitResult HitResult;
    if (UKismetSystemLibrary::LineTraceSingle(this, GetActorLocation(), GetActorLocation() + GetActorUpVector() * -126.f, ETraceTypeQuery::TraceTypeQuery1, false, EmptyActorArray, EDrawDebugTrace::ForOneFrame, HitResult, true))
    {
      if (!LandedDelegate.Contains(this, FName("BufferedJump")))
      {
        LandedDelegate.AddDynamic(this, &AAPlatformerCharacter::BufferedJump);
      }
      return;
    }
    if (DoubleJumpCount > 0 && !bCanWallJump)
    {
      DoubleJumpCount--;
      DoubleJump();
    }
  }
  //wall jumping (if reqs met)
  WallJump();
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
  if (bCanSuperGlide)
  {
    bCanSuperGlide = false;
    bSuperGlideSlideQueued = true;
    UKismetSystemLibrary::PrintString(this, "Superglide slide queued");
    GetWorld()->GetTimerManager().SetTimer(SuperglideInitiateHandle, this, &AAPlatformerCharacter::SetSuperGlideSlideQueuedFalse, 0.03, false);
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
  FVector LastVelocityVector = GetMovementComponent()->Velocity;
  LastVelocityVector.Z = 0;

  if (LastVelocityVector.Size() > 75.f && bIsSprinting)
  {
    bIsSliding = true;
    FTimerHandle SlideHeightChangeTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(SlideHeightChangeTimerHandle, this, &AAPlatformerCharacter::SlideHeightChangeTrue, 0.01, false);
    PCRef->SetIgnoreMoveInput(true);
    //turn ground friction lower
    GetCharacterMovement()->GroundFriction = 0.25;
    GetCharacterMovement()->BrakingDecelerationWalking = 300.f;

    //boost the character in the direction of SlideForceVector over 0.2s (can cancel with EndSlideCompletely or EndSlideInAir)
    if (GetWorld()->GetTimerManager().GetTimerRemaining(SlideCooldownTimer)<=0.f && LastVelocityVector.Size() < 1100.f)
    {
      //set the slideforcevector
      FHitResult HitResult;
      FVector StartLocation = GetActorLocation();
      FVector EndLocation = StartLocation - FVector(0, 0, 150.f);
      GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility);
      FVector GroundNormal = HitResult.Normal;
      UKismetSystemLibrary::PrintString(this, GroundNormal.ToString());
      SlideForce.SlideForceVector = LastVelocityVector - (FVector::DotProduct(LastVelocityVector, GroundNormal) * GroundNormal);
      SlideForce.SlideForceVector.Normalize();
      
      UKismetSystemLibrary::PrintString(this, "boosting with this vector: "+ SlideForce.SlideForceVector.ToString());
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
  if (SlideForceTimerCount == 0)
  {
  FVector SlideForceVectorWithoutZ = SlideForce.SlideForceVector;
  int PosOrNegAngle = UKismetMathLibrary::SignOfFloat(SlideForceVectorWithoutZ.Z) * -1;
  SlideForceVectorWithoutZ.Z = 0;
  SlideForceVectorWithoutZ.Normalize();
  FVector FrameLaunchForce = SlideForceVectorWithoutZ * 2750000.f * 0.02f / 0.2f;
  double SlideForceAngle = UKismetMathLibrary::Acos(FVector::DotProduct(SlideForce.SlideForceVector, SlideForceVectorWithoutZ)) * PosOrNegAngle;
  //multiplier is based on ln(angle+1)*0.69 + 1 where it's angle between SlideForceAngle and equivalent vector on ground
  double Multiplier = UKismetMathLibrary::ClampAngle((UKismetMathLibrary::Loge(SlideForceAngle+1.f) * 0.69) + 1.f, 0.f, 1.4);
  if (PosOrNegAngle == -1)
  {
    Multiplier *= 0.3;
  }
  FrameLaunchForce *= Multiplier;
  SlideForce.FrameLaunchForce = FrameLaunchForce;
  }
  GetCharacterMovement()->AddForce(SlideForce.FrameLaunchForce);

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
  SlideForceTimerCount = 0;
  GetCharacterMovement()->GroundFriction = 6.f;
  GetCharacterMovement()->BrakingDecelerationWalking = 2048.f;

  LandedDelegate.AddDynamic(this, &AAPlatformerCharacter::BeginSlide);
  TickDelegate.AddDynamic(this, &AAPlatformerCharacter::WallJumpCompute);
}

void AAPlatformerCharacter::CheckSlideSpeed()
{
  if (!bIsSliding)
  {
    return;
  }
  if (GetMovementComponent()->Velocity.Size() < 200.f)
  {
    EndSlideCompletely();
    Crouch();
  }
}

void AAPlatformerCharacter::EndSlideCompletely()
{
  PCRef->SetIgnoreMoveInput(false);
  GetWorld()->GetTimerManager().ClearTimer(SlideForceTimerHandle);
  SlideForceTimerCount = 0;
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

void AAPlatformerCharacter::WallJump()
{
  if (!bCanWallJump)
  {
    return;
  }
  if (bIsClimbing)
  {
    return;
  }
  if (bCanSuperGlide || bSuperGlideSlideQueued)
  {
    return;
  }
  if (!WallJumpDetectStrict())
  {
    return;
  }
  UKismetSystemLibrary::PrintString(this, "trying to wall jump");
  //calc xy direction
  FVector LaunchVector = WallJump0.PlayerVelocityXY - (WallJump0.WallNormal * (2 * FVector::DotProduct(WallJump0.PlayerVelocityXY, WallJump0.WallNormal)));
  //max out xy at 3000 magnitude
  if (LaunchVector.Size() > 700.f)
  {
    LaunchVector.Normalize();
    LaunchVector *= 700.f;
  }
  //the wall could be slanted up or down slightly so += is used
  LaunchVector.Z += 400.f;
  //UKismetMathLibrary::ClampAngle((900.f + WallJump0.PlayerVelocity.Z), 150.f, 1100.f)
  //can't wall jump if moving too fast
  if (GetMovementComponent()->Velocity.Size()<600.f)
  {
    UKismetSystemLibrary::PrintString(this, "wall jump successful");
    LaunchCharacter(LaunchVector, false, false);
    //double jump cooldown
    bDoubleJumpCooldown = true;
    GetWorld()->GetTimerManager().SetTimer(DoubleJumpCooldownHandle, this, &AAPlatformerCharacter::SetDoubleJumpCooldownFalse, 0.1, false);
  }
  bCanWallJump = false;
  bHadWallJumpOpportunity = true;
  if (!LandedDelegate.Contains(this, FName("SetWallJumpOpportunityFalse")))
  {
    LandedDelegate.AddDynamic(this, &AAPlatformerCharacter::SetWallJumpOpportunityFalse);
  }
}

bool AAPlatformerCharacter::WallJumpDetect(bool WriteToStruct)
{
  //copied from forwardwalldetect, but with wider radius for more leeway
  //also only works if capsule hits one actor, I'm not doing walljump off of multiple walls at once
  TArray<FHitResult> HitResults;
  FVector StartLocation = GetActorLocation();
  FVector EndLocation = StartLocation + (GetActorForwardVector() * 20.f);

  FCollisionShape MyCapsule = FCollisionShape::MakeCapsule(39.f, 50.f);

  bool bHit = GetWorld()->SweepMultiByChannel(HitResults, StartLocation, EndLocation, FQuat::Identity, ECC_Visibility, MyCapsule);

  if (bHit)
  {
    if (HitResults.Num() != 1)
    {
      return false;
    }
    for (auto& Hit : HitResults)
    {
      AActor* HitActor = Hit.GetActor();
      if (HitActor == nullptr)
      {
        return false;
      }
      if (HitActor->ActorHasTag(FName("NotClimbable")))
      {
        return false;
      }
      if (UKismetMathLibrary::Abs(Hit.Normal.Z)>0.4)
      {
        return false;
      }
      if (WriteToStruct)
      {
      WallJump0.WallNormal = Hit.Normal;
      }
    }
    return true;
  }
  return false;
}

bool AAPlatformerCharacter::WallJumpDetectStrict()
{
  TArray<FHitResult> HitResults;
  FVector StartLocation = GetActorLocation();
  FVector EndLocation = StartLocation + (GetActorForwardVector() * 32.5);

  FCollisionShape MyCapsule = FCollisionShape::MakeCapsule(10.f, 90.f);

  bool bHit = GetWorld()->SweepMultiByChannel(HitResults, StartLocation, EndLocation, FQuat::Identity, ECC_Visibility, MyCapsule);

  if (bHit)
  {
    if (HitResults.Num() != 1)
    {
      return false;
    }
    for (auto& Hit : HitResults)
    {
      AActor* HitActor = Hit.GetActor();
      if (HitActor == nullptr)
      {
        return false;
      }
      if (HitActor->ActorHasTag(FName("NotClimbable")))
      {
        return false;
      }
      if (UKismetMathLibrary::Abs(Hit.Normal.Z)>0.4)
      {
        return false;
      }
    }
    return true;
  }
  return false;
}

void AAPlatformerCharacter::WallJumpCompute()
{
  if (bCanWallJump)
  {
    if (!WallJumpDetect())
    {
      bHadWallJumpOpportunity = true;
      if (!LandedDelegate.Contains(this, FName("SetWallJumpOpportunityFalse")))
      {
        LandedDelegate.AddDynamic(this, &AAPlatformerCharacter::SetWallJumpOpportunityFalse);
      }
      GetWorld()->GetTimerManager().ClearTimer(WallJumpWindowHandle);
      if (TickDelegate.Contains(this, FName("WallJumpCompute")))
      {
        TickDelegate.RemoveDynamic(this, &AAPlatformerCharacter::WallJumpCompute);
      }
      bCanWallJump = false;
    }
    return;
  }
  if (bHadWallJumpOpportunity)
  {
    return;
  }
  if (WallJumpDetect(true))
  {
    UKismetSystemLibrary::PrintString(this, "WallJump opportunity detected and starting timer");
    //set vector data
    WallJump0.PlayerVelocity = GetMovementComponent()->Velocity;
    WallJump0.PlayerVelocityXY = FVector(GetMovementComponent()->Velocity.X, GetMovementComponent()->Velocity.Y, 0.f);
    //make player cling to wall with looping applying force (over 0.1s)
    GetWorld()->GetTimerManager().SetTimer(WallJump0.WallJumpSlowDownHandle, this, &AAPlatformerCharacter::WallJumpSlowDown, 0.01, true);
    bCanWallJump = true;
    //start walljumpwindow
    GetWorld()->GetTimerManager().SetTimer(WallJumpWindowHandle, this, &AAPlatformerCharacter::UnbindWallJumpDetect, 0.2, false);
  } 
}

void AAPlatformerCharacter::WallJumpSlowDown()
{
  if (WallJump0.SlowTickCount == 11)
  {
    WallJump0.SlowTickCount = 0;
    GetWorld()->GetTimerManager().ClearTimer(WallJump0.WallJumpSlowDownHandle);
    return;
  }
  UKismetSystemLibrary::PrintString(this, "slowing down for wall jump");
  //rotated pi/2 ccw in xy direction from normal vector
  FVector ParallelVector = FVector(WallJump0.WallNormal.Y * -1, WallJump0.WallNormal.X, WallJump0.WallNormal.Z);
  FVector SlowVector = ParallelVector * (-1 * FVector::DotProduct(ParallelVector, WallJump0.PlayerVelocity));
  LaunchCharacter(SlowVector * 0.11, false, false);
  WallJump0.SlowTickCount++;
}

void AAPlatformerCharacter::SetWallJumpOpportunityFalse(const FHitResult &Hit)
{
  bHadWallJumpOpportunity = false;
}

void AAPlatformerCharacter::UnbindWallJumpDetect()
{
  if (TickDelegate.Contains(this, FName("WallJumpCompute")))
  {
    TickDelegate.RemoveDynamic(this, &AAPlatformerCharacter::WallJumpCompute);
  }
  bCanWallJump = false;
}

void AAPlatformerCharacter::SuperGlide()
{
  MantleTimelineStop();
  SetIsMantlingFalse();
  GetWorld()->GetTimerManager().ClearTimer(SetMantlingFalseHandle);
  //rotated pi/2 ccw in xy direction from lastclimbnormal
  //also superglide z velo is fixed to jumpvelo boost
  //this isn't even the rightward vector but I'm not making this a physics z mech problem
  FVector LastClimbRight = FVector(LastClimbNormal.Y * -1, LastClimbNormal.X, 0.f);
  LastClimbRight.Normalize();
  FVector LastClimbForward = LastClimbNormal * -1.f;
  LastClimbForward.Z = 0.f;
  LastClimbForward.Normalize();

  FVector SuperGlideVector = ((LastMoveInput.X * -1) * LastClimbRight) + (LastMoveInput.Y * LastClimbForward);
  SuperGlideVector.Normalize();
  SuperGlideVector *= MaxSlideSpeed;
  SuperGlideVector.Z = JumpVelocity;
  LaunchCharacter(SuperGlideVector, false, false);
}

void AAPlatformerCharacter::SetCanSuperGlideTrue()
{
  UKismetSystemLibrary::PrintString(this, "Superglide possible");
  bCanSuperGlide = true;
  GetWorld()->GetTimerManager().SetTimer(SuperGlideWindowEndHandle, this, &AAPlatformerCharacter::SetCanSuperGlideFalse, 0.05, false);
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
  //wall jumping
  bCanWallJump = false;
  UnbindWallJumpDetect();
  //Double Jumping
  if (bHasDoubleJumpBoots)
  {
    DoubleJumpCount = 1;
  }
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

  //double jump cooldown
  bDoubleJumpCooldown = true;
  GetWorld()->GetTimerManager().SetTimer(DoubleJumpCooldownHandle, this, &AAPlatformerCharacter::SetDoubleJumpCooldownFalse, 0.1, false);

  JumpDelegate.Broadcast();
}

void AAPlatformerCharacter::OnWalkingOffLedge_Implementation(const FVector &PreviousFloorImpactNormal, const FVector &PreviousFloorContactNormal, const FVector &PreviousLocation, float TimeDelta)
{
  if (bIsSliding)
  {
    EndSlideInAir();
  }
  //coyote time start
  bCoyoteTime = true;
  //really long rn to test
  GetWorld()->GetTimerManager().SetTimer(CoyoteTimeHandle, this, &AAPlatformerCharacter::SetCoyoteTimeFalse, 0.1, false);
}

void AAPlatformerCharacter::NotifyHit(UPrimitiveComponent *MyComp, AActor *Other, UPrimitiveComponent *OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult &Hit)
{
  TMap<FName, int> TagsMap;
  //Add all of the possible tags
  TagsMap.Add(FName("Hazard"), 1);
  TagsMap.Add(FName("Checkpoint"), 2);

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
    Timer = 0.1;
    return JumpVelocity * 0.85;
  case 2:
    Timer = 0.15;
    return JumpVelocity * 0.75;
  case 3:
  Timer = 0.3;
    return JumpVelocity * 0.6;
  case 4:
  Timer = 0.4;
    return JumpVelocity * 0.5;
  case 5:
    Timer = 0.5;
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
