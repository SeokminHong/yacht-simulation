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
	if (bStarted)
	{
		return;
	}
	Transforms.Reset();
	Elapsed = 0.f;
	TArray<FTransform> v;
	for (AActor* d : Dice)
	{
		Cast<UPrimitiveComponent>(d->GetRootComponent())->SetSimulatePhysics(true);
		v.Emplace(d->GetActorTransform());
	}
	Transforms.Emplace(Elapsed, MoveTemp(v));
	bStarted = true;
	bPlaying = false;
}

void AMyActor::StopRecord()
{
	bStarted = false;

	/**
	 *   R   P
	 *   0   0 => 1
	 * -90   0 => 2
	 *   x  90 => 3
	 *   x -90 => 4
	 *  90   0 => 5
	 * 180   0 => 6
	 */
	TArray<FQuat> Offsets;
	for (auto* d : Dice)
	{
		FRotator Rot = d->GetActorRotation();
		Offsets.Emplace(FRotator{ 0.f, Rot.Yaw, 0.f }.Quaternion() * Rot.Quaternion().Inverse());
	}

	FArchive* WriterArchive = IFileManager::Get().CreateFileWriter(*(FPaths::ProjectDir() / FileName + TEXT(".json")));
	auto Writer = TJsonWriter<TCHAR>::Create(WriterArchive);
	TSharedPtr<FJsonValueArray> root = MakeShareable(new FJsonValueArray(TArray<TSharedPtr<FJsonValue>>()));
	auto r = root->AsArray();

	UE_LOG(LogClass, Log, TEXT("End Game!"));
	for (TransformType& t : Transforms)
	{
		TSharedPtr<FJsonObject> o = MakeShareable(new FJsonObject());
		o->SetNumberField(TEXT("time"), t.Key);
		TSharedPtr<FJsonValueArray> tf = MakeShareable(new FJsonValueArray(TArray<TSharedPtr<FJsonValue>>()));
		auto a = tf->AsArray();
		int i = 0;
		for (FTransform& x : t.Value)
		{
			TSharedPtr<FJsonObject> tfObj = MakeShareable(new FJsonObject());
			FVector Loc = x.GetLocation();
			FRotator Rot = (Offsets[i] * x.GetRotation()).Rotator();
			tfObj->SetNumberField(TEXT("x"), Loc.X);
			tfObj->SetNumberField(TEXT("y"), Loc.Y);
			tfObj->SetNumberField(TEXT("z"), Loc.Z);
			tfObj->SetNumberField(TEXT("rol"), Rot.Roll);
			tfObj->SetNumberField(TEXT("pit"), Rot.Pitch);
			tfObj->SetNumberField(TEXT("yaw"), Rot.Yaw);
			
			a.Emplace(MakeShareable(new FJsonValueObject(MoveTemp(tfObj))));
			i++;
		}
		o->SetArrayField(TEXT("tf"), MoveTemp(a));
		r.Emplace(MakeShareable(new FJsonValueObject(MoveTemp(o))));
	}
	FJsonSerializer::Serialize(r, Writer);
	Writer->Close();
	delete WriterArchive;
}

void AMyActor::PlayRecord()
{
	bPlaying = true;
	Elapsed = 0.f;
	FArchive* ReaderArchive = IFileManager::Get().CreateFileReader(*(FPaths::ProjectDir() / FileName + TEXT(".json")));
	auto JsonReader = TJsonReader<TCHAR>::Create(ReaderArchive);
	TSharedPtr<FJsonValue> JsonValue = MakeShareable(new FJsonValueArray(TArray<TSharedPtr<FJsonValue>>()));
	FJsonSerializer::Deserialize(JsonReader, JsonValue);
	auto RootArr = JsonValue->AsArray();
	Transforms.Empty(RootArr.Num());
	for (auto&& Value : RootArr)
	{
		auto timestamp = Value->AsObject();
		TransformType t;
		t.Key = timestamp->GetNumberField(TEXT("time"));
		auto& tf = timestamp->GetArrayField(TEXT("tf"));
		for (const auto& x : tf)
		{
			FVector l;
			FRotator r;
			const auto& v = x->AsObject();
			l.X = v->GetNumberField(TEXT("x"));
			l.Y = v->GetNumberField(TEXT("y"));
			l.Z = v->GetNumberField(TEXT("z"));
			r.Roll = v->GetNumberField(TEXT("rol"));
			r.Pitch = v->GetNumberField(TEXT("pit"));
			r.Yaw = v->GetNumberField(TEXT("yaw"));
			t.Value.Emplace(MoveTemp(r), MoveTemp(l), FVector{ Scale });
		}
		Transforms.Emplace(MoveTemp(t));
	}
	for (auto d : Dice)
	{
		Cast<UPrimitiveComponent>(d->GetRootComponent())->SetSimulatePhysics(false);
	}
	ReaderArchive->Close();
}

// Called every frame
void AMyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Elapsed += DeltaTime;
	if (bPlaying)
	{
		for (int Index = 0; Index < Transforms.Num() - 1; Index++)
		{
			if (Elapsed > Transforms[Index].Key)
			{
				continue;
			}
			for (int J = 0; J < Transforms[Index].Value.Num(); J++)
			{
				FTransform v;
				v.Blend(Transforms[Index].Value[J], Transforms[Index + 1].Value[J], (Elapsed - Transforms[Index].Key) / (Transforms[Index + 1].Key - Transforms[Index].Key));
				Dice[J]->SetActorTransform(v);
			}
			break;
		}
		return;
	}
	if (!bStarted)
	{
		return;
	}

	TArray<FTransform> v;
	for (AActor* d : Dice)
	{
		v.Emplace(d->GetActorTransform());
	}
	Transforms.Emplace(Elapsed, MoveTemp(v));
}

