// Copyright Tirefly. All Rights Reserved.


#include "TireflyActorPoolWorldSubsystem.h"

#include "Engine/World.h"
#include "TimerManager.h"
#include "TireflyPoolingActorInterface.h"


void UTireflyActorPoolWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UTireflyActorPoolWorldSubsystem::Deinitialize()
{
	ActorPool_ClearAllPools();

	Super::Deinitialize();
}

void UTireflyActorPoolWorldSubsystem::ActorPool_ClearAllPools()
{
	for (auto Pool : ActorPoolOfClass)
	{
		for (auto Actor : Pool.Value.ActorPool)
		{
			if (IsValid(Actor))
			{
				Actor->Destroy(true);
			}
		}
	}

	for (auto Pool : ActorPoolOfID)
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
	ActorPoolOfID.Empty();
}

void UTireflyActorPoolWorldSubsystem::ActorPool_ClearPoolOfClass(TSubclassOf<AActor> ActorClass)
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

void UTireflyActorPoolWorldSubsystem::ActorPool_ClearPoolOfID(FName ActorID)
{
	if (FTireflyActorPool* Pool = ActorPoolOfID.Find(ActorID))
	{
		for (auto Actor : Pool->ActorPool)
		{
			if (IsValid(Actor))
			{
				Actor->Destroy(true);
			}
		}
		Pool->ActorPool.Empty();
		ActorPoolOfID.Remove(ActorID);
	}
}

AActor* UTireflyActorPoolWorldSubsystem::FetchActor_Internal(
	const TSubclassOf<AActor>& ActorClass,
	FName ActorID)
{
	FTireflyActorPool* Pool = ActorPoolOfID.Find(ActorID);
	if (!Pool)
	{
		Pool = ActorPoolOfClass.Find(ActorClass);
	}

	if (Pool && Pool->ActorPool.Num() > 0)
	{
		AActor* Actor = Pool->ActorPool.Pop();

		return Actor;
	}

	return nullptr;
}

AActor* UTireflyActorPoolWorldSubsystem::K2_ActorPool_FetchActor(TSubclassOf<AActor> ActorClass, FName ActorID)
{
	return ActorPool_FetchActor<AActor>(ActorClass, ActorID);
}

TArray<AActor*> UTireflyActorPoolWorldSubsystem::K2_ActorPool_FetchActors(TSubclassOf<AActor> ActorClass, FName ActorID,
                                                                           int32 Count)
{
	return ActorPool_FetchActors<AActor>(ActorClass, ActorID, Count);
}

AActor* UTireflyActorPoolWorldSubsystem::SpawnActor_Internal(
	const UObject* WorldContext,
	const TSubclassOf<AActor>& ActorClass,
	FName ActorID,
	const FTransform& Transform,
	float Lifetime,
	const ESpawnActorCollisionHandlingMethod CollisionHandling,
	AActor* Owner,
	APawn* Instigator)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
	if (!IsValid(World) || (!IsValid(ActorClass) && ActorID == NAME_None))
	{
		return nullptr;
	}

	AActor* Actor = ActorPool_FetchActor<AActor>(ActorClass, ActorID);
	if (Actor)
	{
		Actor->SetActorTransform(Transform, false, nullptr, ETeleportType::ResetPhysics);
		Actor->SetInstigator(Instigator);
		Actor->SetOwner(Owner);
		
		if (Actor->Implements<UTireflyPoolingActorInterface>())
		{
			if (ITireflyPoolingActorInterface::Execute_PoolingGetActorID(Actor) != ActorID && ActorID != NAME_None)
			{
				ITireflyPoolingActorInterface::Execute_PoolingSetActorID(Actor, ActorID);
			}
			ITireflyPoolingActorInterface::Execute_PoolingBeginPlay(Actor);
		}
	}
	else
	{
		if (!IsValid(ActorClass))
		{
			return nullptr;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = Owner;
		SpawnParameters.Instigator = Instigator;
		SpawnParameters.SpawnCollisionHandlingOverride = CollisionHandling;

		Actor = World->SpawnActor<AActor>(ActorClass, Transform, SpawnParameters);
		if (Actor->Implements<UTireflyPoolingActorInterface>())
		{
			if (ActorID != NAME_None)
			{
				ITireflyPoolingActorInterface::Execute_PoolingSetActorID(Actor, ActorID);
			}
			ITireflyPoolingActorInterface::Execute_PoolingBeginPlay(Actor);
		}
	}

	if (IsValid(Actor) && Lifetime > 0.f)
	{
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegate = FTimerDelegate::CreateLambda(
			[this, Actor]
			{
				ActorPool_ReleaseActor(Actor);
				ActorLifetimeTimers.Remove(Actor);
			});
		World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, Lifetime, false);
		ActorLifetimeTimers.Add(Actor, TimerHandle);
	}

	return Actor;
}

AActor* UTireflyActorPoolWorldSubsystem::ActorPool_BeginDeferredActorSpawn_Internal(
	const UObject* WorldContext,
	TSubclassOf<AActor> ActorClass,
	FName ActorID,
	const FTransform& SpawnTransform,
	ESpawnActorCollisionHandlingMethod CollisionHandling,
	AActor* Owner,
	APawn* Instigator)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
	if (!IsValid(World) || (!IsValid(ActorClass) && ActorID == NAME_None))
	{
		return nullptr;
	}

	auto SetActorID = [ActorID](AActor* InActor)
	{
		if (InActor->Implements<UTireflyPoolingActorInterface>())
		{
			if (ITireflyPoolingActorInterface::Execute_PoolingGetActorID(InActor) != ActorID && ActorID != NAME_None)
			{
				ITireflyPoolingActorInterface::Execute_PoolingSetActorID(InActor, ActorID);
			}
		}
	};

	AActor* Actor = ActorPool_FetchActor<AActor>(ActorClass, ActorID);
	if (Actor)
	{
		SetActorID(Actor);
		Actor->SetActorTransform(SpawnTransform, true, nullptr, ETeleportType::ResetPhysics);
		Actor->SetInstigator(Instigator);
		Actor->SetOwner(Owner);

		return Actor;
	}

	if (!IsValid(ActorClass))
	{
		return nullptr;
	}

	Actor = World->SpawnActorDeferred<AActor>(ActorClass, SpawnTransform, Owner, Instigator, CollisionHandling);
	SetActorID(Actor);

	return Actor;
}

AActor* UTireflyActorPoolWorldSubsystem::ActorPool_FinishSpawningActor_Internal(
	const UObject* WorldContext,
	AActor* Actor,
	const FTransform& SpawnTransform,
	float Lifetime)
{
	UWorld* World = WorldContext->GetWorld();
	if (!IsValid(World) || !IsValid(Actor))
	{
		return nullptr;
	}

	if ((!Actor->IsActorInitialized()))
	{
		Actor->FinishSpawning(SpawnTransform);
	}

	if (Actor->Implements<UTireflyPoolingActorInterface>())
	{
		ITireflyPoolingActorInterface::Execute_PoolingBeginPlay(Actor);
	}

	if (Lifetime > 0.f)
	{
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegate = FTimerDelegate::CreateLambda(
			[this, Actor]
			{
				ActorPool_ReleaseActor(Actor);
				ActorLifetimeTimers.Remove(Actor);
			});
		World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, Lifetime, false);
		ActorLifetimeTimers.Add(Actor, TimerHandle);
	}

	return Actor;
}

void UTireflyActorPoolWorldSubsystem::ActorPool_ReleaseActor(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}

	FName ActorID = NAME_None;
	if (Actor->Implements<UTireflyPoolingActorInterface>())
	{
		ActorID = ITireflyPoolingActorInterface::Execute_PoolingGetActorID(Actor);
		ITireflyPoolingActorInterface::Execute_PoolingEndPlay(Actor);
	}

	if (ActorID != NAME_None)
	{
		FTireflyActorPool& Pool = ActorPoolOfID.FindOrAdd(ActorID);
		Pool.ActorPool.Push(Actor);

		return;
	}

	FTireflyActorPool& Pool = ActorPoolOfClass.FindOrAdd(Actor->GetClass());	
	Pool.ActorPool.Push(Actor);	
}

void UTireflyActorPoolWorldSubsystem::ActorPool_WarmUp(const UObject* WorldContextObject,
	TSubclassOf<AActor> ActorClass, FName ActorID, const FTransform& Transform, AActor* Owner, APawn* Instigator,
	int32 Count)
{
	UWorld* World = WorldContextObject->GetWorld();

	if (!IsValid(World) || !IsValid(ActorClass) || ActorID == NAME_None || Count <= 0)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Owner;
	SpawnParameters.Instigator = Instigator;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FTireflyActorPool& Pool = ActorID != NAME_None ? ActorPoolOfID.FindOrAdd(ActorID) : ActorPoolOfClass.FindOrAdd(ActorClass);
	Pool.ActorPool.Reserve(Count);
	for (int32 i = 0; i < Count; i++)
	{
		AActor* Actor = World->SpawnActor<AActor>(ActorClass, Transform, SpawnParameters);
		if (Actor->Implements<UTireflyPoolingActorInterface>())
		{
			if (ActorID != NAME_None)
			{
				ITireflyPoolingActorInterface::Execute_PoolingSetActorID(Actor, ActorID);
			}
			ITireflyPoolingActorInterface::Execute_PoolingWarmUp(Actor);
		}

		Pool.ActorPool.Push(Actor);
	}
}

TArray<TSubclassOf<AActor>> UTireflyActorPoolWorldSubsystem::ActorPool_DebugActorClasses() const
{
	TArray<TSubclassOf<AActor>> ActorClasses;
	ActorPoolOfClass.GetKeys(ActorClasses);

	return ActorClasses;
}

TArray<FName> UTireflyActorPoolWorldSubsystem::ActorPool_DebugActorIDs() const
{
	TArray<FName> ActorIDs;
	ActorPoolOfID.GetKeys(ActorIDs);

	return ActorIDs;
}

int32 UTireflyActorPoolWorldSubsystem::ActorPool_DebugActorNumberOfClass(TSubclassOf<AActor> ActorClass)
{
	if (!ActorPoolOfClass.Contains(ActorClass))
	{
		return -1;
	}

	return ActorPoolOfClass[ActorClass].ActorPool.Num();
}

int32 UTireflyActorPoolWorldSubsystem::ActorPool_DebugActorNumberOfID(FName ActorID)
{
	if (!ActorPoolOfID.Contains(ActorID))
	{
		return -1;
	}

	return ActorPoolOfID[ActorID].ActorPool.Num();
}
