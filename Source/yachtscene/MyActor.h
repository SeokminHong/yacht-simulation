// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyActor.generated.h"

UCLASS()
class YACHTSCENE_API AMyActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyActor();

	UFUNCTION(BlueprintCallable)
	void StartRecord();
	UFUNCTION(BlueprintCallable)
	void StopRecord();
	UFUNCTION(BlueprintCallable)
	void PlayRecord();

protected:
	bool bStarted = false;
	bool bPlaying = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AActor*> Dice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int> Eyes{ 1, 1, 1, 1, 1 };

	UPROPERTY(EditAnywhere)
	FString FileName;

	UPROPERTY(EditAnywhere)
	float Scale;

	using FTimestampType = TPair<float, TArray<FTransform>>;
	TArray<FTimestampType> Timestamps;

	float Elapsed = 0.f;
	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
