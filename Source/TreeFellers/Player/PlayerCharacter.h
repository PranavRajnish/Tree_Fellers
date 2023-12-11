// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

class  UBuildComponent;
class ADoor;
class AAxe;
class AHammer;

UCLASS()
class TREEFELLERS_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	void StartCalculateAttackCollision();
	UFUNCTION(BlueprintCallable)
	void StopCalculateAttackCollision();
	void AxeImpact();


protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* PlayerCamera;
	UPROPERTY(VisibleAnywhere);
	UBuildComponent* BuildComponent;

	UPROPERTY(EditAnywhere, Category = Tools)
	TSubclassOf<AAxe> AxeClass;
	UPROPERTY(VisibleAnywhere)
	AAxe* Axe;

	UPROPERTY(EditAnywhere, Category = Tools)
		TSubclassOf<AHammer> HammerClass;
	UPROPERTY(VisibleAnywhere)
		AHammer* Hammer;

	UPROPERTY(VisibleAnywhere)
	APlayerController* PlayerController;

	// Animation
	UPROPERTY(EditAnywhere, Category = Animation)
	class UAnimMontage* AxeSwing;

	UPROPERTY(Replicated)
	bool bIsSwingingAxe = false;

	// Player Input
	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void Turn(float AxisValue);
	void LookUp(float AxisValue);
	void AttackButtonPressed();
	void BuildButtonPressed();
	void PlaceBuildPressed();
	void MouseWheelUp();
	void MouseWheelDown();
	void InteractButtonPressed();
	void RotateBuildClockwise();
	void RotateBuildAntiClockwise();
	void StopRotatingBuild();

	// Axe
	UFUNCTION(Server, Reliable)
	void ServerSwingTool();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSwingTool();

	// Hammer
	//UFUNCTION(Server, Reliable)
	//	void ServerSwingHammer();
	//UFUNCTION(NetMulticast, Reliable)
	//	void MulticastSwingHammer();

	bool bCalculateAttackCollision = false;
	bool bIsBuildModeOn = false;
	UPROPERTY(EditAnywhere, Category = "Camera")
	TSubclassOf<class UCameraShakeBase> SwingCameraShake;
	
	void SwitchTool(bool bSwitchToHammer);
	UFUNCTION(Server, Reliable)
		void ServerSwitchTool(bool bSwitchToHammer);
	UFUNCTION(NetMulticast, Reliable)
		void MulticastSwitchTool(bool bSwitchToHammer);

	void SetToolVisibility(bool visibility);

	// Interaction
	UPROPERTY()
	ADoor* InteractionObject = nullptr;


public:	
	FORCEINLINE bool GetIsSwingingAxe() const { return bIsSwingingAxe; }
	FORCEINLINE void SetIsSwingingAxe(bool isSwinging) { bIsSwingingAxe = isSwinging; }
	FORCEINLINE AAxe* GetAxe() const { return Axe; }
	FORCEINLINE ADoor* GetInteractionObject() const { return InteractionObject; }
	FORCEINLINE void SetInteractionObject(ADoor* NewInteractionObject) { InteractionObject = NewInteractionObject; }
	FORCEINLINE bool GetIsInBuildMode() const { return bIsBuildModeOn; }

};
