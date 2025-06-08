// Copyright Tirefly. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StructUtils/InstancedStruct.h"
#include "TireflyActorPoolLibrary.generated.h"



// 用于Actor对象池的函数库
UCLASS()
class TIREFLYACTORPOOL_API UTireflyActorPoolLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

#pragma region ActorPool_GenericOperation

public:
	/**
	 * 从Actor对象池中生成一个Actor实例
	 * 
	 * @param WorldContext 世界上下文对象，默认为当前世界对象，只有通过世界才能生成Actor
	 * @param ActorClass 要生成的Actor类型
	 * @param ActorId 要生成的Actor的Id标识
	 * @param SpawnTransform 要生成的Actor的初始化世界坐标系下的Transform
	 * @param Lifetime 生成的Actor的存活时间，默认为-1，表示一直存活
	 * @param CollisionHandling 生成Actor时的初始碰撞处理方式，默认为AlwaysSpawn
	 * @param Owner 要生成的Actor的Owner，默认为空
	 * @param Instigator 要生成的Actor的Instigator，默认为空
	 * @return 从对象池中生成的Actor实例
	 */
	UFUNCTION(BlueprintCallable, Category = "Tirefly Actor Pool", Meta = (WorldContext = "WorldContext", DeterminesOutputType = "ActorClass"))
	static AActor* SpawnActorFromPool(
		const UObject* WorldContext,
		TSubclassOf<AActor> ActorClass,
		FName ActorId,
		const FTransform& SpawnTransform,
		float Lifetime = -1.f,
		ESpawnActorCollisionHandlingMethod CollisionHandling = ESpawnActorCollisionHandlingMethod::AlwaysSpawn,
		AActor* Owner = nullptr,
		APawn* Instigator = nullptr);

	/**
	 * 从Actor对象池中生成一个Actor实例
	 * 
	 * @param WorldContext 世界上下文对象，默认为当前世界对象，只有通过世界才能生成Actor
	 * @param ActorClass 要生成的Actor类型
	 * @param ActorId 要生成的Actor的Id标识
	 * @param SpawnTransform 要生成的Actor的初始化世界坐标系下的Transform
	 * @param InitialData Actor实例的初始化数据
	 * @param Lifetime 生成的Actor的存活时间，默认为-1，表示一直存活
	 * @param CollisionHandling 生成Actor时的初始碰撞处理方式，默认为AlwaysSpawn
	 * @param Owner 要生成的Actor的Owner，默认为空
	 * @param Instigator 要生成的Actor的Instigator，默认为空
	 * @return 从对象池中生成的Actor实例
	 */
	UFUNCTION(BlueprintCallable, Category = "Tirefly Actor Pool", Meta = (WorldContext = "WorldContext", DeterminesOutputType = "ActorClass"))
	static AActor* SpawnActorFromPoolWithParams(
		const UObject* WorldContext,
		TSubclassOf<AActor> ActorClass,
		FName ActorId,
		const FTransform& SpawnTransform,
		const FInstancedStruct& InitialData,
		float Lifetime = -1.f,
		ESpawnActorCollisionHandlingMethod CollisionHandling = ESpawnActorCollisionHandlingMethod::AlwaysSpawn,
		AActor* Owner = nullptr,
		APawn* Instigator = nullptr);

	// 回收Actor到对象池中
	UFUNCTION(BlueprintCallable, Category = "Tirefly Actor Pool", Meta = (WorldContext = "WorldContext"))
	static void RecycleActorToPool(const UObject* WorldContext, AActor* Actor);

	/**
	 * 预热特定类型（或特定Id）的对象池，生成指定数量的Actor并使其在池中待命
	 *
	 * @param WorldContext 世界上下文对象，默认为当前世界对象，只有通过世界才能生成Actor
	 * @param ActorClass 对象池的目标类型
	 * @param ActorId 对象池的目标Id
	 * @param Count 预热的Actor数量
	 */
	UFUNCTION(BlueprintCallable, Category = "Tirefly Actor Pool", Meta = (WorldContext = "WorldContext"))
	static void WarmUpActorPool(
		const UObject* WorldContext,
		TSubclassOf<AActor> ActorClass,
		FName ActorId,
		int32 Count = 16);

#pragma endregion
	

#pragma region ActorPool_GenericOperation_Actor

public:
	// Actor通用的从对象池中取出后进行初始化的操作。
	UFUNCTION(BlueprintCallable, Category = "Tirefly Actor Pool", Meta = (WorldContext = "WorldContext", DefaultToSelf = "Actor"))
	static void GenericBeginPlay_Actor(const UObject* WorldContext, AActor* Actor);

	// Actor通用的回到对象池后进入冻结状态的操作。
	UFUNCTION(BlueprintCallable, Category = "Tirefly Actor Pool", Meta = (WorldContext = "WorldContext", DefaultToSelf = "Actor"))
	static void GenericEndPlay_Actor(const UObject* WorldContext, AActor* Actor);

	// Actor通用的在对象池中生成后进入待命状态的操作。
	UFUNCTION(BlueprintCallable, Category = "Tirefly Actor Pool", Meta = (WorldContext = "WorldContext", DefaultToSelf = "Actor"))
	static void GenericWarmUp_Actor(const UObject* WorldContext, AActor* Actor);

#pragma endregion


#pragma region ActorPool_GenericOperation_Pawn

public:
	// Pawn通用的从对象池中取出后进行初始化的操作。
	UFUNCTION(BlueprintCallable, Category = "Tirefly Actor Pool", Meta = (WorldContext = "WorldContext", DefaultToSelf = "Pawn"))
	static void GenericBeginPlay_Pawn(const UObject* WorldContext, APawn* Pawn);

	// Pawn通用的回到对象池后进入冻结状态的操作。
	UFUNCTION(BlueprintCallable, Category = "Tirefly Actor Pool", Meta = (WorldContext = "WorldContext", DefaultToSelf = "Pawn"))
	static void GenericEndPlay_Pawn(const UObject* WorldContext, APawn* Pawn);

	// Pawn通用的在对象池中生成后进入待命状态的操作。
	UFUNCTION(BlueprintCallable, Category = "Tirefly Actor Pool", Meta = (WorldContext = "WorldContext", DefaultToSelf = "Pawn"))
	static void GenericWarmUp_Pawn(const UObject* WorldContext, APawn* Pawn);

#pragma endregion


#pragma region ActorPool_GenericOperation_Character

public:
	// Character通用的从对象池中取出后进行初始化的操作。
	UFUNCTION(BlueprintCallable, Category = "Tirefly Actor Pool", Meta = (WorldContext = "WorldContext", DefaultToSelf = "Character"))
	static void GenericBeginPlay_Character(const UObject* WorldContext, ACharacter* Character);

	// Character通用的回到对象池后进入冻结状态的操作。
	UFUNCTION(BlueprintCallable, Category = "Tirefly Actor Pool", Meta = (WorldContext = "WorldContext", DefaultToSelf = "Character"))
	static void GenericEndPlay_Character(const UObject* WorldContext, ACharacter* Character);

	// Character通用的在对象池中生成后进入待命状态的操作。
	UFUNCTION(BlueprintCallable, Category = "Tirefly Actor Pool", Meta = (WorldContext = "WorldContext", DefaultToSelf = "Character"))
	static void GenericWarmUp_Character(const UObject* WorldContext, ACharacter* Character);

#pragma endregion

private:
	// 处理单个组件的激活/停用
	static void ProcessComponent(UActorComponent* Component, bool bActivate);
	
	// 处理Pawn的Controller相关操作
	static void ProcessPawnController(APawn* Pawn, bool bActivate);
};