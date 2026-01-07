// Copyright Punal Manalan. All Rights Reserved.

#include "AI/MF_EAISActionExecutorComponent.h"
#include "Player/MF_PlayerCharacter.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

UMF_EAISActionExecutorComponent::UMF_EAISActionExecutorComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UMF_EAISActionExecutorComponent::BeginPlay()
{
    Super::BeginPlay();
    OwnerCharacter = Cast<AMF_PlayerCharacter>(GetOwner());
}

FEAIS_ActionResult UMF_EAISActionExecutorComponent::EAIS_ExecuteAction_Implementation(const FName ActionId, const FString& ParamsJson)
{
    if (!OwnerCharacter) 
    {
        FEAIS_ActionResult Result;
        Result.bSuccess = false;
        Result.Message = TEXT("No owner character");
        return Result;
    }

    // [ANTI-RUBBERBAND] Critical Guard: Do NOT execute AI actions if controlled by a Human Player
    // This prevents the AI system from fighting with client-side prediction inputs.
    if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
    {
        FEAIS_ActionResult Result;
        Result.bSuccess = false;
        Result.Message = TEXT("Action blocked: Character is controlled by Human Player");
        return Result;
    }

    if (ActionId == "MF.Shoot") return HandleShoot(ParamsJson);
    if (ActionId == "MF.Pass") return HandlePass(ParamsJson);
    if (ActionId == "MF.Tackle") return HandleTackle(ParamsJson);
    if (ActionId == "MF.Sprint") return HandleSprint(ParamsJson);
    if (ActionId == "MF.Face") return HandleFace(ParamsJson);
    if (ActionId == "MF.Mark") return HandleMark(ParamsJson);

    FEAIS_ActionResult Result;
    Result.bSuccess = false;
    Result.Message = FString::Printf(TEXT("Unknown action: %s"), *ActionId.ToString());
    return Result;
}

FEAIS_ActionResult UMF_EAISActionExecutorComponent::HandleShoot(const FString& ParamsJson)
{
    FEAIS_ActionResult Result;
    
    // Parse direction and power (optional)
    FVector Direction = OwnerCharacter->GetActorForwardVector();
    float Power = 1.0f; // Multiplier of shoot speed

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ParamsJson);
    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        if (JsonObject->HasField("power")) Power = JsonObject->GetNumberField("power");
        // Target resolution is usually handled by MoveTo/AimAt, but we can override here
    }

    // Call server-side execution directly
    // Note: AMF_PlayerCharacter::ExecuteShoot takes world direction and raw power
    OwnerCharacter->Server_RequestShoot(Direction, Power * 2000.0f); // Proxy call to handle state/physics

    Result.bSuccess = true;
    return Result;
}

FEAIS_ActionResult UMF_EAISActionExecutorComponent::HandlePass(const FString& ParamsJson)
{
    FEAIS_ActionResult Result;
    FVector Direction = OwnerCharacter->GetActorForwardVector();
    float Power = 0.5f;

    OwnerCharacter->Server_RequestPass(Direction, Power * 1500.0f);

    Result.bSuccess = true;
    return Result;
}

FEAIS_ActionResult UMF_EAISActionExecutorComponent::HandleTackle(const FString& ParamsJson)
{
    FEAIS_ActionResult Result;
    OwnerCharacter->Server_RequestTackle();
    Result.bSuccess = true;
    return Result;
}

FEAIS_ActionResult UMF_EAISActionExecutorComponent::HandleSprint(const FString& ParamsJson)
{
    FEAIS_ActionResult Result;
    
    bool bActive = true; // Default to sprint on
    
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ParamsJson);
    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        if (JsonObject->HasField("active"))
        {
            bActive = JsonObject->GetBoolField("active");
        }
    }

    OwnerCharacter->SetSprinting(bActive);
    Result.bSuccess = true;
    Result.Message = bActive ? TEXT("Sprint ON") : TEXT("Sprint OFF");
    return Result;
}

FEAIS_ActionResult UMF_EAISActionExecutorComponent::HandleFace(const FString& ParamsJson)
{
    FEAIS_ActionResult Result;
    
    FString TargetName = TEXT("Ball"); // Default target
    
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ParamsJson);
    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        if (JsonObject->HasField("target"))
        {
            TargetName = JsonObject->GetStringField("target");
        }
    }

    // Resolve target location via TargetProvider interface
    FVector TargetLocation;
    if (IEAIS_TargetProvider::Execute_EAIS_GetTargetLocation(OwnerCharacter, FName(*TargetName), TargetLocation))
    {
        const FVector Direction = (TargetLocation - OwnerCharacter->GetActorLocation()).GetSafeNormal2D();
        if (!Direction.IsNearlyZero())
        {
            const FRotator NewRotation = Direction.Rotation();
            OwnerCharacter->SetActorRotation(FRotator(0.0f, NewRotation.Yaw, 0.0f));
            Result.bSuccess = true;
            Result.Message = FString::Printf(TEXT("Facing %s"), *TargetName);
        }
        else
        {
            Result.bSuccess = false;
            Result.Message = TEXT("Already at target location");
        }
    }
    else
    {
        Result.bSuccess = false;
        Result.Message = FString::Printf(TEXT("Target not found: %s"), *TargetName);
    }
    
    return Result;
}

FEAIS_ActionResult UMF_EAISActionExecutorComponent::HandleMark(const FString& ParamsJson)
{
    FEAIS_ActionResult Result;
    
    // Mark means follow the nearest opponent tightly
    // We'll get the nearest opponent's location and move towards them
    FVector TargetLocation;
    if (IEAIS_TargetProvider::Execute_EAIS_GetTargetLocation(OwnerCharacter, FName("NearestOpponent"), TargetLocation))
    {
        // Calculate offset position (don't stand on top of them, stay 100 units away)
        const FVector ToOpponent = TargetLocation - OwnerCharacter->GetActorLocation();
        const float Distance = ToOpponent.Size();
        
        if (Distance > 100.0f)
        {
            const FVector MarkPosition = TargetLocation - ToOpponent.GetSafeNormal() * 100.0f;
            
            // Use AI movement component to move there
            if (AController* Controller = OwnerCharacter->GetController())
            {
                OwnerCharacter->AddMovementInput(ToOpponent.GetSafeNormal(), 1.0f);
            }
            
            Result.bSuccess = true;
            Result.Message = TEXT("Marking opponent");
        }
        else
        {
            Result.bSuccess = true;
            Result.Message = TEXT("Already marking");
        }
    }
    else
    {
        Result.bSuccess = false;
        Result.Message = TEXT("No opponent to mark");
    }
    
    return Result;
}
