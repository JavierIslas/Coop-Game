// Fill out your copyright notice in the Description page of Project Settings.

#include "SHealtComponent.h"
#include "SGameMode.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
USHealtComponent::USHealtComponent()
{
	DefaultHealth = 100;

	bIsDead = false;

	SetIsReplicated(true);

	Team = 255;
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
	if (Damage <= 0.0f || bIsDead)
	{
		return;
	}

	if ((DamageCauser != DamagedActor) && (IsFriendly(GetOwner(), DamageCauser)))
	{
		return;
	}
	Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);

	UE_LOG(LogTemp, Log, TEXT("Health Changed : %s"), *FString::SanitizeFloat(Health));

	bIsDead = Health <= 0.0f;

	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

	if (bIsDead)
	{
		ASGameMode* GM = Cast<ASGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			GM->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
		}
	}
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

float USHealtComponent::GetHealth() const
{
	return Health;
}

bool USHealtComponent::IsFriendly(AActor* A, AActor* B)
{
	if (A == nullptr || B == nullptr)
	{
		return true;
	}
	USHealtComponent* HealthComponentA = Cast<USHealtComponent>(A->GetComponentByClass(USHealtComponent::StaticClass()));
	USHealtComponent* HealthComponentB = Cast<USHealtComponent>(B->GetComponentByClass(USHealtComponent::StaticClass()));
	if (HealthComponentA == nullptr || HealthComponentB == nullptr)
	{
		return true;
	}
	return HealthComponentA->Team == HealthComponentB->Team;
}


void USHealtComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USHealtComponent, Health);
}
