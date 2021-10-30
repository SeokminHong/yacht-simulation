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
	Timestamps.Reset();
	Elapsed = 0.f;
	TArray<FTransform> v;
	for (AActor* d : Dice)
	{
		Cast<UPrimitiveComponent>(d->GetRootComponent())->SetSimulatePhysics(true);
		v.Emplace(d->GetActorTransform());
	}
	Timestamps.Emplace(Elapsed, MoveTemp(v));
	bStarted = true;
	bPlaying = false;
}

TSharedPtr<FJsonValue> QuatToJson(FQuat Rot)
{
	TSharedPtr<FJsonObject> RotObj = MakeShareable(new FJsonObject());
	RotObj->SetNumberField(TEXT("qx"), Rot.X);
	RotObj->SetNumberField(TEXT("qy"), Rot.Y);
	RotObj->SetNumberField(TEXT("qz"), Rot.Z);
	RotObj->SetNumberField(TEXT("qw"), Rot.W);
	return TSharedPtr<FJsonValue>(new FJsonValueObject(MoveTemp(RotObj)));
}

void AMyActor::StopRecord()
{
	bStarted = false;

	FArchive* WriterArchive = IFileManager::Get().CreateFileWriter(*(FPaths::ProjectDir() / FileName + TEXT(".json")));
	auto Writer = TJsonWriter<ANSICHAR>::Create(WriterArchive);

	TSharedPtr<FJsonObject> Root = MakeShareable(new FJsonObject());
	TArray<TSharedPtr<FJsonValue>> OffsetArray;

	TArray<FQuat> Offsets;
	for (auto* d : Dice)
	{
		FRotator Rot = d->GetActorRotation();
		FQuat Normalized = FRotator{ 0.f, Rot.Yaw, 0.f }.Quaternion();
		Offsets.Emplace(Normalized * Rot.Quaternion().Inverse());

		TArray<TSharedPtr<FJsonValue>> OffsetValueArray;
		/**
		 *   R   P
		 *   0   0 => 1
		 * -90   0 => 2
		 *   x  90 => 3
		 *   x -90 => 4
		 *  90   0 => 5
		 * 180   0 => 6
		 */
		OffsetValueArray.Append({
			QuatToJson(FQuat::Identity),
			QuatToJson(FRotator{ 0.f, Rot.Yaw, -90.f }.Quaternion() * Normalized.Inverse()),
			QuatToJson(FRotator{ 90.f, Rot.Yaw, Rot.Roll }.Quaternion() * Normalized.Inverse()),
			QuatToJson(FRotator{ -90.f, Rot.Yaw, Rot.Roll }.Quaternion() * Normalized.Inverse()),
			QuatToJson(FRotator{ 0.f, Rot.Yaw, 90.f }.Quaternion() * Normalized.Inverse()),
			QuatToJson(FRotator{ 0.f, Rot.Yaw, 180.f }.Quaternion() * Normalized.Inverse()),
		});
		OffsetArray.Emplace(MakeShared<FJsonValueArray>(MoveTemp(OffsetValueArray)));
	}
	Root->SetArrayField(TEXT("offsets"), MoveTemp(OffsetArray));

	TSharedPtr<FJsonValueArray> _TimestampArray = MakeShareable(new FJsonValueArray(TArray<TSharedPtr<FJsonValue>>()));
	auto TimestampArray = _TimestampArray->AsArray();

	UE_LOG(LogClass, Log, TEXT("End Game!"));
	for (FTimestampType& Timestamp : Timestamps)
	{
		TSharedPtr<FJsonObject> TimestampElem = MakeShareable(new FJsonObject());
		TimestampElem->SetNumberField(TEXT("time"), Timestamp.Key);
		TArray<TSharedPtr<FJsonValue>> TransformArray;
		int i = 0;
		for (FTransform& Transform : Timestamp.Value)
		{
			TSharedPtr<FJsonObject> tfObj = MakeShareable(new FJsonObject());
			FVector Loc = Transform.GetLocation();
			FQuat Rot = (Offsets[i] * Transform.GetRotation());
			FRotator _r = Rot.Rotator();
			_r.Yaw = -_r.Yaw;
			Rot = _r.Quaternion();
			tfObj->SetNumberField(TEXT("x"), Loc.X / 25.f);
			tfObj->SetNumberField(TEXT("y"), Loc.Y / 25.f);
			tfObj->SetNumberField(TEXT("z"), Loc.Z / 25.f);
			tfObj->SetNumberField(TEXT("qx"), Rot.X);
			tfObj->SetNumberField(TEXT("qy"), Rot.Y);
			tfObj->SetNumberField(TEXT("qz"), Rot.Z);
			tfObj->SetNumberField(TEXT("qw"), Rot.W);
			
			TransformArray.Emplace(MakeShareable(new FJsonValueObject(MoveTemp(tfObj))));
			i++;
		}
		TimestampElem->SetArrayField(TEXT("tf"), MoveTemp(TransformArray));
		TimestampArray.Emplace(MakeShareable(new FJsonValueObject(MoveTemp(TimestampElem))));
	}
	Root->SetArrayField(TEXT("timestamps"), MoveTemp(TimestampArray));
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);
	Writer->Close();
	delete WriterArchive;
}

void AMyActor::PlayRecord()
{
	bPlaying = true;
	Elapsed = 0.f;
	FArchive* ReaderArchive = IFileManager::Get().CreateFileReader(*(FPaths::ProjectDir() / FileName + TEXT(".json")));
	auto JsonReader = TJsonReader<ANSICHAR>::Create(ReaderArchive);
	TSharedPtr<FJsonValue> JsonValue = MakeShareable(new FJsonValueArray(TArray<TSharedPtr<FJsonValue>>()));
	FJsonSerializer::Deserialize(JsonReader, JsonValue);
	auto RootObj = JsonValue->AsObject();

	auto OffsetArray = RootObj->GetArrayField(TEXT("offsets"));
	TArray<TArray<FQuat>> Offsets;
	for (const auto& o : OffsetArray)
	{
		auto v = o->AsArray();
		TArray<FQuat> arr;
		for (const auto& q : v)
		{
			FQuat rot;
			auto rotObj = q->AsObject();
			rot.X = rotObj->GetNumberField(TEXT("qx"));
			rot.Y = rotObj->GetNumberField(TEXT("qy"));
			rot.Z = rotObj->GetNumberField(TEXT("qz"));
			rot.W = rotObj->GetNumberField(TEXT("qw"));
			arr.Emplace(MoveTemp(rot));
		}
		Offsets.Emplace(MoveTemp(arr));
	}

	auto TimestampArray = RootObj->GetArrayField(TEXT("timestamps"));
	Timestamps.Empty(TimestampArray.Num());
	for (auto&& Timestamp : TimestampArray)
	{
		auto timestamp = Timestamp->AsObject();
		FTimestampType t;
		t.Key = timestamp->GetNumberField(TEXT("time"));
		auto& tf = timestamp->GetArrayField(TEXT("tf"));
		int i = 0;
		for (const auto& x : tf)
		{
			FVector l;
			FQuat r;
			const auto& v = x->AsObject();
			l.X = v->GetNumberField(TEXT("x")) * -25.f;
			l.Y = v->GetNumberField(TEXT("y")) * 25.f;
			l.Z = v->GetNumberField(TEXT("z")) * 25.f;
			r.X = v->GetNumberField(TEXT("qx"));
			r.Y = v->GetNumberField(TEXT("qy"));
			r.Z = v->GetNumberField(TEXT("qz"));
			r.W = v->GetNumberField(TEXT("qw"));
			t.Value.Emplace(Offsets[i][Eyes[i] - 1] * MoveTemp(r), MoveTemp(l), FVector{ Scale });
			i++;
		}
		Timestamps.Emplace(MoveTemp(t));
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
		for (int Index = 0; Index < Timestamps.Num() - 1; Index++)
		{
			if (Elapsed > Timestamps[Index].Key)
			{
				continue;
			}
			for (int J = 0; J < Timestamps[Index].Value.Num(); J++)
			{
				FTransform t1 = Timestamps[Index].Value[J];
				FTransform t2 = Timestamps[Index + 1].Value[J];
				float Alpha = (Elapsed - Timestamps[Index].Key) / (Timestamps[Index + 1].Key - Timestamps[Index].Key);
				const FRotator DeltaAngle = t2.Rotator() - t1.Rotator();
				FTransform v{ FQuat(t1.Rotator() + Alpha * DeltaAngle), FMath::Lerp(t1.GetLocation(), t2.GetLocation(), Alpha), FVector{ Scale } };
				//v.Blend(Timestamps[Index].Value[J], Timestamps[Index + 1].Value[J], (Elapsed - Timestamps[Index].Key) / (Timestamps[Index + 1].Key - Timestamps[Index].Key));
				Dice[J]->SetActorTransform(MoveTemp(v));
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
	Timestamps.Emplace(Elapsed, MoveTemp(v));
}

