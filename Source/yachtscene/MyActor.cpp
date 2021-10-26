// Fill out your copyright notice in the Description page of Project Settings.


#include "MyActor.h"
#include "Misc/FileHelper.h"

// Sets default values
AMyActor::AMyActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMyActor::BeginPlay()
{
	Super::BeginPlay();
	
	Transforms.Reset(5);
	Elapsed = 0.f;
	TArray<TransformType> v;
	for (AActor* d : Dice)
	{
		v.Emplace(Elapsed, d->GetActorTransform());
	}
	Transforms.Emplace(MoveTemp(v));
}

void AMyActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	//TArray<FString> Output;
	//Output.Emplace(TEXT("{"));

	/**
     *   0   0   x => 1
     * -90   0   x => 2
     *   x  90   0 => 3
     *   x -90 180 => 4
     *  90   0   x => 5
	 * 180   0   x => 6
	 */



	//Output.Emplace(TEXT("}"));
	//FFileHelper::SaveStringArrayToFile(Output, *(FPaths::ProjectDir() / FileName + TEXT(".json")));
	UE_LOG(LogClass, Log, TEXT("End Game!"));
	for (AActor* d : Dice)
	{
		UE_LOG(LogClass, Log, TEXT("%s - %s"), *d->GetName(), *d->GetActorRotation().ToString());
	}
}

// Called every frame
void AMyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Elapsed += DeltaTime;

	TArray<TransformType> v;
	for (AActor* d : Dice)
	{
		v.Emplace(Elapsed, d->GetActorTransform());
	}
	Transforms.Emplace(MoveTemp(v));
}

