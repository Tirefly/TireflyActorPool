// Copyright Tirefly. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "StructUtils/InstancedStruct.h"
#include "TireflyActorPoolWorldSubsystem.generated.h"



// Actor对象池
USTRUCT()
struct FTireflyActorPool
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<AActor*> ActorPool;
};



// 基于世界子系统的Actor对象池子系统
UCLASS()
class TIREFLYACTORPOOL_API UTireflyActorPoolWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

#pragma region WorldSubsystem

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

private:
	// 对象池操作的线程安全锁
	FCriticalSection PoolLock;

#pragma endregion


#pragma region ActorPool_Clear

public:
	// 清理所有Actor池
	UFUNCTION(BlueprintCallable, Category = "Tirefly Actor Pool")
	void ClearAllActorPools();

	// 清理指定类型的Actor池
	UFUNCTION(BlueprintCallable, Category = "Tirefly Actor Pool", Meta = (DisplayName = "Clear Actor Pools (Class)"))
	void ClearActorPoolsOfClass(TSubclassOf<AActor> ActorClass);

	// 清理指定Id的Actor池
	UFUNCTION(BlueprintCallable, Category = "Tirefly Actor Pool", Meta = (DisplayName = "Clear Actor Pools (Id)"))
	void ClearActorPoolOfId(FName ActorId);

#pragma endregion


#pragma region ActorPool_Spawn

protected:
	AActor* FetchActorFromPool(const TSubclassOf<AActor>& ActorClass, FName ActorId);
	
	AActor* SpawnActor_Internal(
		const TSubclassOf<AActor>& ActorClass,
		FName ActorId,
		const FTransform& Transform,
		const FInstancedStruct* InitialData = nullptr,
		float Lifetime = -1.f,
		const ESpawnActorCollisionHandlingMethod CollisionHandling = ESpawnActorCollisionHandlingMethod::AlwaysSpawn,
		AActor* Owner = nullptr,
		APawn* Instigator = nullptr);

public:
	template<typename T>
	T* SpawnActorFromPool(
		TSubclassOf<T> ActorClass,
		FName ActorId,
		const FTransform& Transform,
		const FInstancedStruct* InitialData = nullptr,
		float Lifetime = -1.f,
		const ESpawnActorCollisionHandlingMethod CollisionHandling = ESpawnActorCollisionHandlingMethod::AlwaysSpawn,
		AActor* Owner = nullptr,
		APawn* Instigator = nullptr);

#pragma endregion


#pragma region ActorPool_Recycle

public:
	/**
	 * 把Actor回收到Actor池里，如果Actor有Id，
	 * 并且Actor实现了 ITireflyPoolingActorInterface::GetActorId，
	 * 则回到对应Id的Actor池，
	 * 否则回到Actor类的Actor池
	 * 
	 * @param Actor 要回收的Actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Tirefly Actor Pool")
	void RecycleActorToPool(AActor* Actor);

#pragma endregion


#pragma region ActorPool_WarmUp

public:
	/**
	 * 预热特定类型（或特定Id）的对象池，生成指定数量的Actor并使其在池中待命
	 * 
	 * @param ActorClass 对象池的目标类型
	 * @param ActorId 对象池的目标Id
	 * @param Count 预热的Actor数量
	 */
	UFUNCTION(BlueprintCallable, Category = "Tirefly Actor Pool")
	void WarmUpActorPool(
		TSubclassOf<AActor> ActorClass,
		FName ActorId,
		int32 Count = 16);
	
#pragma endregion


#pragma region ActorPool_Debug

public:
	// 获取在Actor类对象池中所有的Actor类型
	UFUNCTION(BlueprintPure, Category = "Tirefly Actor Pool")
	TArray<TSubclassOf<AActor>> Debug_GetAllActorPoolClasses() const;

	// 获取在ActorId对象池中所有的ActorId
	UFUNCTION(BlueprintPure, Category = "Tirefly Actor Pool")
	TArray<FName> Debug_GetAllActorPoolIds() const;

	// 获取特定类型的对象池中剩余Actor的数量，如果不存在指定类型的对象池或者池中没有Actor实例，则返回-1
	UFUNCTION(BlueprintPure, Category = "Tirefly Actor Pool")
	int32 Debug_GetActorNumberOfClassPool(const TSubclassOf<AActor>& ActorClass) const;

	// 获取特定Id的对象池中剩余Actor的数量，如果不存在指定类型的对象池或者池中没有Actor实例，则返回-1
	UFUNCTION(BlueprintPure, Category = "Tirefly Actor Pool")
	int32 Debug_GetActorNumberOfIdPool(FName ActorId) const;

#pragma endregion


#pragma region ActorPool_Declaration

private:
	UPROPERTY()
	TMap<TSubclassOf<AActor>, FTireflyActorPool> ActorPoolOfClass;

	UPROPERTY()
	TMap<FName, FTireflyActorPool> ActorPoolOfId;

	UPROPERTY()
	TMap<AActor*, FTimerHandle> ActorLifetimeTimers;

#pragma endregion
};


#pragma region ActorPool_FunctionTemplate

template<typename T>
T* UTireflyActorPoolWorldSubsystem::SpawnActorFromPool(
	TSubclassOf<T> ActorClass,
	FName ActorId,
	const FTransform& Transform,
	const FInstancedStruct* InitialData,
	float Lifetime,
	const ESpawnActorCollisionHandlingMethod CollisionHandling,
	AActor* Owner,
	APawn* Instigator)
{
	return Cast<T>(SpawnActor_Internal(ActorClass, ActorId, Transform, InitialData, Lifetime, CollisionHandling, Owner, Instigator));
}

#pragma endregion