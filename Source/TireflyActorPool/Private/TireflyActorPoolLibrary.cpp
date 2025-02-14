// Copyright Tirefly. All Rights Reserved.


#include "TireflyActorPoolLibrary.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"

#include "NiagaraComponent.h"
#include "TireflyActorPoolWorldSubsystem.h"
#include "Particles/ParticleSystemComponent.h"


AActor* UTireflyActorPoolLibrary::GenericActorPool_BeginDeferredActorSpawn(
	const UObject* WorldContext,
	TSubclassOf<AActor> ActorClass,
	FName ActorID,
	const FTransform& SpawnTransform,
	ESpawnActorCollisionHandlingMethod CollisionHandling,
	AActor* Owner,
	APawn* Instigator)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] - WorldContext is invalid."), *FString(__FUNCTION__));
		return nullptr;
	}

	UTireflyActorPoolWorldSubsystem* ActorPoolSubsystem = World->GetSubsystem<UTireflyActorPoolWorldSubsystem>();
	if (!ActorPoolSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] - ActorPoolSubsystem is invalid."), *FString(__FUNCTION__));
		return nullptr;
	}

	return ActorPoolSubsystem->ActorPool_BeginDeferredActorSpawn_Internal(
		WorldContext,
		ActorClass,
		ActorID,
		SpawnTransform,
		CollisionHandling,
		Owner,
		Instigator);
}

AActor* UTireflyActorPoolLibrary::GenericActorPool_FinishSpawningActor(
	const UObject* WorldContext,
	AActor* Actor,
	const FTransform& SpawnTransform,
	float Lifetime)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] - WorldContext is invalid."), *FString(__FUNCTION__));
		return nullptr;
	}

	UTireflyActorPoolWorldSubsystem* ActorPoolSubsystem = World->GetSubsystem<UTireflyActorPoolWorldSubsystem>();
	if (!ActorPoolSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] - ActorPoolSubsystem is invalid."), *FString(__FUNCTION__));
		return nullptr;
	}

	return ActorPoolSubsystem->ActorPool_FinishSpawningActor_Internal(
		WorldContext,
		Actor,
		SpawnTransform,
		Lifetime);
}

void UTireflyActorPoolLibrary::GenericBeginPlay_Actor(const UObject* WorldContext, AActor* Actor)
{
	Actor->SetActorTickEnabled(true);
	Actor->SetActorEnableCollision(true);
	Actor->SetActorHiddenInGame(false);

	TInlineComponentArray<UActorComponent*>Components;
	Actor->GetComponents(Components);
	for (UActorComponent* Component : Components)
	{
		if (UParticleSystemComponent* ParticleSystem = Cast<UParticleSystemComponent>(Component))
		{
			ParticleSystem->SetActive(true, true);
			ParticleSystem->ActivateSystem();

			continue;
		}

		if (UNiagaraComponent* Niagara = Cast<UNiagaraComponent>(Component))
		{
			Niagara->SetActive(true, true);
			Niagara->ActivateSystem();
			
			continue;
		}

		if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component))
		{
			Primitive->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
			Primitive->SetPhysicsLinearVelocity(FVector::ZeroVector);
			Primitive->SetComponentTickEnabled(true);
			Primitive->SetVisibility(true, true);

			Primitive->SetActive(true, true);

			continue;
		}

		if (UMovementComponent* Movement = Cast<UMovementComponent>(Component))
		{
			Movement->SetUpdatedComponent(Actor->GetRootComponent());
			Movement->SetActive(true, true);
			if (UProjectileMovementComponent* ProjectileMovement = Cast<UProjectileMovementComponent>(Movement))
			{
				ProjectileMovement->SetVelocityInLocalSpace(FVector::XAxisVector * ProjectileMovement->InitialSpeed);
			}
		}

		Component->SetActive(true, true);
	}
}

void UTireflyActorPoolLibrary::GenericEndPlay_Actor(const UObject* WorldContext, AActor* Actor)
{
	Actor->SetActorTickEnabled(false);
	Actor->SetActorEnableCollision(false);
	Actor->SetActorHiddenInGame(true);

	TInlineComponentArray<UActorComponent*>Components;
	Actor->GetComponents(Components);
	for (UActorComponent* Component : Components)
	{
		if (UParticleSystemComponent* ParticleSystem = Cast<UParticleSystemComponent>(Component))
		{
			ParticleSystem->DeactivateSystem();
			Component->SetActive(false);

			continue;
		}

		if (UNiagaraComponent* Niagara = Cast<UNiagaraComponent>(Component))
		{
			Niagara->DeactivateImmediate();
			Component->SetActive(false);

			continue;
		}

		if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component))
		{
			Primitive->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
			Primitive->SetPhysicsLinearVelocity(FVector::ZeroVector);
			Primitive->SetComponentTickEnabled(false);
			Primitive->SetSimulatePhysics(false);
			Primitive->SetVisibility(false, true);
			Component->SetActive(false);

			continue;
		}

		if (UMovementComponent* Movement = Cast<UMovementComponent>(Component))
		{
			Movement->StopMovementImmediately();
			Movement->SetUpdatedComponent(nullptr);
		}

		Component->SetActive(false);
	}
}

void UTireflyActorPoolLibrary::GenericWarmUp_Actor(const UObject* WorldContext, AActor* Actor)
{
	GenericEndPlay_Actor(WorldContext, Actor);
}

void UTireflyActorPoolLibrary::GenericBeginPlay_Pawn(const UObject* WorldContext, APawn* Pawn)
{
	GenericBeginPlay_Actor(WorldContext, Pawn);

	Pawn->SpawnDefaultController();
	if (IsValid(Pawn->GetController()))
	{
		if (const AAIController* AIController = Cast<AAIController>(Pawn->GetController()))
		{
			if (UBrainComponent* Brain = AIController->GetBrainComponent())
			{
				Brain->StartLogic();
			}
		}
	}
}

void UTireflyActorPoolLibrary::GenericEndPlay_Pawn(const UObject* WorldContext, APawn* Pawn)
{
	GenericEndPlay_Actor(WorldContext, Pawn);

	if (IsValid(Pawn->GetController()))
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

void UTireflyActorPoolLibrary::GenericWarmUp_Pawn(const UObject* WorldContext, APawn* Pawn)
{
	GenericWarmUp_Actor(WorldContext, Pawn);
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
