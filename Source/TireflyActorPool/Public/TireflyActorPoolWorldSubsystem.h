// Copyright Tirefly. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TireflyActorPoolWorldSubsystem.generated.h"


USTRUCT()
struct FTireflyActorPool
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<AActor*> ActorPool;
};


/** 基于世界的Actor对象池子系统 */
/** World based actor pool subsystem */
UCLASS()
class TIREFLYACTORPOOL_API UTireflyActorPoolWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

#pragma region WorldSubsystem

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

#pragma endregion


#pragma region ActorPool_Clear

public:
	// 清理所有Actor池。
	// Clear all actor pools
	UFUNCTION(BlueprintCallable, Category = "Actor Pool", Meta = (DisplayName = "Actor Pool - Clear All Pools"))
	void ActorPool_ClearAllPools();

	// 清理指定类的Actor池。
	// Clear the actor pool of specified class.
	UFUNCTION(BlueprintCallable, Category = "Actor Pool", Meta = (DisplayName = "Actor Pool - Clear Actor Pool of Class"))
	void ActorPool_ClearPoolOfClass(TSubclassOf<AActor> ActorClass);

	// 清理指定ID的Actor池。
	// Clear the actor pool of specified ID.
	UFUNCTION(BlueprintCallable, Category = "Actor Pool", Meta = (DisplayName = "Actor Pool - Clear Actor Pool of ID"))
	void ActorPool_ClearPoolOfID(FName ActorID);

#pragma endregion


#pragma region ActorPool_Fetch

protected:
	AActor* FetchActor_Internal(const TSubclassOf<AActor>& ActorClass, FName ActorID);

public:
	// 从Actor池里提取一个特定类的Actor实例。请确保要使用的对象池存在，且对象池中确实有可使用的Actor实例。
	// Extract an actor instance of a specific class from the Actor pool. Make sure that the object pool you want to use exists and that there are Actor instances available in the object pool.
	UFUNCTION(BlueprintCallable, Category = "Actor Pool", Meta = (DisplayName = "Actor Pool - Fetch Actor", DeterminesOutputType = "ActorClass"))
	AActor* K2_ActorPool_FetchActor(TSubclassOf<AActor> ActorClass, FName ActorID);

	template<typename T>
	T* ActorPool_FetchActor(TSubclassOf<T> ActorClass, FName ActorID);

	// 从Actor池里提取一个特定类的Actor实例集。请确保要使用的对象池存在，且对象池中确实有可使用的Actor实例。
	// Extract a collection of Actor instances of a specific class from the Actor pool. Make sure that the object pool you want to use exists and that there are Actor instances available in the object pool.
	UFUNCTION(BlueprintCallable, Category = "Actor Pool", Meta = (DisplayName = "Actor Pool - Fetch Actors", DeterminesOutputType = "ActorClass"))
	TArray<AActor*> K2_ActorPool_FetchActors(TSubclassOf<AActor> ActorClass, FName ActorID, int32 Count = 16);

	template<typename T>
	TArray<T*> ActorPool_FetchActors(TSubclassOf<T> ActorClass, FName ActorID, int32 Count = 16);

#pragma endregion


#pragma region ActorPool_Spawn

protected:
	AActor* SpawnActor_Internal(
		const UObject* WorldContext,
		const TSubclassOf<AActor>& ActorClass,
		FName ActorID,
		const FTransform& Transform,
		float Lifetime = -1.f,
		const ESpawnActorCollisionHandlingMethod CollisionHandling = ESpawnActorCollisionHandlingMethod::AlwaysSpawn,
		AActor* Owner = nullptr,
		APawn* Instigator = nullptr);

public:
	AActor* ActorPool_BeginDeferredActorSpawn_Internal(
		const UObject* WorldContext,
		TSubclassOf<AActor> ActorClass,
		FName ActorID,
		const FTransform& SpawnTransform,
		ESpawnActorCollisionHandlingMethod CollisionHandling = ESpawnActorCollisionHandlingMethod::AlwaysSpawn,
		AActor* Owner = nullptr,
		APawn* Instigator = nullptr);

	AActor* ActorPool_FinishSpawningActor_Internal(
		const UObject* WorldContext,
		AActor* Actor,
		const FTransform& SpawnTransform,
		float Lifetime);

	template<typename T>
	T* ActorPool_SpawnActor(
		TSubclassOf<T> ActorClass,
		FName ActorID,
		const FTransform& Transform,
		float Lifetime = -1.f,
		const ESpawnActorCollisionHandlingMethod CollisionHandling = ESpawnActorCollisionHandlingMethod::AlwaysSpawn,
		AActor* Owner = nullptr,
		APawn* Instigator = nullptr);

#pragma endregion


#pragma region ActorPool_Release

public:
	// 把Actor回收到Actor池里，如果Actor有ID（并且Actor实现了ITireflyPoolingActorInterface::GetActorID）则回到对应ID的Actor池，否则回到Actor类的Actor池。
	// Recycle the Actor back into the Actor pool. If the Actor has an ID (dn implements ITireflyPoolingActorInterface::GetActorID), return it to the ID-based Actor pool; otherwise, return it to the class-based Actor pool.
	UFUNCTION(BlueprintCallable, Category = "Actor Pool", Meta = (DisplayName = "Actor Pool - Release Actor"))
	void ActorPool_ReleaseActor(AActor* Actor);

#pragma endregion


#pragma region ActorPool_WarmUp

public:
	// 生成特定数量的指定类以及指定ID的Actor并放进Actor池中待命。
	// Spawn a specific number of Actors of a specified class and a specified ID ,and place them in the Actor pool on standby.
	UFUNCTION(BlueprintCallable, Category = "Actor Pool", Meta = (
		WorldContext = "WorldContext",
		DisplayName = "Actor Pool - Warm Up"))
	void ActorPool_WarmUp(
		const UObject* WorldContext,
		TSubclassOf<AActor> ActorClass,
		FName ActorID,
		const FTransform& Transform,
		AActor* Owner = nullptr,
		APawn* Instigator = nullptr,
		int32 Count = 16);
	
#pragma endregion


#pragma region ActorPool_Debug

public:
	// 返回在Actor类对象池中所有的Actor类型。
	// Return all Actor classes of ActorPoolOfClass.
	UFUNCTION(BlueprintPure, Category = "Actor Pool", Meta = (DisplayName = "Actor Pool - Debug Actor Classes"))
	TArray<TSubclassOf<AActor>> ActorPool_DebugActorClasses() const;

	// 返回在ActorID对象池中所有的ActorID。
	// Return all Actor IDs of ActorPoolOfID.
	UFUNCTION(BlueprintPure, Category = "Actor Pool", Meta = (DisplayName = "Actor Pool - Debug Actor IDs"))
	TArray<FName> ActorPool_DebugActorIDs() const;

	// 返回在对象池中待命的指定类的Actor的数量，如果不存在指定类的Actor的对象池，则返回-1。
	// Return the number of Actors of a specified class on standby in the object pool. If the object pool for the specified class of Actors does not exist, return -1.
	UFUNCTION(BlueprintPure, Category = "Actor Pool", Meta = (DisplayName = "Actor Pool - Debug Actor Number Of Class"))
	int32 ActorPool_DebugActorNumberOfClass(TSubclassOf<AActor> ActorClass);

	// 返回在对象池中待命的指定ID的Actor的数量，如果不存在指定ID的Actor的对象池，则返回-1。
	// Return the number of Actors of a specified ID on standby in the object pool. If the object pool for the specified ID of Actors does not exist, return -1.
	UFUNCTION(BlueprintPure, Category = "Actor Pool", Meta = (DisplayName = "Actor Pool - Debug Actor Number Of ID"))
	int32 ActorPool_DebugActorNumberOfID(FName ActorID);

#pragma endregion


#pragma region ActorPool_Declaration

protected:
	UPROPERTY()
	TMap<TSubclassOf<AActor>, FTireflyActorPool> ActorPoolOfClass;

	UPROPERTY()
	TMap<FName, FTireflyActorPool> ActorPoolOfID;

	UPROPERTY()
	TMap<AActor*, FTimerHandle> ActorLifetimeTimers;

#pragma endregion
};

#pragma region ActorPool_FunctionTemplate

template <typename T>
T* UTireflyActorPoolWorldSubsystem::ActorPool_FetchActor(TSubclassOf<T> ActorClass, FName ActorID)
{
	return Cast<T>(FetchActor_Internal(ActorClass, ActorID));
}

template <typename T>
TArray<T*> UTireflyActorPoolWorldSubsystem::ActorPool_FetchActors(TSubclassOf<T> ActorClass, FName ActorID,
	int32 Count)
{
	TArray<T*> OutActors;
	for (int32 i = 0; i < Count; ++i)
	{
		OutActors.Add(Cast<T>(FetchActor_Internal(ActorClass, ActorID)));
	}
	
	return OutActors;
}

template<typename T>
T* UTireflyActorPoolWorldSubsystem::ActorPool_SpawnActor(
	TSubclassOf<T> ActorClass,
	FName ActorID,
	const FTransform& Transform,
	float Lifetime,
	const ESpawnActorCollisionHandlingMethod CollisionHandling,
	AActor* Owner,
	APawn* Instigator)
{
	return Cast<T>(SpawnActor_Internal(ActorClass, ActorID, Transform, Lifetime, Owner, Instigator, CollisionHandling));
}

#pragma endregion