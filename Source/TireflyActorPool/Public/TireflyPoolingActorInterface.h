// Copyright Tirefly. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "StructUtils/InstancedStruct.h"
#include "TireflyPoolingActorInterface.generated.h"



UINTERFACE(MinimalAPI, BlueprintType)
class UTireflyPoolingActorInterface : public UInterface
{
	GENERATED_BODY()
};


// Actor池生成的Actor需要实现的接口
class TIREFLYACTORPOOL_API ITireflyPoolingActorInterface
{
	GENERATED_BODY()
	
public:
	// Actor从对象池中生成后执行的 “对象池专属BeginPlay”
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Tirefly Actor Pool")
	void PoolingBeginPlay();
	virtual void PoolingBeginPlay_Implementation() {}

	// Actor从对象池中生成后执行的初始化，通过InstancedStruct进行属性初始化
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Tirefly Actor Pool")
	void PoolingInitialized(const FInstancedStruct& InitialData);
	virtual void PoolingInitialized_Implementation(const FInstancedStruct& InitialData) {}

	// Actor被放回对象池中后执行的操作，类似EndPlay
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Tirefly Actor Pool")
	void PoolingEndPlay();
	virtual void PoolingEndPlay_Implementation() {}

	// Actor在对象池中预热时执行的函数
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Tirefly Actor Pool")
	void PoolingWarmUp();
	virtual void PoolingWarmUp_Implementation() {}

	// 获取对象池Actor的Id，用来识别对象池Actor
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Tirefly Actor Pool")
	FName PoolingGetActorId() const;
	virtual FName PoolingGetActorId_Implementation() const { return NAME_None; }

	// 设置对象池Actor的Id，会在Actor从对象池中首次生成时默认调用
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Tirefly Actor Pool")
	void PoolingSetActorId(FName NewActorId);
	virtual void PoolingSetActorId_Implementation(FName NewActorId) {}
};
