// Copyright Tirefly. All Rights Reserved.


#include "TireflyActorPoolLibrary.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"

#include "NiagaraComponent.h"
#include "TireflyActorPoolLogChannels.h"
#include "TireflyActorPoolWorldSubsystem.h"
#include "Particles/ParticleSystemComponent.h"



AActor* UTireflyActorPoolLibrary::SpawnActorFromPool(
	const UObject* WorldContext,
	TSubclassOf<AActor> ActorClass,
	FName ActorId,
	const FTransform& SpawnTransform,
	float Lifetime,
	ESpawnActorCollisionHandlingMethod CollisionHandling,
	AActor* Owner,
	APawn* Instigator)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		UE_LOG(LogTireflyActorPool, Error, TEXT("[%s] Invalid World"), *FString(__FUNCTION__));
		return nullptr;
	}

	UTireflyActorPoolWorldSubsystem* SubsystemAP = World->GetSubsystem<UTireflyActorPoolWorldSubsystem>();
	if (!SubsystemAP)
	{
		UE_LOG(LogTireflyActorPool, Error, TEXT("[%s] Invalid Subsystem"), *FString(__FUNCTION__));
		return nullptr;
	}
	
	return SubsystemAP->SpawnActorFromPool(
		ActorClass,
		ActorId,
		SpawnTransform,
		nullptr,
		Lifetime,
		CollisionHandling,
		Owner,
		Instigator);
}

AActor* UTireflyActorPoolLibrary::SpawnActorFromPoolWithParams(const UObject* WorldContext,
	TSubclassOf<AActor> ActorClass, FName ActorId, const FTransform& SpawnTransform, const FInstancedStruct& InitialData,
	float Lifetime, ESpawnActorCollisionHandlingMethod CollisionHandling, AActor* Owner, APawn* Instigator)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		UE_LOG(LogTireflyActorPool, Error, TEXT("[%s] Invalid World"), *FString(__FUNCTION__));
		return nullptr;
	}

	UTireflyActorPoolWorldSubsystem* SubsystemAP = World->GetSubsystem<UTireflyActorPoolWorldSubsystem>();
	if (!SubsystemAP)
	{
		UE_LOG(LogTireflyActorPool, Error, TEXT("[%s] Invalid Subsystem"), *FString(__FUNCTION__));
		return nullptr;
	}

	return SubsystemAP->SpawnActorFromPool(
		ActorClass,
		ActorId,
		SpawnTransform,
		&InitialData,
		Lifetime,
		CollisionHandling,
		Owner,
		Instigator);
}

void UTireflyActorPoolLibrary::RecycleActorToPool(const UObject* WorldContext, AActor* Actor)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		UE_LOG(LogTireflyActorPool, Error, TEXT("[%s] Invalid World"), *FString(__FUNCTION__));
		return;
	}

	UTireflyActorPoolWorldSubsystem* SubsystemAP = World->GetSubsystem<UTireflyActorPoolWorldSubsystem>();
	if (!SubsystemAP)
	{
		UE_LOG(LogTireflyActorPool, Error, TEXT("[%s] Invalid Subsystem"), *FString(__FUNCTION__));
		return;
	}

	SubsystemAP->RecycleActorToPool(Actor);
}

void UTireflyActorPoolLibrary::WarmUpActorPool(
	const UObject* WorldContext,
	TSubclassOf<AActor> ActorClass,
	FName ActorId,
	int32 Count)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		UE_LOG(LogTireflyActorPool, Error, TEXT("[%s] Invalid World"), *FString(__FUNCTION__));
		return;
	}

	UTireflyActorPoolWorldSubsystem* SubsystemAP = World->GetSubsystem<UTireflyActorPoolWorldSubsystem>();
	if (!SubsystemAP)
	{
		UE_LOG(LogTireflyActorPool, Error, TEXT("[%s] Invalid Subsystem"), *FString(__FUNCTION__));
		return;
	}

	SubsystemAP->WarmUpActorPool(ActorClass, ActorId, Count);
}

void UTireflyActorPoolLibrary::ProcessComponent(UActorComponent* Component, bool bActivate)
{
	if (!Component)
	{
		return;
	}

	if (UParticleSystemComponent* ParticleSystem = Cast<UParticleSystemComponent>(Component))
	{
		if (bActivate)
		{
			ParticleSystem->SetActive(true, true);
			ParticleSystem->ActivateSystem();
		}
		else
		{
			ParticleSystem->DeactivateSystem();
			Component->SetActive(false);
		}
		return;
	}

	if (UNiagaraComponent* Niagara = Cast<UNiagaraComponent>(Component))
	{
		if (bActivate)
		{
			Niagara->SetActive(true, true);
			Niagara->ActivateSystem();
		}
		else
		{
			Niagara->DeactivateImmediate();
			Component->SetActive(false);
		}
		return;
	}

	if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component))
	{
		if (bActivate)
		{
			Primitive->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
			Primitive->SetPhysicsLinearVelocity(FVector::ZeroVector);
			Primitive->SetComponentTickEnabled(true);
			Primitive->SetVisibility(true, true);
			Primitive->SetActive(true, true);
		}
		else
		{
			Primitive->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
			Primitive->SetPhysicsLinearVelocity(FVector::ZeroVector);
			Primitive->SetComponentTickEnabled(false);
			Primitive->SetSimulatePhysics(false);
			Primitive->SetVisibility(false, true);
			Component->SetActive(false);
		}
		return;
	}

	if (UMovementComponent* Movement = Cast<UMovementComponent>(Component))
	{
		if (bActivate)
		{
			Movement->SetUpdatedComponent(Movement->GetOwner()->GetRootComponent());
			Movement->SetActive(true, true);
			if (UProjectileMovementComponent* ProjectileMovement = Cast<UProjectileMovementComponent>(Movement))
			{
				ProjectileMovement->SetVelocityInLocalSpace(FVector::XAxisVector * ProjectileMovement->InitialSpeed);
			}
		}
		else
		{
			Movement->StopMovementImmediately();
			Movement->SetUpdatedComponent(nullptr);
		}
		return;
	}

	Component->SetActive(bActivate, true);
}

void UTireflyActorPoolLibrary::ProcessPawnController(APawn* Pawn, bool bActivate)
{
	if (!IsValid(Pawn))
	{
		return;
	}

	if (bActivate)
	{
		Pawn->SpawnDefaultController();
		if (IsValid(Pawn->GetController()))
		{
			if (const AAIController* AIController = Cast<AAIController>(Pawn->GetController()))
			{
				if (UBrainComponent* Brain = AIController->GetBrainComponent())
				{
					Brain->RestartLogic();
				}
			}
		}
	}
	else if (IsValid(Pawn->GetController()))
	{
		if (const AAIController* AIController = Cast<AAIController>(Pawn->GetController()))
		{
			if (UBrainComponent* Brain = AIController->GetBrainComponent())
			{
				Brain->Cleanup();
			}
		}
	}
}

void UTireflyActorPoolLibrary::GenericBeginPlay_Actor(const UObject* WorldContext, AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}

	Actor->SetActorTickEnabled(true);
	Actor->SetActorEnableCollision(true);
	Actor->SetActorHiddenInGame(false);

	TInlineComponentArray<UActorComponent*> Components;
	Actor->GetComponents(Components);
	for (UActorComponent* Component : Components)
	{
		ProcessComponent(Component, true);
	}
}

void UTireflyActorPoolLibrary::GenericEndPlay_Actor(const UObject* WorldContext, AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}

	Actor->SetActorTickEnabled(false);
	Actor->SetActorEnableCollision(false);
	Actor->SetActorHiddenInGame(true);

	TInlineComponentArray<UActorComponent*> Components;
	Actor->GetComponents(Components);
	for (UActorComponent* Component : Components)
	{
		ProcessComponent(Component, false);
	}
}

void UTireflyActorPoolLibrary::GenericWarmUp_Actor(const UObject* WorldContext, AActor* Actor)
{
	GenericEndPlay_Actor(WorldContext, Actor);
}

void UTireflyActorPoolLibrary::GenericBeginPlay_Pawn(const UObject* WorldContext, APawn* Pawn)
{
	if (!IsValid(Pawn))
	{
		return;
	}

	GenericBeginPlay_Actor(WorldContext, Pawn);
	ProcessPawnController(Pawn, true);
}

void UTireflyActorPoolLibrary::GenericEndPlay_Pawn(const UObject* WorldContext, APawn* Pawn)
{
	if (!IsValid(Pawn))
	{
		return;
	}

	GenericEndPlay_Actor(WorldContext, Pawn);
	ProcessPawnController(Pawn, false);
}

void UTireflyActorPoolLibrary::GenericWarmUp_Pawn(const UObject* WorldContext, APawn* Pawn)
{
	GenericEndPlay_Pawn(WorldContext, Pawn);
}

void UTireflyActorPoolLibrary::GenericBeginPlay_Character(const UObject* WorldContext, ACharacter* Character)
{
	GenericBeginPlay_Pawn(WorldContext, Character);
}

void UTireflyActorPoolLibrary::GenericEndPlay_Character(const UObject* WorldContext, ACharacter* Character)
{
	GenericEndPlay_Pawn(WorldContext, Character);
}

void UTireflyActorPoolLibrary::GenericWarmUp_Character(const UObject* WorldContext, ACharacter* Character)
{
	GenericWarmUp_Pawn(WorldContext, Character);
}
