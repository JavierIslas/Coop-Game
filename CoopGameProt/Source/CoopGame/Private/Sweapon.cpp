// Fill out your copyright notice in the Description page of Project Settings.

#include "Sweapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SkeletalMeshComponent.h"


static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(TEXT("COOP.DebugWeapons"), DebugWeaponDrawing, TEXT("Draw DebugLines for Weapons"), ECVF_Cheat);

// Sets default values
ASweapon::ASweapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	SocketName = "MuzzleSocket";
	TargetName = "BeamEnd";
}


void ASweapon::Fire()
{
	AActor* Owner = GetOwner();
	if (Owner)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		Owner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
		FVector ShotDirection = EyeRotation.Vector();
		FVector TraceEnd = EyeLocation + (EyeRotation.Vector() * 10000);
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Owner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;

		FVector TracerEndPoint = TraceEnd;

		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECC_Visibility, QueryParams))
		{
			AActor* HitActor = Hit.GetActor();
			UGameplayStatics::ApplyPointDamage(HitActor, 20.0f, ShotDirection, Hit, Owner->GetInstigatorController(), this, DamageType);

			if (ImpactEffect)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
			}
			TracerEndPoint = Hit.ImpactPoint;
		}
		if (DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Red, false, 1.0f, 0, 1.0f);
		}

		PlayFireEffect(TracerEndPoint);
	}
}

void ASweapon::PlayFireEffect(FVector TraceEnd)
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, SocketName);
	}


	if (SmokeEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(SocketName);
		UParticleSystemComponent* Tracer = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SmokeEffect, MuzzleLocation);
		if (Tracer)
		{
			Tracer->SetVectorParameter(TargetName, TraceEnd);
		}
	}

	APawn* Owner = Cast<APawn>(GetOwner());
	if (Owner)
	{
		APlayerController* PC = Cast<APlayerController>(Owner->GetController());
		if (PC)
		{
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
}



