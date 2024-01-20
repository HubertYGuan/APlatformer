#include "APlatformerCharacter.h"
// Copyright Epic Games, Inc. All Rights Reserved.
#include "APlatformerProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
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


//called on landing
void AAPlatformerCharacter::WallRunReset()
{
  WallRun.VelocityLastTick = FVector();
  WallRun.bIsRunning = false;
  WallRun.Height = -69420.f;
  WallRun.Wall = nullptr;
  WallRun.InVector = FVector();
  WallRun.OutVector = FVector();
  WallRun.LeanInRotator = FRotator();
  WallRun.SameWallCooldown = false;
  WallRun.bDoubleJumped = false;
  WallRun.bCancellingAnim = false;
  WallRunHandleReset();
  WallRunCoyoteClear();
  UpdateLeanIn(0.f);
  UKismetSystemLibrary::PrintString(this, "wall run reset successful");
  UKismetSystemLibrary::PrintString(this, FString::SanitizeFloat(WallRun.Height));
}

void AAPlatformerCharacter::WallRunSetIsRunning(bool Value)
{
  WallRun.bIsRunning = Value;
}

void AAPlatformerCharacter::WallRunSetLeaning(bool Value)
{
  WallRun.bLeaning = Value;
}

void AAPlatformerCharacter::WallRunSetSameWallCooldown(bool Value)
{
  WallRun.SameWallCooldown = Value;
}

void AAPlatformerCharacter::WallRunSetCancellingAnim(bool Value)
{
  WallRun.bCancellingAnim = Value;
}

void AAPlatformerCharacter::EnableMoveInput(const FHitResult &Hit)
{
  PCRef->SetIgnoreMoveInput(false);
  LandedDelegate.RemoveDynamic(this, &AAPlatformerCharacter::EnableMoveInput);
}

void AAPlatformerCharacter::MoveInputActionMethod(const FInputActionValue & Value)
{
  WallRunCompute(Value);
  //some checks are doubled but don't worry about that
  Climb(Value);
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
  if (WallRun.bIsRunning || WallRun.bLeaning)
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
  //stopclimbing will only be called on forward input end
  if (LastMoveInput.Y > 0)
  {
    StopClimbing();
  }
  //also sets move released true
  bIsMoveInputReleased = true;
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
    for (auto& Hit : HitResults)
    {
      AActor* HitActor = Hit.GetActor();
      if (HitActor == nullptr)
      {
        UKismetSystemLibrary::PrintString(this, "nullptr false");
        return false;
      }
      //check if it's a trigger
      if (Cast<UShapeComponent>(Hit.GetComponent()))
      {
        continue;
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

void AAPlatformerCharacter::HoverCompute()
{
  //amazing efficiency::vvvv
  HoverDetect();
}

bool AAPlatformerCharacter::HoverDetect()
{
  FHitResult HitResult;
  FVector StartLocation = GetFirstPersonCameraComponent()->GetComponentLocation();
  FVector EndLocation = StartLocation + (GetFirstPersonCameraComponent()->GetForwardVector() * 500.f);

  GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility);

  if (!HitResult.bBlockingHit)
  {
    //either stopped hovering or was never hovering in the first place
    if (!HoveredInteractable.GetObject())
    {
      HoveredPosition = FVector();
      bIsHovering = false;
      return false;
    }
    else if (HoveredInteractable.GetInterface())
    {
      UKismetSystemLibrary::PrintString(this, "ending hover");
    HoveredInteractable.GetInterface()->EndHover(this);
    HoveredInteractable.SetObject(nullptr);
    HoveredInteractable.SetInterface(nullptr);
    HoveredPosition = FVector();
    bIsHovering = false;
    return false;
    }
  }
  AActor* HitActor = HitResult.GetActor();

  if (IInteractInterface *Interactable = Cast<IInteractInterface>(HitActor))
  {
    //this is the old interface, not the new one that we're hovering over (or maybe it is the same)
    IInteractInterface *Interface = HoveredInteractable.GetInterface();
    AActor *OldActor = Cast<AActor>(HoveredInteractable.GetObject());
  if (!Interactable->GetOnCooldown())
  {
    if (OldActor == nullptr)
    {
      UKismetSystemLibrary::PrintString(this, "hover actor found");
      //start hovering on something after not hovering
      HoveredInteractable.SetObject(HitActor);
      HoveredInteractable.SetInterface(Interactable);
      Interactable->StartHover(this);
      HoveredPosition = HitActor->GetActorLocation();
      bIsHovering = true;
      return true;
    }
    else if (OldActor == HitActor)
    {
      UKismetSystemLibrary::PrintString(this, "continuing to hover");
      //continue hovering on the same thing if not on cooldown
    }
    else
    {
      //transfer hovering from old to new
      UKismetSystemLibrary::PrintString(this, "ending hover by switching");
      Interface->EndHover(this);
      HoveredInteractable.SetObject(HitActor);
      HoveredInteractable.SetInterface(Interactable);
      Interactable->StartHover(this);
      HoveredPosition = HitActor->GetActorLocation();
      bIsHovering = true;
      return true;
    }
  }
    else
  {
    if (bIsHovering)
    {
      //if the newest thing is on cooldown, stop hovering
      UKismetSystemLibrary::PrintString(this, "nulling object, on cooldown");
      Interactable->EndHover(this);
      HoveredInteractable.SetObject(nullptr);
      HoveredInteractable.SetInterface(nullptr);
      HoveredPosition = FVector();
      bIsHovering = false;
      return false;
    }
  }
  }
  return false;
}

void AAPlatformerCharacter::UpdatePosition(float Value)
{
  FVector NewLocation = FMath::Lerp(MantleStart, MantleTarget, Value);
  SetActorLocation(NewLocation);
}

FVector AAPlatformerCharacter::VectorLerp(FVector A, FVector B, double Alpha)
{
  return FVector(UKismetMathLibrary::Lerp(A.X, B.X, Alpha), UKismetMathLibrary::Lerp(A.Y, B.Y, Alpha), UKismetMathLibrary::Lerp(A.Z, B.Z, Alpha));
}

FRotator AAPlatformerCharacter::RotatorLerp(FRotator A, FRotator B, double Alpha)
{
  double BPitch = B.Pitch;
  double BYaw = B.Yaw;
  double BRoll = B.Roll;
  double APitch = A.Pitch;
  double AYaw = A.Yaw;
  double ARoll = A.Roll;
  if (BPitch>180.f)
  {
    BPitch-=360.f;
  }
  if (BYaw>180.f)
  {
    BYaw-=360.f;
  }
  if (BRoll>180.f)
  {
    BRoll-=360.f;
  }
  if (APitch>180.f)
  {
    APitch-=360.f;
  }
  if (AYaw>180.f)
  {
    AYaw-=360.f;
  }
  if (ARoll>180.f)
  {
    ARoll-=360.f;
  }

  return FRotator(UKismetMathLibrary::Lerp(APitch, BPitch, Alpha), UKismetMathLibrary::Lerp(AYaw, BYaw, Alpha), UKismetMathLibrary::Lerp(ARoll, BRoll, Alpha));
}

void AAPlatformerCharacter::UpdateCameraTransform(float Value, bool Returning)
{
  if (Returning)
  {
    FirstPersonCameraComponent->SetWorldLocation(VectorLerp(CameraPanReturnLocation, CameraPanLocation, Value), false);
    PCRef->SetControlRotation(RotatorLerp(CameraPanReturnRotation, CameraPanRotation, Value));
  }
  else
  {
    FirstPersonCameraComponent->SetWorldLocation(VectorLerp(CameraPanInitLocation, CameraPanLocation, Value), false);
    PCRef->SetControlRotation(RotatorLerp(CameraPanInitRotation, CameraPanRotation, Value));
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
  GetWorld()->GetTimerManager().SetTimer(OnJumpedHandle, this, &AAPlatformerCharacter::ResetClimbCooldown, 0.4, false);

  WallRun.bDoubleJumped = true;
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

void AAPlatformerCharacter::WallRunHandleReset()
{
  GetWorld()->GetTimerManager().ClearTimer(WallRun.Handle);
}

void AAPlatformerCharacter::WallRunHandleStart()
{
  GetWorld()->GetTimerManager().SetTimer(WallRun.Handle, this, &AAPlatformerCharacter::ResetSlideCooldown, 1.75, false);
}

void AAPlatformerCharacter::WallRunCoyoteClear()
{
  GetWorld()->GetTimerManager().ClearTimer(WallRun.CoyoteHandle);
}

void AAPlatformerCharacter::WallRunCompute(const FInputActionValue &Value)
{
  if (!WallRun.bCanRun)
  {
    return;
  }
  if (!GetCharacterMovement()->IsFalling())
  {
    return;
  }
  if (!bIsSprinting)
  {
    return;
  }
  GetWorld()->GetTimerManager().ClearTimer(LetGoHandle);

  bool bOnWall = WallRunDetect(false, true);

  //check the angle of forwardvector compared to normal (will climb and stop wall running if too high)
  if (UKismetMathLibrary::Abs((GetActorForwardVector().Dot(WallRun.Normal)))>0.9)
  {
    UKismetSystemLibrary::PrintString(this, "wall normal angle too high c++");
    if (WallRun.bIsRunning || WallRun.bLeaning)
    {
    WallRunCancel();
    }
    return;
  }

  if (!WallRun.bIsRunning)
  {
    if (!bOnWall)
    {
      if (WallRun.bLeaning)
      {
        UKismetSystemLibrary::PrintString(this, "leaning cancel c++");
        WallRunDetect(true);
        //cancel lean in
        WallRunStartTimelineStop();
      }
      //wallbump
      if (GetCharacterMovement()->bWantsToCrouch)
      {
        UKismetSystemLibrary::PrintString(this, "wallbump c++");
        WallRunDetect(true);
        WallBump();
      }
    }
    else if (WallRunDetect() && !WallRun.bLeaning)
    {
      UKismetSystemLibrary::PrintString(this, "starting wall run timeline c__");
      //start lean in
      WallRunStartTimelineBegin();
      //do some other stuff related to double jumping
      WallRunStart();
    }
  }
  else
  //is running
  {
    //case that already running but no longer on wall
    if (!bOnWall)
    {
      UKismetSystemLibrary::PrintString(this, "cancel no longer on wall c++");
      WallRunCancel();
      return;
    }
    UKismetSystemLibrary::PrintString(this, "slow or boost c++");
    FVector VelocityXY = GetVelocity();
    VelocityXY.Z = 0.f;
    //case that you can keep running, but need to slow down
    if (VelocityXY.Size()>WallRun.MaxSpeedXY)
    {
      WallRunSlow();
    }
    else
    {
      //need speed up
      WallRunBoost();
    }
  }
}

bool AAPlatformerCharacter::WallRunDetect(bool ForWallRunStop, bool OnlyCollisionCheck)
{
  UKismetSystemLibrary::PrintString(this, "wallrundetect");
  //I h8 dry programming
  TArray<FHitResult> HitResults;
  FVector StartLocation = GetActorLocation();

  FCollisionShape MyCapsule = FCollisionShape::MakeCapsule(45.5, 66.f);

  bool bHit = GetWorld()->SweepMultiByChannel(HitResults, StartLocation, StartLocation+FVector(0, 0, -25.f), FQuat::Identity, ECC_Visibility, MyCapsule);

  if (bHit)
  {
    for (auto& Hit : HitResults)
    {
      AActor* HitActor = Hit.GetActor();
      UKismetSystemLibrary::PrintString(this, HitActor->GetName());
      if (WallRun.Wall != nullptr)
      {
      UKismetSystemLibrary::PrintString(this, "name of samewall: " + WallRun.Wall->GetName());
      }
      if (HitActor == nullptr)
      {
        UKismetSystemLibrary::PrintString(this, "nullptr false");
        return false;
      }
      //check if it's a trigger
      if (Cast<UShapeComponent>(Hit.GetComponent()))
      {
        continue;
      }
      if (HitActor->ActorHasTag(FName("NotClimbable")))
      {
        UKismetSystemLibrary::PrintString(this, "not climbable false");
        return false;
      }

      //check if the wall is too slanted to wall run
      if (UKismetMathLibrary::Abs(Hit.ImpactNormal.Z) > 0.3f && !OnlyCollisionCheck)
      {
        UKismetSystemLibrary::PrintString(this, "normal z value false");
        return false;
      }

      if (OnlyCollisionCheck)
      {
        UKismetSystemLibrary::PrintString(this, "wallrundetect collision only true");
        WallRun.Normal = Hit.ImpactNormal;
        return true;
      }
      //height check, can only wall jump on same wall if lower (can freely if different walls)
      UKismetSystemLibrary::PrintString(this, "isrunning: " + FString::FromInt(WallRun.bIsRunning) + " does hitactor == the wall: "+ FString::FromInt(HitActor == WallRun.Wall));
      //hitactor not equalling the wall bruh, seems like wallrunwall is still nullptr
      if (!WallRun.bIsRunning && ((GetActorLocation().Z >= WallRun.Height - 50.f && WallRun.Height != -69420.f) || WallRun.SameWallCooldown) && HitActor == WallRun.Wall)
      {
        UKismetSystemLibrary::PrintString(this, "height too high or not reset and same wall false");
        return false;
      }
      if (ForWallRunStop)
      {
        WallRun.Normal = Hit.ImpactNormal;
        WallRun.Height = GetActorLocation().Z;
        WallRun.Wall = HitActor;
      }
      else
      {
        WallRun.Normal = Hit.ImpactNormal;
        WallRun.InVector = WallRun.VelocityLastTick;
      }
      UKismetSystemLibrary::PrintString(this, "wallrundetect true");
      return true;
    }
  }
  UKismetSystemLibrary::PrintString(this, "no hit false");
  return false;
}

void AAPlatformerCharacter::SameWallDetect()
{
  TArray<FHitResult> HitResults;
  FVector StartLocation = GetActorLocation();

  //you need to be pretty clear of the wall to clear the samewallcooldown
  FCollisionShape MyCapsule = FCollisionShape::MakeCapsule(50.5, 66.f);

  bool bHit = GetWorld()->SweepMultiByChannel(HitResults, StartLocation, StartLocation+FVector(0, 0, -25.f), FQuat::Identity, ECC_Visibility, MyCapsule);

  if (bHit)
  {
    for (auto& Hit : HitResults)
    {
      AActor* HitActor = Hit.GetActor();
      if (HitActor == nullptr)
      {
        UKismetSystemLibrary::PrintString(this, "nullptr error");
        return;
      }
      //check if it's the hitactor
      if (HitActor == WallRun.Wall)
      {
        return;
      }
    }
  }
  WallRun.SameWallCooldown = false;
  TickDelegate.RemoveDynamic(this, &AAPlatformerCharacter::SameWallDetect);
}

void AAPlatformerCharacter::BindSameWallDetect()
{
  if (!TickDelegate.Contains(this, FName("SameWallDetect")))
  {
    TickDelegate.AddDynamic(this, &AAPlatformerCharacter::SameWallDetect);
  }
}

void AAPlatformerCharacter::WallRunStart()
{
  if (WallRun.bDoubleJumped)
  {
    DoubleJumpCount++;
    WallRun.bDoubleJumped = false;
  }
}

void AAPlatformerCharacter::WallRunCancel()
{
  WallRunDetect(true);
  WallRunStartTimelineStop();
  WallRun.bGetVelocity = true;
  GetWorld()->GetTimerManager().SetTimer(WallRun.CoyoteHandle, this, &AAPlatformerCharacter::ResetSlideCooldown, 0.1, false);
}

void AAPlatformerCharacter::WallRunLetGo()
{
  if (!WallRun.bIsRunning && !WallRun.bLeaning)
  {
    return;
  }
  else if (WallRun.bLeaning)
  {
    WallRunCancel();
  }
  GetWorld()->GetTimerManager().SetTimer(LetGoHandle, this, &AAPlatformerCharacter::WallRunCancel, 0.5, false);
}

void AAPlatformerCharacter::WallRunBoost()
{
  //unit vector
  FVector LaunchVector = (WallRun.ForwardVector * WallRun.ForwardVector.Dot(GetVelocity())).GetSafeNormal();
  if ((GetVelocity() + LaunchVector).Size() < 1000.f)
  {
    LaunchCharacter(LaunchVector*30.f, false, false);
  }
  else
  {
    LaunchCharacter(LaunchVector*1000.f, true, true);
  }
}

void AAPlatformerCharacter::WallRunSlow()
{
  //unit vector
  FVector LaunchVector = (WallRun.ForwardVector * WallRun.ForwardVector.Dot(GetVelocity())).GetSafeNormal();
  if ((GetVelocity() + LaunchVector).Size() > 1000.f)
  {
    LaunchCharacter(LaunchVector*-30.f, false, false);
  }
  else
  {
    LaunchCharacter(LaunchVector*1000.f, true, true);
  }
}

void AAPlatformerCharacter::WallRunJump()
{
  //crouch kicking not working is a feature not a bug
  UKismetSystemLibrary::PrintString(this, "calling wallrunjump");
  //one factor to determine magnitude of outvector (additive):
  float TimeOnWall = GetWorld()->GetTimerManager().GetTimerElapsed(WallRun.Handle);
  //60 tps ticks on wall rounded up (i couldn't find the ceiling function)
  int TicksOnWall = UKismetMathLibrary::FFloor((TimeOnWall / 60.f)+1.f);
  //this velocity should be parallel to the wall and perpendicular to wall normal (if I didn't break wall run speed gain)
  FVector2D VelocityXY = FVector2D(GetVelocity().X, GetVelocity().Y);

  float NormalMultiplier = 500.f;
  
  //reflect like the wall jump (can be modified if end boosting i.e. coyote time)
  WallRun.OutVector = FVector(VelocityXY, 0.f);
  //check for a wall kick and set outvector's magnitude appropriately
  if (TicksOnWall <= 5)
  {
    WallRun.OutVector *= 1000.f / WallRun.OutVector.Size();
    //additional boost based on how soon you wall kick
    FVector TimeOnWallAddVector = WallRun.OutVector.GetSafeNormal();
    switch (TicksOnWall)
    {
    case 1:
      //firstie, max addition
      TimeOnWallAddVector *= 200.f;
      break;
    case 2:
      TimeOnWallAddVector *= 150.f;
      break;
    case 3:
      TimeOnWallAddVector *= 125.f;
      break;
    case 4:
      TimeOnWallAddVector *= 100.f;
      break;
    case 5:
      TimeOnWallAddVector *= 75.f;
      break;
    default:
      //somehow less than one tick on the wall
      break;
    }
  WallRun.OutVector += TimeOnWallAddVector;
  }
  //add the boost away from the wall
  WallRun.OutVector += WallRun.Normal * NormalMultiplier;

  float OutVectorMagnitude = WallRun.OutVector.Size();

  //accounting for player's camera angle (in degrees), will try to modify angle of outvector to go toward camera forward vector at the cost of magnitude
  //15 degrees or less away means no cost of magnitude
  float PlayerCameraAngle;
  FVector CameraForwardVector2D = GetFirstPersonCameraComponent()->GetForwardVector().GetSafeNormal2D();
  //dot product between camera forward vector (normalized) and out vector (without z taken into account)
  float DotProductCO = CameraForwardVector2D.Dot(WallRun.OutVector.GetSafeNormal2D());

  //positive is away, zero is no effect (neutral), negative is inward
  int SignOfCO = UKismetMathLibrary::SignOfFloat(DotProductCO);

  PlayerCameraAngle = UKismetMathLibrary::ClampAngle(UKismetMathLibrary::DegAcos(DotProductCO), 0.f, 60.f);

  //cut it in half to get the actual degree direction change
  PlayerCameraAngle *= 0.5;

  //initially normal in direction of camera forward vector, it always points left relative to outvector b/c right hand rule moment
  FVector OutVectorNormal = WallRun.OutVector.Cross(FVector::UnitZ()).GetSafeNormal();

  //direct the normal to direction of camera forward vector relative to outvector
  OutVectorNormal *= UKismetMathLibrary::SignOfFloat((OutVectorNormal - FVector(0.f, 0.f, OutVectorNormal.Z)).Dot(CameraForwardVector2D));

  FVector NewDirection = WallRun.OutVector.GetSafeNormal() * UKismetMathLibrary::DegCos(PlayerCameraAngle) + OutVectorNormal * UKismetMathLibrary::DegSin(PlayerCameraAngle);
  //just in case these basis vectors aren't perpendicular or rounding is bad idk
  NewDirection.Normalize();

  //calculate the new magnitude based on how much the direction angle is changed
  if (PlayerCameraAngle <= 7.5)
  {
    //no change
    WallRun.OutVector = NewDirection * OutVectorMagnitude;
  }
  else
  {
    WallRun.OutVector = NewDirection * (OutVectorMagnitude * (-0.01 * (PlayerCameraAngle-10.f) + 1));
  }
  //add z velocity
  WallRun.OutVector += FVector(0.f, 0.f, JumpVelocity * 0.9);
  UKismetSystemLibrary::PrintString(this, "yo there's supposed to be a vector below:");
  UKismetSystemLibrary::PrintString(this, WallRun.OutVector.ToString());
  LaunchCharacter(WallRun.OutVector, true, false);
}

void AAPlatformerCharacter::WallBump()
{
  //equivalent to a mini wall jump (without z boost), significantly reduced velocity in wallrun.normal direction

  FVector VelocityXY = GetVelocity();
  VelocityXY.Z = 0.f;
  FVector LaunchVector = VelocityXY - (WallRun.Normal * (1.2 * FVector::DotProduct(VelocityXY, WallRun.Normal)));
  LaunchCharacter(LaunchVector, true, false);
}

void AAPlatformerCharacter::CalculateMaxLeanIn()
{
  //create a forwardvector parallel to the wall in the direction of the player's movement
  WallRun.ForwardVector = WallRun.Normal.Cross(FVector(0.f,0.f, 1.f));
  //normalized dot product
  float ForwardsDot = WallRun.ForwardVector.GetSafeNormal().Dot(GetVelocity().GetSafeNormal());
  WallRun.ForwardVector *= ForwardsDot;
  WallRun.ForwardVector.Normalize();
  float ForwardsDot2 = WallRun.ForwardVector.Dot(GetVelocity().GetSafeNormal());

  //same with the upvector
  WallRun.UpVector = WallRun.Normal.Cross(WallRun.ForwardVector);
  //normalized dot product
  float UpsDot = WallRun.UpVector.GetSafeNormal().Dot(FVector(0.f,0.f,1.f));
  WallRun.UpVector *= UpsDot;
  WallRun.UpVector.Normalize();
  float UpsDot2 = WallRun.UpVector.Dot(FVector(0.f, 0.f, 1.f));

  //angle between (0, 0, 1) and Wall UpVector
  float AngleBtwnUps = UKismetMathLibrary::DegAcos(UpsDot2);
  //angle between Actor and Wall Forward vectors
  float AngleBtwnForwards = UKismetMathLibrary::DegAcos(ForwardsDot2);
  UKismetSystemLibrary::PrintString(this, "anglebtwnups: " + FString::SanitizeFloat(AngleBtwnUps));
  UKismetSystemLibrary::PrintString(this, "anglebtwnforwards: " + FString::SanitizeFloat(AngleBtwnForwards));
  //calculate roll
  float AngleX = (15 + AngleBtwnUps)*UKismetMathLibrary::DegCos(AngleBtwnForwards)*UKismetMathLibrary::SignOfFloat(WallRun.Normal.Dot(GetActorRightVector()));
  //calculate pitch
  float AngleY = (15 + AngleBtwnUps)*UKismetMathLibrary::DegSin(AngleBtwnForwards)*-1;
  WallRun.LeanInRotator = FRotator(0.f, AngleY, AngleX);
}

void AAPlatformerCharacter::UpdateLeanIn(float Value)
{
  FirstPersonCameraComponent->SetRelativeRotation(WallRun.LeanInRotator*Value);
}

void AAPlatformerCharacter::UpdateZVelocity(float Value)
{
  float NewZVelocity = FMath::Lerp(WallRun.InVector.Z, 0.f, Value);
  LaunchCharacter(FVector(0.f,0.f,NewZVelocity), false, true);
}

AAPlatformerCharacter::AAPlatformerCharacter()
{
	// Character doesnt have a rifle at start
	bHasRifle = false;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(40.f, 96.0f);
  
  //create a spring arm so the camera can be rotated
  SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
  SpringArm->SetupAttachment(GetCapsuleComponent());
  SpringArm->SetRelativeLocation(FVector(-10.f, 0.f, 60.f));
  SpringArm->TargetArmLength = 0.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.f,0.f,0.f)); // Position the camera
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
  TickDelegate.AddDynamic(this, &AAPlatformerCharacter::HoverCompute);
}

void AAPlatformerCharacter::Tick(float DeltaSeconds)
{
  DeltaTime = DeltaSeconds;
  if (WallRun.bGetVelocity)
  {
    //cool b/c it's not a timer
    VelocityButEvenBeforeNextTick = GetVelocity();
    FLatentActionInfo LatentInfo;
    LatentInfo.CallbackTarget = this;
    LatentInfo.ExecutionFunction = "SetVelocityLastTick";
    LatentInfo.Linkage = 0;
    LatentInfo.UUID = 1;
    UKismetSystemLibrary::DelayUntilNextTick(this, LatentInfo);
  }
  TickDelegate.Broadcast();

  //totally not copy pasted
  if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		// Blueprint code outside of the construction script should not run in the editor
		// Allow tick if we are not a dedicated server, or we allow this tick on dedicated servers
		if (GetWorldSettings() != nullptr && (bAllowReceiveTickEventOnDedicatedServer || !IsRunningDedicatedServer()))
		{
			ReceiveTick(DeltaSeconds);
		}


		// Update any latent actions we have for this actor

		// If this tick is skipped on a frame because we've got a TickInterval, our latent actions will be ticked
		// anyway by UWorld::Tick(). Given that, our latent actions don't need to be passed a larger
		// DeltaSeconds to make up the frames that they missed (because they wouldn't have missed any).
		// So pass in the world's DeltaSeconds value rather than our specific DeltaSeconds value.
		UWorld* MyWorld = GetWorld();
		if (MyWorld)
		{
			FLatentActionManager& LatentActionManager = MyWorld->GetLatentActionManager();
			LatentActionManager.ProcessLatentActions(this, MyWorld->GetDeltaSeconds());
		}
	}
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

    EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAPlatformerCharacter::MoveInputActionMethod);
    EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AAPlatformerCharacter::StopClimbingActionMethod);

    //wall running
    EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AAPlatformerCharacter::WallRunLetGo);

    //ledge cancel
    EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Started, this, &AAPlatformerCharacter::StopLedgeHangAndClimbing);
    EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AAPlatformerCharacter::StopLedgeHangAndClimbing);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAPlatformerCharacter::Look);

    //Sprinting
    EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AAPlatformerCharacter::Sprint);

    //Open the Paused Menu
    EnhancedInputComponent->BindAction(PausedMenuAction, ETriggerEvent::Started, this, &AAPlatformerCharacter::TriggerPausedMenu);

    //interact actions
    EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AAPlatformerCharacter::StartInteract);
    EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &AAPlatformerCharacter::StopInteract);
	}
}


void AAPlatformerCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
  if (WallRun.bIsRunning || WallRun.bLeaning)
  {
    //wallrun movement controls
    //copied from calculatemaxleanin to get the forward angle btwn, but using actorforwardvector instead of velocity

    //create a forwardvector parallel to the wall in the direction of the player's movement
    WallRun.ForwardVector = WallRun.Normal.Cross(FVector(0.f,0.f, 1.f));
    //normalized dot product
    float ForwardsDot = WallRun.ForwardVector.GetSafeNormal().Dot(GetVelocity().GetSafeNormal());
    WallRun.ForwardVector *= ForwardsDot;
    WallRun.ForwardVector.Normalize();
    float ForwardsDot2 = WallRun.ForwardVector.Dot(GetActorForwardVector().GetSafeNormal());

    //angle between Actor and Wall Forward vectors
    float AngleBtwnForwards = UKismetMathLibrary::DegAcos(ForwardsDot2);

    if (MovementVector.X != 0.f)
    {
      AddMovementInput(GetActorRightVector(), MovementVector.X * StrafeMultiplier);
    }
    UKismetSystemLibrary::PrintString(this, "in move controls: your forward angle between is " + FString::SanitizeFloat(AngleBtwnForwards));
    if (AngleBtwnForwards < 40.f)
    {
      //angle between forwards less than 40 means you can't move forward or back
      bIsMoveInputReleased = false;
      LastMoveInput = MovementVector;
      return;
    }
    if (MovementVector.Y>0)
    {
		  AddMovementInput(GetActorForwardVector(), MovementVector.Y * ForwardMultiplier);
    }
    else
    {
      AddMovementInput(GetActorForwardVector(), MovementVector.Y);
    }

    bIsMoveInputReleased = false;
    LastMoveInput = MovementVector;
    return;
  }

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
    //lurching meant for digital inputs as expected (doesn't take forward or strafe multiplier into account), can't lurch while wallrunning or leaning
    //first check if new direction pressed (not released)
    if (MovementVector.Size() >= LastMoveInput.Size() && (MovementVector != LastMoveInput || bIsMoveInputReleased))
    {
      //get the time since last jump and lurch if is less than periodmax
      int TimeSinceJump = 0.4 - GetWorld()->GetTimerManager().GetTimerRemaining(OnJumpedHandle);
      UKismetSystemLibrary::PrintString(this, "lurch check moment");
      if (TimeSinceJump >= 0.4)
      {
        bIsMoveInputReleased = false;
        LastMoveInput = MovementVector;
        return;
      }
      FVector2D VelocityXY = FVector2D(GetVelocity().X, GetVelocity().Y);
      //combined with dropoff multiplier to calculate finallurchvector magnitude
      int LurchPeriodMultiplier;
      if (TimeSinceJump <= Lurch.PeriodMin)
      {
        LurchPeriodMultiplier = 1;
      }
      else
      {
        LurchPeriodMultiplier = (-1 / (Lurch.PeriodMax - Lurch.PeriodMin))*(TimeSinceJump - Lurch.PeriodMin) + 1;
      }
      //angle check
      FVector2D ForwardVector2D = FVector2D(GetActorForwardVector().X, GetActorForwardVector().Y);
      FVector2D RightVector2D = FVector2D(GetActorRightVector().X, GetActorRightVector().Y);
      //also normalized
      FVector2D MovementVectorOriented = (ForwardVector2D * MovementVector.Y) + (RightVector2D * MovementVector.X);
      float AngleBetween = UKismetMathLibrary::DegAcos(VelocityXY.GetSafeNormal().Dot(MovementVectorOriented.GetSafeNormal()));
      float DropoffMultiplier = 1;
      if (AngleBetween > Lurch.MaxAngle)
      {
        UKismetSystemLibrary::PrintString(this, "angle between calculated" + FString::SanitizeFloat(AngleBetween));
        DropoffMultiplier = (1 - ((AngleBetween - Lurch.MaxAngle) * Lurch.Dropoff));
      }
      float NewMagnitude = (DropoffMultiplier * LurchPeriodMultiplier) * VelocityXY.Size();
      FVector2D NewDirection = ((VelocityXY.GetSafeNormal()) + (MovementVectorOriented.GetSafeNormal() * (Lurch.Strength))).GetSafeNormal();
      FVector FinalLurchVector = FVector(NewDirection.X, NewDirection.Y, 0.f) * NewMagnitude;
      UKismetSystemLibrary::PrintString(this, NewDirection.ToString());
      UKismetSystemLibrary::PrintString(this, FinalLurchVector.ToString());
      UKismetSystemLibrary::PrintString(this, "maybe normalization off" + ((VelocityXY.GetSafeNormal()) + (MovementVector.GetSafeNormal() * (Lurch.Strength))).ToString());
      UKismetSystemLibrary::PrintString(this, "dropoff multiplier " + FString::SanitizeFloat(DropoffMultiplier));
      UKismetSystemLibrary::PrintString(this, "period multiplier " + FString::SanitizeFloat(LurchPeriodMultiplier));
      LaunchCharacter(FinalLurchVector, true, false);
    }
    bIsMoveInputReleased = false;
    LastMoveInput = MovementVector;
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

void AAPlatformerCharacter::JumpE()
{
  Jump();
  if (WallRun.bIsRunning || GetWorld()->GetTimerManager().GetTimerRemaining(WallRun.CoyoteHandle) > 0.f)
  {
    //wall run jump
    UKismetSystemLibrary::PrintString(this, "trying to cancel for wall jump");
    WallRunJump();
    WallRunCancel();
    GetWorld()->GetTimerManager().SetTimer(OnJumpedHandle, this, &AAPlatformerCharacter::ResetClimbCooldown, 0.4, false);
    return;
  }
  if (WallRun.bLeaning)
  {
    WallRunCancel();
    return;
  }
  
  if (bSuperGlideSlideQueued)
  {
    bSuperGlideSlideQueued = false;
    UKismetSystemLibrary::PrintString(this, "Superglide successful");
    SuperGlide();
    OnJumped();
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
    float Multiplier = GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * -1.15;
    if (UKismetSystemLibrary::LineTraceSingle(this, GetActorLocation(), GetActorLocation() + FVector(0.f, 0.f, Multiplier), ETraceTypeQuery::TraceTypeQuery1, false, EmptyActorArray, EDrawDebugTrace::ForOneFrame, HitResult, true))
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

void AAPlatformerCharacter::StartInteract()
{
  if (HoveredInteractable.GetInterface() == nullptr)
  {
    return;
  }
  HoveredInteractable.GetInterface()->StartInteract(this);
  bIsInteracting = true;
}

void AAPlatformerCharacter::StopInteract()
{
  if (HoveredInteractable.GetInterface() == nullptr || !bIsInteracting)
  {
    return;
  }
  if (!bIsHovering)
  {
    bIsInteracting = false;
    return;
  }
  HoveredInteractable.GetInterface()->CancelInteract(this);
  bIsInteracting = false;
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
  if (WallRun.bIsRunning || WallRun.bLeaning)
  {
    WallRunCancel();
    return;
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
  //you can't regular wall jump when you can wall run (you have wallrunjump instead)
  if (!WallRun.bCanRun)
  {
    TickDelegate.AddDynamic(this, &AAPlatformerCharacter::WallJumpCompute);
  }
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
  //clear onjumped timer handle
  GetWorld()->GetTimerManager().ClearTimer(OnJumpedHandle);

  //reset wall running
  UKismetSystemLibrary::PrintString(this, "attempting to reset wallrun struct");
  GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AAPlatformerCharacter::WallRunReset);
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

  //double jump cooldown to make sure player doesn't get punished when double clicking jump within 0.1s
  bDoubleJumpCooldown = true;
  GetWorld()->GetTimerManager().SetTimer(DoubleJumpCooldownHandle, this, &AAPlatformerCharacter::SetDoubleJumpCooldownFalse, 0.1, false);
  //right now just for lurching
  GetWorld()->GetTimerManager().SetTimer(OnJumpedHandle, this, &AAPlatformerCharacter::ResetClimbCooldown, 0.4, false);

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
  //remember todo, update save game to include everything
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

void AAPlatformerCharacter::SetCanWallRunTrue()
{
  WallRun.bCanRun = true;
  WallRun.bGetVelocity = true;
}

void AAPlatformerCharacter::SetCanWallRunFalse()
{
  WallRun.bCanRun = false;
  WallRunCancel();
  UpdateLeanIn(0.f);
}
