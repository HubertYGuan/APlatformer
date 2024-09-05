// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "OverlapInterface.h"
#include "InteractInterface.h"
#include "APlatformerCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class USpringArmComponent;
class UAnimMontage;
class USoundBase;
class APCThing;
class UMainGI;
class AInteractable;
class UGunComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTickDelegateSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FJumpDelegateSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWallRunResetDelegateSignature);

UENUM(BlueprintType)
enum class EGravShiftDirection : uint8
{
  DOWN = 0  UMETA(DisplayName = "DOWN"),
  LEFT = 1  UMETA(DisplayName = "LEFT"),
  UP = 2    UMETA(DisplayName = "UP"),
  RIGHT = 3 UMETA(DisplayName = "RIGHT")
};

USTRUCT(BlueprintType)
struct FWallRunStruct
{
  GENERATED_BODY()
  
  FVector VelocityLastTick = FVector();

  float MaxSpeedXY = 1000.f;
  
  UPROPERTY(BlueprintReadOnly)
  FTimerHandle Handle;

  float CoyoteTime = 0.1;

  FTimerHandle CoyoteHandle;

  UPROPERTY(BlueprintReadOnly)
  bool bIsRunning = false;

  UPROPERTY(BlueprintReadOnly)
  //you can't wallrun multiple times on the same wall without losing height
  float Height = -69420.f;

  UPROPERTY(BlueprintReadOnly)
  AActor *Wall = nullptr;

  UPROPERTY(BlueprintReadOnly)
  //wall running will be time/general area based, not wall based (so bool is used instead of tag)
  bool bCanRun = false;

  //idk why this is in here, it's kinda redundant
  FVector InVector = FVector();

  FVector OutVector = FVector();

  FVector Normal = FVector();

  //going up parallel to wall
  FVector UpVector = FVector();

  //going forward prallel to wall
  FVector ForwardVector = FVector();

  //lean in rotator:
  FRotator LeanInRotator = FRotator();

  UPROPERTY(BlueprintReadOnly)
  bool SameWallCooldown = false;

  //if player double jumped since last coming off ground or wall
  bool bDoubleJumped = false;

  bool bGetVelocity = false;

  UPROPERTY(BlueprintReadOnly)
  //if player is currently in the lean in animation
  bool bLeaning = false;
  //if player is currently in the cancel wallrunning animation
  UPROPERTY(BlueprintReadOnly)
  bool bCancellingAnim = false;

  bool bWallBumpQueued = false;
  bool bWallBumping = false;
};
UCLASS(config=Game)
class AAPlatformerCharacter : public ACharacter, public IOverlapInterface, public IInteractInterface
{
	GENERATED_BODY()

  UFUNCTION()
  void WallRunReset();

  UFUNCTION(BlueprintCallable)
  void WallRunSetIsRunning(bool Value);

  UFUNCTION(BlueprintCallable)
  void WallRunSetLeaning(bool Value);

  UFUNCTION(BlueprintCallable)
  void WallRunSetSameWallCooldown(bool Value);

  UFUNCTION(BlueprintCallable)
  void WallRunSetCancellingAnim(bool Value);

  UFUNCTION()
  void WallBumpReset(const FHitResult& Hit){WallRun.bWallBumpQueued = false; WallRun.bWallBumping = false;}

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

  // Another camera for the player mesh to attach to
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* MeshCameraComponent;

  //spring arm camera is attached to
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
  USpringArmComponent *SpringArm;

  //ref to player controller
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  APCThing *PCRef;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  UMainGI *GIRef;

  UPROPERTY()
  float DeltaTime;

  //enable move input
  UFUNCTION()
  void EnableMoveInput(const FHitResult& Hit);

  //called on every move input
  UFUNCTION()
  void MoveInputActionMethod(const FInputActionValue& Value);

  //delegate that is broadcasted every single tick
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
  FTickDelegateSignature TickDelegate;

  //delegate that is broadcasted every jump
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  FJumpDelegateSignature JumpDelegate;

  //bound to landed and auto activates JumpE after a one tick delay since landing is still midair
  UFUNCTION()
  void BufferedJump(const FHitResult& Hit);
  
  //The max walk speed of the character (not sprinting)
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  double MaxSpeedDefault = 500.f;

  //The max walk speed of the character (not sprinting)
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  double MaxSlideSpeed = 1000.f;

  //The jump z velocity of the character (no fatigue)
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  double JumpVelocity = 550.f;

  //The air control of the character
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  double AirControl = 0.1;

  //The number of consecutive landings used to calculate jump fatigue <=5 (uses landings not jumps b/c being able to jump fully after falling a lot is weird)
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = JumpFatigue, meta = (AllowPrivateAccess = "true"))
  int JumpFatigueCount = 0;

  //The time needed to stay on the ground to dissipate jump fatigue
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = JumpFatigue, meta = (AllowPrivateAccess = "true"))
  double JumpFatigueTimer = 0.f;

  //The timer handle for the timer that calls JumpFatigueTimerReset
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = JumpFatigue, meta = (AllowPrivateAccess = "true"))
  FTimerHandle JumpFatigueTimerHandle;
  
  //The last checkpoint position
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = Checkpoint, meta = (AllowPrivateAccess = "true"))
  FVector LastCheckpointPos = FVector(0,0,0);

  //last checkpoint's level (if it's the default, it will be set in begin play)
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = Checkpoint, meta = (AllowPrivateAccess = "true"))
  int LastCheckpointLevel;

  //The last checkpoint actor reference
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = Checkpoint, meta = (AllowPrivateAccess = "true"))
  AActor* LastCheckpointRef;

  //climbing (on w pressed (ticking))
  //if currently climbng
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = Climbing, meta = (AllowPrivateAccess = "true"))
  bool bIsClimbing = false;

  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = Climbing, meta = (AllowPrivateAccess = "true"))
  bool bClimbCooldown = false;

  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = Climbing, meta = (AllowPrivateAccess = "true"))
  bool bIsMantling = false;

  UFUNCTION()
  void SetIsMantlingFalse();

  FTimerHandle SetMantlingFalseHandle;

  //if near ledge (can hang on ledge or go on top)
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = Climbing, meta = (AllowPrivateAccess = "true"))
  bool bIsOnLedge = false;

  //timer handle for climbing timer
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = Climbing, meta = (AllowPrivateAccess = "true"))
  FTimerHandle ClimbingTimer;

  //length of time in seconds player can climb (defualt 2 seconds)
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = Climbing, meta = (AllowPrivateAccess = "true"))
  double MaxClimbLength = 2.f;

  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = Climbing, meta = (AllowPrivateAccess = "true"))
  double MaxClimbVelocityZ = 300.f;

  //timer handle for climb cooldown timer
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = Climbing, meta = (AllowPrivateAccess = "true"))
  FTimerHandle ClimbCooldownTimer;

  //time it takes for character to be able to climb again (default 1.25 seconds)
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = Climbing, meta = (AllowPrivateAccess = "true"))
  double ClimbCooldown = 1.25f;

  //position of ledge when LedgeDetect detects ledge
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = Climbing, meta = (AllowPrivateAccess = "true"))
  FVector LedgePos = FVector();

  //function to reset climb cooldown by clearing the climbing timer handle
  UFUNCTION(BlueprintCallable, Category = Climbing)
  void ResetClimbCooldown();

  //climbing function called on movement input
  UFUNCTION(BlueprintCallable, Category = Climbing)
  void Climb(const FInputActionValue& Value);

  //last move input's axis vector thing
  FVector2D LastMoveInput = FVector2D(0.f, 0.f);

  //Function to set climbing boolean to false, called upon letting go of movement or landing
  UFUNCTION(BlueprintCallable, Category = Climbing)
  void StopClimbing();

  void StopClimbingActionMethod();

  //forward line trace to detect wall
  UFUNCTION(BlueprintCallable, Category = Climbing)
  bool ForwardWallDetect();

  //forward line trace starting from above player to determine if player is on ledge and should be able to hang
  UFUNCTION(BlueprintCallable, Category = Climbing)
  bool LedgeDetect();

  //sets a minimum z height for player and disables sideways movement, cancel by jumping or pressing back
  UFUNCTION()
  void LedgeHang();

  //detects if player reaches certain Z threshold and disables all climbing and ledge stuff when they do
  UFUNCTION()
  void LedgeMantle();

  UPROPERTY(BlueprintReadOnly, Category = Climbing, meta = (AllowPrivateAccess = "true"))
  FVector MantleTarget;

  UPROPERTY(BlueprintReadOnly, Category = Climbing, meta = (AllowPrivateAccess = "true"))
  FVector LastClimbNormal;

  UPROPERTY(BlueprintReadOnly, Category = Climbing, meta = (AllowPrivateAccess = "true"))
  FVector MantleStart;

  //called when jump, completely stops all ledge hanging and climbing
  void StopLedgeHangAndClimbing(const FInputActionValue& InputActionValue);

  //function to disable or enable strafing
  UFUNCTION(BLueprintCallable)
  void SetIgnoreStrafing(bool ShouldDisable);

  //function to disable or enable forward input
  UFUNCTION(BLueprintCallable)
  void SetIgnoreForward(bool ShouldDisable);

  //how much strafe movement should be multiplied by
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, meta = (AllowPrivateAccess = "true"))
  double StrafeMultiplier = 1.f;

  //how much forward movement should be multiplied by
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, meta = (AllowPrivateAccess = "true"))
  double ForwardMultiplier = 1.f;

  //If player is dead
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = Checkpoint, meta = (AllowPrivateAccess = "true"))
  bool bIsDead = false;

  //If player has shoes
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = Gear, meta = (AllowPrivateAccess = "true"))
  bool bHasShoes = false;

  //if player is sprinting
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, meta = (AllowPrivateAccess = "true"))
  bool bIsSprinting = false;

  //If player has climbing gear
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = Gear, meta = (AllowPrivateAccess = "true"))
  bool bHasClimbingGear = false;

  //If player has sliding gear
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = Gear, meta = (AllowPrivateAccess = "true"))
  bool bHasSlidingGear = false;

  //If player is currently sliding (does not mean they're on the ground)
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = Sliding, meta = (AllowPrivateAccess = "true"))
  bool bIsSliding = false;

  //If player can wall jump (only true for a short time), set true when WallJumpCompute detects a wall and set to false if timer runs out or WallJumpDetect
  //detects player no longer looking at wall (unbinds WallJumpCompute after)
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = WallJump, meta = (AllowPrivateAccess = "true"))
  bool bCanWallJump = false;

  //If WallJumpDetect has recently triggered true then false
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = WallJump, meta = (AllowPrivateAccess = "true"))
  bool bHadWallJumpOpportunity = false;

  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = WallJump, meta = (AllowPrivateAccess = "true"))
  bool bCanSuperGlide = false;

  bool bSuperGlideSlideQueued = false;

  //timer handle for timer for small window of walljump availability
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = WallJump, meta = (AllowPrivateAccess = "true"))
  FTimerHandle WallJumpWindowHandle;

//timer handle for the slide boost cooldown timer
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = Sliding, meta = (AllowPrivateAccess = "true"))
  FTimerHandle SlideCooldownTimer;

  //timer handle for application of slide boost force, this timer ticks every 0.02s and loops
  FTimerHandle SlideForceTimerHandle;

  //count for amount of times the slide force timer has applied force
  uint8_t SlideForceTimerCount = 0;

  //time it takes for character to be able to slide boost again (they will still be able to slide) (default 2 seconds)
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = Sliding, meta = (AllowPrivateAccess = "true"))
  double SlideCooldown = 2.f;
  
  //function to reset slide cooldown by clearing the sliding timer handle
  UFUNCTION(BlueprintCallable, Category = Sliding)
  void ResetSlideCooldown();

  //if the player has unlocked gravity manipulator (by default it allows faster falling) (but level triggers
  //can upgrade it temporarily to allow sideways gravity and reduced gravity)
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = GravMan, meta = (AllowPrivateAccess = "true"))
  bool bHasGravMan = false;

  //if player can shift gravity 90 degrees
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = GravMan, meta = (AllowPrivateAccess = "true"))
  bool bCanGravShift = false;

  //the rightward vector for grav shift
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = GravMan, meta = (AllowPrivateAccess = "true"))
  FVector GravShiftRightVector;

  //if player can reduce gravity
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = GravMan, meta = (AllowPrivateAccess = "true"))
  bool bCanReduceGrav = false;

  //grav shift currently wip
  UFUNCTION(BlueprintCallable, Category = GravMan)
  void GravShift(EGravShiftDirection Direction);

  //# of times can double jump (increase by one every time contact dj orb)
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = DoubleJump, meta = (AllowPrivateAccess = "true"))
  int DoubleJumpCount;

  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = DoubleJump, meta = (AllowPrivateAccess = "true"))
  FTimerHandle DoubleJumpHandle;

  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = DoubleJump, meta = (AllowPrivateAccess = "true"))
  bool bIsDoubleJumpHeld = false;

  //makes scroll jumping actually possible since you won't immediately double jump after doing it
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = DoubleJump, meta = (AllowPrivateAccess = "true"))
  bool bDoubleJumpCooldown = false;

  FTimerHandle DoubleJumpCooldownHandle;

  void SetDoubleJumpCooldownFalse() {bDoubleJumpCooldown = false;};
  //it triggers double jump, unlike regular jump, it is dependent on how long you hold
  UFUNCTION(BlueprintCallable, Category = DoubleJump)
  void DoubleJump();

  UFUNCTION(BlueprintCallable, Category = DoubleJump)
  void EndDoubleJump();

  //ticks 10 times or 1/5 of second in total
  UFUNCTION(BlueprintCallable, Category = DoubleJump)
  void ApplyDoubleJumpForce();

  int DoubleJumpForceCounter = 0;

  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = DoubleJump, meta = (AllowPrivateAccess = "true"))
  bool bHasDoubleJumpBoots = false;

  public:
  UFUNCTION(BlueprintCallable, Category = DoubleJump)
  void DoubleJumpBootsPickup();

  UFUNCTION(BlueprintCallable, Category = DoubleJump)
  void DoubleJumpOrbOverlap();

  protected:
  //timer called every onjumped (currently used just for lurch)
  FTimerHandle OnJumpedHandle;

  //lurching
  //struct for default lurch settings (borrowed not stolen from titanfall)
  struct LurchStruct
  {
    //time when lurching is possible
    float PeriodMax = 0.4;
    //time when lurch magnitude curve isn't in effect (linear down to zero after this)
    float PeriodMin = 0.2;

    //determines how fast velocity drops off beyond maxangle (should set so player will still be able to lurch even if 180 tho)
    //DegreeChange is based on input MovementVector not new velocity vector
    //DropoffMultiplier = (1 - (DegreeChange - MaxAngle) * Dropoff)
    float Dropoff = 0.003;

    //max angle between velocity and lurch direction without speed loss
    float MaxAngle = 25.f;
    //How much a lurch input affects velocity direction (GetVelocity().GetSafeNormal()+MovementVector.GetSafeNormal()*Strength).GetSafeNormal() for new direction
    float Strength = 0.5;
  };

  LurchStruct Lurch;

  UPROPERTY()
  bool bIsMoveInputReleased = true;

  FVector VelocityButEvenBeforeNextTick;

  UFUNCTION()
  void SetVelocityLastTick() {WallRun.VelocityLastTick = VelocityButEvenBeforeNextTick;}

  UPROPERTY(BlueprintReadWrite, Category = WallRun)
  FWallRunStruct WallRun;
  
  UFUNCTION(BlueprintCallable)
  void WallRunHandleReset();

  UFUNCTION(BlueprintCallable)
  void WallRunHandleStart();

  UFUNCTION(BlueprintCallable)
  void WallRunCoyoteClear();

  //when wall running, you have to hold w and forward input is nullified in move so player can stick to wall, strafing is not nullified so you can strafe off of a wall
  //called every tick while moving (works if bCanRun)
  UFUNCTION(BlueprintCallable, Category = WallRun)
  void WallRunCompute(const FInputActionValue& Value);

  //detects wall via player sized capsule (vertically shrunk and swept down) and also sets variables in wallrun struct, accounts for height, impactnormal, etc.
  //first bool sets height and normal for stopping wallrun, second bool overrides first if true and disables height check/wall check
  UFUNCTION(BlueprintCallable, Category = WallRun)
  bool WallRunDetect(bool ForWallRunStop = false, bool OnlyCollisionCheck = false);

  //used to set samewallcooldown to false if not in collision with the same wall or any actor
  UFUNCTION()
  void SameWallDetect();

  //checks if self is already binded and binds to tick delegate if not already binded
  UFUNCTION(BlueprintCallable, Category = WallRun)
  void BindSameWallDetect();

  //additionally unbinds getvelocity, adds one double jump if bDoubleJumped
  UFUNCTION(BlueprintCallable, Category = WallRun)
  void WallRunStart();

  //also rebinds getvelocity and gives a very small amount of coyote time to end boost (can be triggered on crouch, 1.75s on wall, looking too far away or in when wall running, strafing away enough from wall)
  UFUNCTION(BlueprintCallable, Category = WallRun)
  void WallRunCancel();

  //checks if wall running or leaning, stops leaning or running via cancel if so (sets timer to cancel in 0.25s if so)
  UFUNCTION(BlueprintCallable, Category = WallRun)
  void WallRunLetGo();

  FTimerHandle LetGoHandle;
  FTimerHandle LetGoHandle2;

  //called inside wallruncompute if player needs boost on wall
  UFUNCTION(BlueprintCallable, Category = WallRun)
  void WallRunBoost();

  //called inside wallruncompute (running) if player needs slow down on wall
  UFUNCTION(BlueprintCallable, Category = WallRun)
  void WallRunSlow();

  //also rebinds getvelocity
  UFUNCTION(BlueprintCallable, Category = WallRun)
  void WallRunJump();

  //similar to TF2, you get pushed a little away from a wall if you try to wall run on it while crouched and get double jump refreshed, also rebinds getvelocity
  UFUNCTION(BlueprintCallable, Category = WallRun)
  void WallBump();

  //does lean in animation (sets bLeaning true) for 0.25s while adjusting (for 0.3s) player's z velocity to zero, starts wall running after
  //starts to return camera angle after 1.25s of running, wallruncancel is called after 1.75s of running
  UFUNCTION(BlueprintImplementableEvent)
  void WallRunStartTimelineBegin();

  //called whenever a wall run is cancelled in some way (jump, cancel, etc.), stops the timeline but doesn't do wallrundetect, does an animation to return camera angle
  UFUNCTION(BlueprintImplementableEvent)
  void WallRunStartTimelineStop();

  //function to calculate max lean in angle
  UFUNCTION(BlueprintCallable, Category = WallRun)
  void CalculateMaxLeanIn();
  //timeline function for lean in/lean out, sets the camera to the correct base (relative) angle for the wall multiplied by the value which should be between 0 and 1
  UFUNCTION(BlueprintCallable, Category = WallRun)
  void UpdateLeanIn(float Value);

  UFUNCTION(BlueprintCallable, Category = WallRun)
  //timeline function for updating z velocity (called in lean in)
  void UpdateZVelocity(float Value);

  public:
  //additionally binds a getvelocity to end of wallrundetect
  UFUNCTION(BlueprintCallable, Category = WallRun)
  void SetCanWallRunTrue();

  //additionally unbinds getvelocity
  UFUNCTION(BlueprintCallable, Category = WallRun)
  void SetCanWallRunFalse();

  protected:
  UPROPERTY()
  FTimerHandle UnusedHandle;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;
  
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;
  /** Crouch Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* CrouchAction;
  /** Sprint Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* SprintAction;
  //esc to pull up the menu/paused ui
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* PausedMenuAction;
  //to increase gravity when held
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* GravIncreaseAction;
  //to shift gravity right/left (positive is right)
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* GravShiftAction;
  //grav float when held
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* GravReduceAction;
  //interact key to press or hold down
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* InteractAction;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* HolsterAction;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* ReloadAction;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* ShootingAction;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* EquipAction1;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* EquipAction2;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

public:
	AAPlatformerCharacter();

  virtual void Destroyed() override;

protected:
	virtual void BeginPlay();

  virtual void Tick(float DeltaSeconds) override;

public:
		
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	/** Bool for AnimBP to switch to another animation set */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon)
	bool bHasRifle;

	/** Setter to set the bool */
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SetHasRifle(bool bNewHasRifle);

	/** Getter for the bool */
	UFUNCTION(BlueprintCallable, Category = Weapon)
	bool GetHasRifle();


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon)
	bool bHasShotgun;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon)
  FName GunOutName;

  // If currently holding the weapon out, not holstered
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon)
  bool bWeaponOut = false;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon)
  bool bWantsToEquip1 = false;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon)
  bool bWantsToEquip2 = false;

  protected:
  // Amount of shotgun ammo in reserve
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon)
  int ShotgunReserve;

  public:
  // Reference to gun that is being held
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon)
  UGunComponent *GunOut;

  UFUNCTION(BlueprintCallable, Category = Weapon)
  int GetShotgunReserve();

  UFUNCTION(BlueprintCallable, Category = Weapon)
  void DecreaseShotgunReserve(int Amount);

  protected:

  // Ref to gun in slot 1
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon)
  UGunComponent *GunSlot1;

  // Ref to gun in slot 2
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon)
  UGunComponent *GunSlot2;



  // Input action functions below called to do these specific things on GunOut

  UFUNCTION(BlueprintCallable, Category="Weapon")
	void StartShootingInput();

	UFUNCTION(BlueprintCallable, Category="Weapon")
	void StopShootingInput();

public:
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void EquipInput1();
  
  UFUNCTION(BlueprintCallable, Category="Weapon")
	void EquipInput2();

protected:
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void HolsterInput();

	UFUNCTION(BlueprintCallable, Category="Weapon")
	void ReloadInput();
  public:
  // Attempts to pickup gun
  void AttemptGunPickup(UGunComponent *Gun, FName Name);

  UFUNCTION(BlueprintCallable, Category = Weapon)
  void AddShotgunAmmo(int Ammo);

  // Attaches gun to character with slot provided
  UFUNCTION(BlueprintCallable, Category = Weapon)
  void AddGun(UGunComponent *Gun, FName Name, int Slot);

	UFUNCTION(BlueprintCallable, Category = Weapon)
	bool GetHasShotgun();

  UFUNCTION(BlueprintCallable, Category = Weapon)
	void SetHasShotgun(bool bNewHasShotgun);

  //to set bHasShoes to true
  UFUNCTION()
  void SetShoesTrue();

  UFUNCTION()
  void SetClimbingTrue();

  UFUNCTION()
  void SetSlidingTrue();

  //get the PCRef
  UFUNCTION()
  APCThing *GetPCRef();

  UFUNCTION()
  UMainGI *GetGIRef();

  UFUNCTION(BlueprintCallable, Category = Save)
  void CreateSave(FString SaveSlot, int SaveIndex, FString CustomName, bool async = true);

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

  void JumpE();

  //Health system 123

  UPROPERTY(BlueprintReadWrite, Category = Health)
  int Health = 100;

  UPROPERTY(BlueprintReadWrite, Category = Health)
  int MaxHealth = 100;

  //deals damage and updates HUD, kills player if health 0 or negative
  UFUNCTION(BlueprintCallable, Category = Health)
  void SimpleDamageCompute(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

  //the default heal that heals 1 point every 0.02s after not taking damage for 3s
  UFUNCTION(BlueprintCallable, Category = Health)
  void DefaultHeal();

  UFUNCTION(BlueprintCallable, Category = Health)
  void DefaultHealTick();

  //any heal, updates HUD
  UFUNCTION(BlueprintCallable, Category = Health)
  void Heal(int Healing);

  FTimerHandle DefaultHealHandle;

  FTimerHandle DefaultHealTickHandle;

  UFUNCTION()
  void UpdateHealthBarThing();
//I didn't realize widgetinteraction component was a thing bruh

  void StartInteract();

  void StopInteract();

  bool bIsInteracting;

  bool bIsHovering;

  //hovered actor's location
  FVector HoveredPosition = FVector();

  public:
  UFUNCTION(BlueprintCallable, Category = Interact)
  FVector GetHoveredPosition(){return HoveredPosition;}

  UFUNCTION(BlueprintCallable, Category = Interact)
  void SetIsInteractingFalse(){bIsInteracting = false;}

  UFUNCTION(BlueprintCallable, Category = Interact)
  bool GetIsInteracting(){return bIsInteracting;}

  protected:

  UPROPERTY(BlueprintReadOnly)
  TScriptInterface<IInteractInterface> HoveredInteractable;

  UFUNCTION()
  //function binded to tick
  void HoverCompute();

  //linetrace for hovercompute, see what the player hovers over and tries to interact with and sets hovered interactable
  bool HoverDetect();

  FTimerHandle SuperglideInitiateHandle;

  UFUNCTION(BlueprintCallable)
  void UpdatePosition(float Value);

  public:
  UPROPERTY(BlueprintReadWrite)
  FVector CameraPanLocation;

  UPROPERTY(BlueprintReadWrite)
  FVector CameraPanReturnLocation;

  UPROPERTY(BlueprintReadWrite)
  FVector CameraPanInitLocation;

  UPROPERTY(BlueprintReadWrite)
  FRotator CameraPanRotation;

  UPROPERTY(BlueprintReadWrite)
  FRotator CameraPanReturnRotation;

  UPROPERTY(BlueprintReadWrite)
  FRotator CameraPanInitRotation;

  //this isn't in kismetmathlibrary for some reason
  FVector VectorLerp(FVector A, FVector B, double Alpha);

  FRotator RotatorLerp(FRotator A, FRotator B, double Alpha);

  
  UFUNCTION(BlueprintCallable)
  void UpdateCameraTransform(float Value, bool Returning);

  UFUNCTION(BlueprintCallable)
  void AlignCameraTransforms();

  protected:

  UFUNCTION(BlueprintImplementableEvent)
  void MantleTimelinePlay();

  UFUNCTION(BlueprintImplementableEvent)
  void MantleTimelineStop();

  //Crouch Functions, doubles for sliding
  void CrouchE();
  void UnCrouchE();

  //Sprint Function (toggle in between)
  void Sprint();

  //starts sliding, can be queued with CrouchE (binded to Landed) at any point (as long as xy velocity is high enough)
  //and unbinds self from landed delegate if it's binded, binds EndSlideInAir to jump
  //while sliding, player wasd input is ignored
  UFUNCTION(BlueprintCallable, Category = Sliding)
  void BeginSlide(const FHitResult& Hit = FHitResult());

  //applies one tenth of the total force of one slide boost (ticks ten times if held down completely)
  //increased slide force depending on how much the vector slides downward, increased up to 40%, decreased down to 100%
  UFUNCTION(BlueprintCallable, Category = Sliding)
  void ApplySlideForce();
  
  //static struct solely used to store data for applyslideforce
  struct SlideForceStruct
  {
    FVector FrameLaunchForce = FVector();
    FVector SlideForceVector = FVector();
  };

  SlideForceStruct SlideForce;
  //ends sliding due to jumping or going midair (bIsSliding is still true), binds BeginSlide to landed
  //adds WallJumpCompute to tickdelegate until unbinded by landed (or maybe wallrun initiate)
  UFUNCTION(BlueprintCallable, Category = Sliding)
  void EndSlideInAir();

  //check if the player is sliding and too slow, so go to crouching (return true if too slow so should go to crouching, else false)
  UFUNCTION(BlueprintCallable, Category = Sliding)
  void CheckSlideSpeed();

  //ends sliding entirely and sets bIsSliding to false, can be binded to landed by player releasing slide key in midair or immediately called on on ground release
  //unbinds EndSlideInAir from jump if bound
  UFUNCTION(BlueprintCallable, Category = Sliding)
  void EndSlideCompletely();

  //changes the height of the character to crouching height when sliding, true means low height false means normal
  UFUNCTION(BlueprintCallable, Category = Sliding)
  void SlideHeightChange(bool ShouldBeLow);

  void SlideHeightChangeTrue();

  //triggers wall jump
  UFUNCTION(BlueprintCallable, Category = WallJump)
  void WallJump();

  //detects if wall is available to wall jump, if so and then false, sets bHadWallJumpOpportunity true
  UFUNCTION(BlueprintCallable, Category = WallJump)
  bool WallJumpDetect(bool WriteToStruct = false);

  bool WallJumpDetectStrict();

  //binded to TickDelegate by EndSlideInAir (can cancel slide midair and still walljump)
  //unbinded by landed or timer 
  UFUNCTION(BlueprintCallable, Category = WallJump)
  void WallJumpCompute();

  UFUNCTION(BlueprintCallable, Category = WallJump)
  void WallJumpSlowDown();

  UFUNCTION(Category = WallJump)
  void SetWallJumpOpportunityFalse(const FHitResult& Hit);

  //used to calculate walljump slowdown and launch vectors
  struct WallJumpStruct
  {
    //velocity vector right before going on wall, has a z value
    FVector PlayerVelocity = FVector();
    //no z value
    FVector PlayerVelocityXY = FVector();
    
    FVector WallNormal = FVector();

    FTimerHandle WallJumpSlowDownHandle = FTimerHandle();

    FVector LaunchVector = FVector();

    int SlowTickCount = 0;
  };

  WallJumpStruct WallJump0;

  //binded to WallJumpWindowHandle and landed by endslideinair
  UFUNCTION(BlueprintCallable, Category = WallJump)
  void UnbindWallJumpDetect();

  //launches the player up and in direction of input (hard to do just like in apex and titanfall, but not as hard)
  UFUNCTION(BlueprintCallable, Category = Superglide)
  void SuperGlide();

  void SetSuperGlideSlideQueuedFalse() {bSuperGlideSlideQueued = false;};

  void SetClimbCooldownFalse() {bClimbCooldown = false;};

  FTimerHandle SuperGlideWindowStartHandle;

  void SetCanSuperGlideTrue();

  FTimerHandle SuperGlideWindowEndHandle;

  void SetCanSuperGlideFalse() {bCanSuperGlide = false;};

  //Landing function called on LandedDelegate.Broadcast()
	UFUNCTION(BlueprintNativeEvent)
  void Landing(const FHitResult& Hit);

  //JumpPad Boost function (set Z velocity)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
  void JumpPadBoost(double VelocityZ);

  //override of OnJumped Function
  virtual void OnJumped_Implementation() override;

  virtual void OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal, const FVector& PreviousFloorContactNormal, const FVector& PreviousLocation, float TimeDelta) override;

  //coyote time
  UPROPERTY()
  FTimerHandle CoyoteTimeHandle;
  UPROPERTY()
  bool bCoyoteTime = false;
  UFUNCTION()
  void SetCoyoteTimeFalse() {bCoyoteTime = false;};

  //override of Notify Hit (still calls receive hit in BP)
  virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

  //Calculating new Jump velocity based on Jump Fatigue
  UFUNCTION(Category = JumpFatigue)
  double JumpVelocityCalc(int & Count, double & Timer);

  //Timer function for jump fatigue
  UFUNCTION(Category = JumpFatigue)
  void JumpFatigueTimerReset();

  //Respawn event, default to last checkpoint, also supposed to reset some consumable and other stuff
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Respawn)
  void RespawnPlayer();

  //event to kill the player
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Respawn)
  void KillPlayer();

  //override of actor overlap functions to trigger interface function
  virtual void NotifyActorBeginOverlap(AActor *OtherActor) override;

  virtual void NotifyActorEndOverlap(AActor *OtherActor) override;

  UFUNCTION(BlueprintCallable, Category = Save)
  void LoadSave();

  void TriggerPausedMenu();
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

  USpringArmComponent *GetSpringArm() const {return SpringArm;}

  UCameraComponent* GetMeshCameraComponent() const {return MeshCameraComponent;};

};

