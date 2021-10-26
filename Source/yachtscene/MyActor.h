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

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	UPROPERTY(EditAnywhere)
	TArray<AActor*> Dice;

	UPROPERTY(EditAnywhere)
	FString FileName;

	using TransformType = TPair<float, FTransform>;
	TArray<TArray<TransformType>> Transforms;

	float Elapsed = 0.f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
