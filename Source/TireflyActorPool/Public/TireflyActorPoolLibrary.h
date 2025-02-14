// Copyright Tirefly. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TireflyActorPoolLibrary.generated.h"


/** 用于Actor对象池的通用函数API */
/** Generic function API for actor pool */
UCLASS()
class TIREFLYACTORPOOL_API UTireflyActorPoolLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

#pragma region ActorPool_GenericOperation_Actor

	// 从ActorPool生成执行指定Actor类的实例，但不会自动运行其构造脚本及其ActorPool初始化。
	// Spawns an instance of the specified actor class from ActorPool, but does not automatically run its construction script and its ActorPool initialization.
	UFUNCTION(BlueprintCallable, Category = "Actor Pool", Meta = (
		WorldContext = "WorldContext",
		UnsafeDuringActorConstruction = "true",
		BlueprintInternalUseOnly = "true"))
	static AActor* GenericActorPool_BeginDeferredActorSpawn(
		const UObject* WorldContext,
		TSubclassOf<AActor> ActorClass,
		FName ActorID,
		const FTransform& SpawnTransform,
		ESpawnActorCollisionHandlingMethod CollisionHandling = ESpawnActorCollisionHandlingMethod::AlwaysSpawn,
		AActor* Owner = nullptr,
		APawn* Instigator = nullptr);

	// 完成从ActorPool生成一个Actor实例，并且会执行Actor的构造脚本和ActorPool初始化。
	// 'Finish' spawning an actor from ActorPool.  This will run the construction script and the ActorPool initialization.
	UFUNCTION(BlueprintCallable, Category = "Actor Pool", Meta = (
		WorldContext = "WorldContext",
		UnsafeDuringActorConstruction = "true",
		BlueprintInternalUseOnly = "true"))
	static AActor* GenericActorPool_FinishSpawningActor(
		const UObject* WorldContext,
		AActor* Actor,
		const FTransform& SpawnTransform,
		float Lifetime);

	// Actor通用的从对象池中取出后进行初始化的操作。
	// Generic operation for an Actor to initialize after being taken out from the object pool.
	UFUNCTION(BlueprintCallable, Category = "Actor Pool", Meta = (WorldContext = "WorldContext", DefaultToSelf = "Actor"))
	static void GenericBeginPlay_Actor(const UObject* WorldContext, AActor* Actor);

	// Actor通用的回到对象池后进入冻结状态的操作。
	// Generic operation for an Actor to enter a frozen state after returning to the object pool.
	UFUNCTION(BlueprintCallable, Category = "Actor Pool", Meta = (WorldContext = "WorldContext", DefaultToSelf = "Actor"))
	static void GenericEndPlay_Actor(const UObject* WorldContext, AActor* Actor);

	// Actor通用的在对象池中生成后进入待命状态的操作。
	// Generic operation for an Actor to enter a standby state after being generated in the object pool.
	UFUNCTION(BlueprintCallable, Category = "Actor Pool", Meta = (WorldContext = "WorldContext", DefaultToSelf = "Actor"))
	static void GenericWarmUp_Actor(const UObject* WorldContext, AActor* Actor);

#pragma endregion


#pragma region ActorPool_GenericOperation_Pawn

	// Pawn通用的从对象池中取出后进行初始化的操作。
	// Generic operation for a Pawn to initialize after being taken out from the object pool.
	UFUNCTION(BlueprintCallable, Category = "Actor Pool", Meta = (WorldContext = "WorldContext", DefaultToSelf = "Pawn"))
	static void GenericBeginPlay_Pawn(const UObject* WorldContext, APawn* Pawn);

	// Pawn通用的回到对象池后进入冻结状态的操作。
	// Generic operation for a Pawn to enter a frozen state after returning to the object pool.
	UFUNCTION(BlueprintCallable, Category = "Actor Pool", Meta = (WorldContext = "WorldContext", DefaultToSelf = "Pawn"))
	static void GenericEndPlay_Pawn(const UObject* WorldContext, APawn* Pawn);

	// Pawn通用的在对象池中生成后进入待命状态的操作。
	// Generic operation for a Pawn to enter a standby state after being generated in the object pool.
	UFUNCTION(BlueprintCallable, Category = "Actor Pool", Meta = (WorldContext = "WorldContext", DefaultToSelf = "Pawn"))
	static void GenericWarmUp_Pawn(const UObject* WorldContext, APawn* Pawn);

#pragma endregion


#pragma region ActorPool_GenericOperation_Character

	// Character通用的从对象池中取出后进行初始化的操作。
	// Generic operation for a Character to initialize after being taken out from the object pool.
	UFUNCTION(BlueprintCallable, Category = "Actor Pool", Meta = (WorldContext = "WorldContext", DefaultToSelf = "Character"))
	static void GenericBeginPlay_Character(const UObject* WorldContext, ACharacter* Character);

	// Character通用的回到对象池后进入冻结状态的操作。
	// Generic operation for a Character to enter a frozen state after returning to the object pool.
	UFUNCTION(BlueprintCallable, Category = "Actor Pool", Meta = (WorldContext = "WorldContext", DefaultToSelf = "Character"))
	static void GenericEndPlay_Character(const UObject* WorldContext, ACharacter* Character);

	// Character通用的在对象池中生成后进入待命状态的操作。
	// Generic operation for a Character to enter a standby state after being generated in the object pool.
	UFUNCTION(BlueprintCallable, Category = "Actor Pool", Meta = (WorldContext = "WorldContext", DefaultToSelf = "Character"))
	static void GenericWarmUp_Character(const UObject* WorldContext, ACharacter* Character);

#pragma endregion
};