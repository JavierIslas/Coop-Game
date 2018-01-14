// Fill out your copyright notice in the Description page of Project Settings.

#include "SHealtComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
USHealtComponent::USHealtComponent()
{
	DefaultHealth = 100;

	SetIsReplicated(true);
}


// Called when the game starts
void USHealtComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* Owner = GetOwner();
		if (Owner)
		{
			Owner->OnTakeAnyDamage.AddDynamic(this, &USHealtComponent::HandleTakeAnyDamage);
		}
	}

	Health = DefaultHealth;
	
}

void USHealtComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.0f)
	{
		return;
	}
	Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);

	UE_LOG(LogTemp, Log, TEXT("Health Changed : %s"), *FString::SanitizeFloat(Health));

	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);
}

void USHealtComponent::OnRep_Health(float OldHealth)
{
	float Damage = Health - OldHealth;
	OnHealthChanged.Broadcast(this, Health, FMath::Abs(Damage), nullptr, nullptr, nullptr);
}

void USHealtComponent::Heal(float HealAmount)
{
	if (HealAmount <= 0.0f || Health <= 0.0f)
	{
		return;
	}

	Health = FMath::Clamp(Health + HealAmount, 0.0f, DefaultHealth);

	UE_LOG(LogTemp, Log, TEXT("Health Changed : %s (+%s)"), *FString::SanitizeFloat(Health), *FString::SanitizeFloat(HealAmount));

	OnHealthChanged.Broadcast(this, Health, HealAmount, nullptr, nullptr, nullptr);

}


void USHealtComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USHealtComponent, Health);
}
