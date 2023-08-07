// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

class  UBuildComponent;

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

	UPROPERTY(EditAnywhere, Category = Axe)
	TSubclassOf<class AAxe> AxeClass;
	UPROPERTY(VisibleAnywhere)
	AAxe* Axe;

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

	// Axe
	UFUNCTION(Server, Reliable)
	void ServerSwingAxe();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSwingAxe();

	bool bCalculateAttackCollision = false;
	bool bIsBuildModeOn = false;

	UPROPERTY(EditAnywhere, Category = "Camera")
	TSubclassOf<class UCameraShakeBase> SwingCameraShake;


public:	
	FORCEINLINE bool GetIsSwingingAxe() const { return bIsSwingingAxe; }
	FORCEINLINE void SetIsSwingingAxe(bool isSwinging) { bIsSwingingAxe = isSwinging; }
	FORCEINLINE AAxe* GetAxe() const { return Axe; }

};
