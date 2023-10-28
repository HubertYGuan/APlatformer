// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "OverlapInterface.h"
#include "APlatformerCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UAnimMontage;
class USoundBase;
class APCThing;
class UMainGI;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTickDelegateSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FJumpDelegateSignature);

UCLASS(config=Game)
class AAPlatformerCharacter : public ACharacter, public IOverlapInterface
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

  //ref to player controller
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  APCThing *PCRef;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  UMainGI *GIRef;

  //enable move input
  UFUNCTION()
  void EnableMoveInput(const FHitResult& Hit);

  //delegate that is broadcasted every single tick
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  FTickDelegateSignature TickDelegate;

  //delegate that is broadcasted every jump
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  FJumpDelegateSignature JumpDelegate;
  
  //The max walk speed of the character (not sprinting)
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  double MaxSpeedDefault = 500.f;

  //The max walk speed of the character (not sprinting)
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  double MaxSlideSpeed = 1000.f;

  //The jump z velocity of the character (no fatigue)
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  double JumpVelocity = 450.f;

  //The air control of the character
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  double AirControl = 0.3;

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
  UFUNCTION(BlueprintCallable, Category = Climbing)
  void LedgeHang();

  //detects if player reaches certain Z threshold and disables all climbing and ledge stuff when they do
  UFUNCTION(BlueprintCallable, Category = Climbing)
  void LedgeMantle();

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

  //If player can wall jump (only true for a short time), set true when WallJumpDetect detects a wall and set to false if timer runs out or WallJumpDetect
  //detects player no longer looking at wall (unbinds WallJumpDetect after)
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = Sliding, meta = (AllowPrivateAccess = "true"))
  bool bCanWallJump = false;

  //If WallJumpDetect has recently triggered true
  UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = Sliding, meta = (AllowPrivateAccess = "true"))
  bool bHasHadWallJumpOpportunity = false;

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
  /** Lurch Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* LurchAction;
  //esc to pull up the menu/paused ui
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* PausedMenuAction;

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
  //adds walljumpdetect to tickdelegate until unbinded by landed (or maybe wallrun initiate)
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

  //Landing function called on LandedDelegate.Broadcast()
	UFUNCTION(BlueprintNativeEvent)
  void Landing(const FHitResult& Hit);

  //JumpPad Boost function (set Z velocity)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
  void JumpPadBoost(double VelocityZ);

  //override of OnJumped Function
  virtual void OnJumped_Implementation() override;

  virtual void OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal, const FVector& PreviousFloorContactNormal, const FVector& PreviousLocation, float TimeDelta) override;

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

};

