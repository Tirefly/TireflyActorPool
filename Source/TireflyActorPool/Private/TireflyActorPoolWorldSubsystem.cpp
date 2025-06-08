// Copyright Tirefly. All Rights Reserved.


#include "TireflyActorPoolWorldSubsystem.h"

#include "Engine/World.h"
#include "TimerManager.h"
#include "TireflyActorPoolLogChannels.h"
#include "TireflyPoolingActorInterface.h"


void UTireflyActorPoolWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UTireflyActorPoolWorldSubsystem::Deinitialize()
{
	ClearAllActorPools();

	Super::Deinitialize();
}

void UTireflyActorPoolWorldSubsystem::ClearAllActorPools()
{
	FScopeLock Lock(&PoolLock);
	
	// 批量处理Class池
	for (auto& Pool : ActorPoolOfClass)
	{
		for (auto Actor : Pool.Value.ActorPool)
		{
			if (IsValid(Actor))
			{
				Actor->Destroy(true);
			}
		}
	}

	for (auto Pool : ActorPoolOfId)
	{
		for (auto Actor : Pool.Value.ActorPool)
		{
			if (IsValid(Actor))
			{
				Actor->Destroy(true);
			}
		}
	}

	ActorPoolOfClass.Empty();
	ActorPoolOfId.Empty();
}

void UTireflyActorPoolWorldSubsystem::ClearActorPoolsOfClass(TSubclassOf<AActor> ActorClass)
{
	if (FTireflyActorPool* Pool = ActorPoolOfClass.Find(ActorClass))
	{
		for (auto Actor : Pool->ActorPool)
		{
			if (IsValid(Actor))
			{
				Actor->Destroy(true);
			}
		}
		Pool->ActorPool.Empty();
		ActorPoolOfClass.Remove(ActorClass);
	}
}

void UTireflyActorPoolWorldSubsystem::ClearActorPoolOfId(FName ActorId)
{
	if (FTireflyActorPool* Pool = ActorPoolOfId.Find(ActorId))
	{
		for (auto Actor : Pool->ActorPool)
		{
			if (IsValid(Actor))
			{
				Actor->Destroy(true);
			}
		}
		Pool->ActorPool.Empty();
		ActorPoolOfId.Remove(ActorId);
	}
}

AActor* UTireflyActorPoolWorldSubsystem::FetchActorFromPool(const TSubclassOf<AActor>& ActorClass, FName ActorId)
{
	if (!ActorClass)
	{
		UE_LOG(LogTireflyActorPool, Error, TEXT("[%s] Invalid ActorClass"), *FString(__FUNCTION__));
		return nullptr;
	}

	if (FTireflyActorPool* Pool = (ActorId != NAME_None) ? ActorPoolOfId.Find(ActorId) : ActorPoolOfClass.Find(ActorClass))
	{
		return Pool->ActorPool.IsEmpty() ? nullptr : Pool->ActorPool.Pop();
	}

	return nullptr;
}

AActor* UTireflyActorPoolWorldSubsystem::SpawnActor_Internal(
	const TSubclassOf<AActor>& ActorClass,
	FName ActorId,
	const FTransform& Transform,
	const FInstancedStruct* InitialData,
	float Lifetime,
	const ESpawnActorCollisionHandlingMethod CollisionHandling,
	AActor* Owner,
	APawn* Instigator)
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		UE_LOG(LogTireflyActorPool, Error, TEXT("[%s] Invalid World"), *FString(__FUNCTION__));
		return nullptr;
	}

	if (!IsValid(ActorClass))
	{
		UE_LOG(LogTireflyActorPool, Error, TEXT("[%s] Invalid ActorClass"), *FString(__FUNCTION__));
		return nullptr;
	}

	if (!ActorClass->ImplementsInterface(UTireflyPoolingActorInterface::StaticClass()))
	{
		UE_LOG(LogTireflyActorPool, Error, TEXT("[%s] ActorClass %s does not implement UTireflyPoolingActorInterface"),
			*FString(__FUNCTION__),
			*ActorClass->GetName());
		return nullptr;
	}

	FScopeLock Lock(&PoolLock);

	AActor* Actor = FetchActorFromPool(ActorClass, ActorId);
	if (Actor)
	{
		Actor->SetActorTransform(Transform, false, nullptr, ETeleportType::ResetPhysics);
		Actor->SetInstigator(Instigator);
		Actor->SetOwner(Owner);
	}
	else
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = Owner;
		SpawnParameters.Instigator = Instigator;
		SpawnParameters.SpawnCollisionHandlingOverride = CollisionHandling;

		Actor = World->SpawnActor<AActor>(ActorClass, Transform, SpawnParameters);
		if (!IsValid(Actor))
		{
			UE_LOG(LogTireflyActorPool, Error, TEXT("[%s] Failed to spawn ActorClass %s"),
				*FString(__FUNCTION__),
				*ActorClass->GetName());
			return nullptr;
		}

		if (ActorId != NAME_None)
		{
			ITireflyPoolingActorInterface::Execute_PoolingSetActorId(Actor, ActorId);
		}
	}

	ITireflyPoolingActorInterface::Execute_PoolingBeginPlay(Actor);
	if (InitialData)
	{
		ITireflyPoolingActorInterface::Execute_PoolingInitialized(Actor, *InitialData);
	}

	if (Lifetime > 0.f)
	{		
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegate = FTimerDelegate::CreateLambda(
			[this, Actor]
			{
				RecycleActorToPool(Actor);
				ActorLifetimeTimers.Remove(Actor);
			});
		World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, Lifetime, false);
		ActorLifetimeTimers.Add(Actor, TimerHandle);
	}

	return Actor;
}

void UTireflyActorPoolWorldSubsystem::RecycleActorToPool(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		UE_LOG(LogTireflyActorPool, Warning, TEXT("[%s] Invalid Actor"), *FString(__FUNCTION__));
		return;
	}

	FScopeLock Lock(&PoolLock);
	
	FName ActorId = NAME_None;
	if (Actor->Implements<UTireflyPoolingActorInterface>())
	{
		ActorId = ITireflyPoolingActorInterface::Execute_PoolingGetActorId(Actor);
		ITireflyPoolingActorInterface::Execute_PoolingEndPlay(Actor);
	}

	if (ActorId != NAME_None)
	{
		FTireflyActorPool& Pool = ActorPoolOfId.FindOrAdd(ActorId);
		Pool.ActorPool.Push(Actor);

		return;
	}

	FTireflyActorPool& Pool = ActorPoolOfClass.FindOrAdd(Actor->GetClass());	
	Pool.ActorPool.Push(Actor);	
}

void UTireflyActorPoolWorldSubsystem::WarmUpActorPool(
	TSubclassOf<AActor> ActorClass,
	FName ActorId,
	int32 Count)
{
	if (!IsValid(ActorClass))
	{
		UE_LOG(LogTireflyActorPool, Warning, TEXT("[%s] Invalid ActorClass"), *FString(__FUNCTION__));
		return;
	}

	if (!ActorClass->ImplementsInterface(UTireflyPoolingActorInterface::StaticClass()))
	{
		UE_LOG(LogTireflyActorPool, Warning, TEXT("[%s] ActorClass %s does not implement UTireflyPoolingActorInterface"),
			*FString(__FUNCTION__),
			*ActorClass->GetName());
		return;
	}

	if (Count <= 0)
	{
		UE_LOG(LogTireflyActorPool, Warning, TEXT("[%s] Count must be greater than 0"), *FString(__FUNCTION__));
		return;
	}
	
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		UE_LOG(LogTireflyActorPool, Error, TEXT("[%s] Invalid World"), *FString(__FUNCTION__));
		return;
	}

	FScopeLock Lock(&PoolLock);

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FTireflyActorPool& Pool = ActorId != NAME_None ? ActorPoolOfId.FindOrAdd(ActorId) : ActorPoolOfClass.FindOrAdd(ActorClass);
	Pool.ActorPool.Reserve(Pool.ActorPool.Num() + Count);
	
	for (int32 i = 0; i < Count; i++)
	{
		AActor* Actor = World->SpawnActor<AActor>(ActorClass, FTransform::Identity, SpawnParameters);
		if (!IsValid(Actor))
		{
			UE_LOG(LogTireflyActorPool, Error, TEXT("[%s] Failed to spawn ActorClass %s"),
				*FString(__FUNCTION__),
				*ActorClass->GetName());
			continue;
		}
		
		if (ActorId != NAME_None)
		{
			ITireflyPoolingActorInterface::Execute_PoolingSetActorId(Actor, ActorId);
		}
		ITireflyPoolingActorInterface::Execute_PoolingWarmUp(Actor);

		Pool.ActorPool.Push(Actor);
	}
}

TArray<TSubclassOf<AActor>> UTireflyActorPoolWorldSubsystem::Debug_GetAllActorPoolClasses() const
{
	TArray<TSubclassOf<AActor>> ActorClasses;
	ActorPoolOfClass.GetKeys(ActorClasses);

	return ActorClasses;
}

TArray<FName> UTireflyActorPoolWorldSubsystem::Debug_GetAllActorPoolIds() const
{
	TArray<FName> ActorIds;
	ActorPoolOfId.GetKeys(ActorIds);

	return ActorIds;
}

int32 UTireflyActorPoolWorldSubsystem::Debug_GetActorNumberOfClassPool(const TSubclassOf<AActor>& ActorClass) const
{
	if (!ActorPoolOfClass.Contains(ActorClass))
	{
		return -1;
	}

	return ActorPoolOfClass[ActorClass].ActorPool.Num();
}

int32 UTireflyActorPoolWorldSubsystem::Debug_GetActorNumberOfIdPool(FName ActorId) const
{
	if (!ActorPoolOfId.Contains(ActorId))
	{
		return -1;
	}

	return ActorPoolOfId[ActorId].ActorPool.Num();
}
