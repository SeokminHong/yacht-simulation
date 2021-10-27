// Fill out your copyright notice in the Description page of Project Settings.


#include "MyActor.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonWriter.h"
#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"

// Sets default values
AMyActor::AMyActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMyActor::StartRecord()
{
	FArchive* WriterArchive = IFileManager::Get().CreateFileWriter(*(FPaths::ProjectDir() / FileName + TEXT(".json")));
	auto Writer = TJsonWriter<TCHAR>::Create(WriterArchive);
	TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create("[{\"time\":0.12}, {}, {}]");
	TSharedPtr<FJsonValue> JsonValue = MakeShareable(new FJsonValueArray(TArray<TSharedPtr<FJsonValue>>()));
	FJsonSerializer::Deserialize(Reader, JsonValue);
	FJsonSerializer::Serialize(JsonValue->AsArray(), Writer);
	Writer->Close();
	delete WriterArchive;
	//for (auto&& v : JsonValue->Values)
	//{
	//	UE_LOG(LogClass, Log, TEXT("Obj Valid: %s"), *v.Key);
	//}
	auto arr = JsonValue->AsArray();
	UE_LOG(LogClass, Log, TEXT("Len: %d"), arr.Num())
	auto obj = arr[0]->AsObject();
	float time = obj->GetNumberField("time");
	UE_LOG(LogClass, Log, TEXT("Read Time: %f"), time)
	if (bStarted)
	{
		return;
	}
	Transforms.Reset();
	Elapsed = 0.f;
	TArray<FTransform> v;
	for (AActor* d : Dice)
	{
		v.Emplace(d->GetActorTransform());
	}
	Transforms.Emplace(Elapsed, MoveTemp(v));
	bStarted = true;
}

void AMyActor::StopRecord()
{
	bStarted = false;
	TArray<FString> Output;
	Output.Emplace(TEXT("["));

	UE_LOG(LogClass, Log, TEXT("End Game!"));
	for (int i = 0; i < 5; i++)
	{
		TransformType& t = Transforms[i];
		FString v = FString::Printf(TEXT("{\"elapsed\":%f,["), t.Key);
		for (FTransform& x : t.Value)
		{
			// AActor* d = Dice[i];
			/**
			 *   R   P   Y
			 *   0   0   x => 1
			 * -90   0   x => 2
			 *   x  90   x => 3
			 *   x -90   x => 4
			 *  90   0   x => 5
			 * 180   0   x => 6
			 */
			// FRotator Rot = d->GetActorRotation();
			// if (FMath::IsNearlyEqual(Rot.Roll, -90.f, 5.f) && FMath::IsNearlyEqual(Rot.Pitch, 0.f, 5.f))
			// {
			// 	UE_LOG(LogClass, Log, TEXT("%s: 2"), *d->GetName());
			// }
			// else if (FMath::IsNearlyEqual(Rot.Pitch, 90.f, 5.f))
			// {
			// 	UE_LOG(LogClass, Log, TEXT("%s: 3"), *d->GetName());
			// }
			// else if (FMath::IsNearlyEqual(Rot.Pitch, -90.f, 5.f))
			// {
			// 	UE_LOG(LogClass, Log, TEXT("%s: 4"), *d->GetName());
			// }
			// else if (FMath::IsNearlyEqual(Rot.Roll, 90.f, 5.f) && FMath::IsNearlyEqual(Rot.Pitch, 0.f, 5.f))
			// {
			// 	UE_LOG(LogClass, Log, TEXT("%s: 5"), *d->GetName());
			// }
			// else if (FMath::IsNearlyEqual(FMath::Abs(Rot.Roll), 180.f, 5.f) && FMath::IsNearlyEqual(Rot.Pitch, 0.f, 5.f))
			// {
			// 	UE_LOG(LogClass, Log, TEXT("%s: 6"), *d->GetName());
			// }
			// else
			// {
			// 	UE_LOG(LogClass, Log, TEXT("%s: 1"), *d->GetName());
			// }
			// UE_LOG(LogClass, Log, TEXT("%s - %s"), *d->GetName(), *d->GetActorRotation().ToString());
		}
		v += "]}";
		if (i != 4)
		{
			v += ',';
		}
		Output.Emplace(MoveTemp(v));
	}
	//Output.Emplace(TEXT("}"));
	//FFileHelper::SaveStringArrayToFile(Output, *(FPaths::ProjectDir() / FileName + TEXT(".json")));
}

void AMyActor::PlayRecord()
{

}

// Called every frame
void AMyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bPlaying)
	{

		return;
	}
	if (!bStarted)
	{
		return;
	}
	Elapsed += DeltaTime;

	TArray<FTransform> v;
	for (AActor* d : Dice)
	{
		v.Emplace(d->GetActorTransform());
	}
	Transforms.Emplace(Elapsed, MoveTemp(v));
}

