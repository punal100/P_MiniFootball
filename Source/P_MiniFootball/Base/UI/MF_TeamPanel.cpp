/*
 * @Author: Punal Manalan
 * @Description: MF_TeamPanel - Reusable team info panel widget implementation
 * @Date: 10/12/2025
 */

#include "MF_TeamPanel.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/Border.h"

void UMF_TeamPanel::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind join button click
    if (JoinButton)
    {
        JoinButton->OnClicked.AddDynamic(this, &UMF_TeamPanel::HandleJoinButtonClicked);
    }
}

void UMF_TeamPanel::NativeDestruct()
{
    // Unbind delegates
    if (JoinButton)
    {
        JoinButton->OnClicked.RemoveDynamic(this, &UMF_TeamPanel::HandleJoinButtonClicked);
    }

    Super::NativeDestruct();
}

void UMF_TeamPanel::SetTeamID(EMF_TeamID InTeamID)
{
    TeamID = InTeamID;
    UpdateTeamVisuals();
}

void UMF_TeamPanel::UpdateTeamVisuals()
{
    // Set team name
    if (TeamNameText)
    {
        FString TeamName;
        switch (TeamID)
        {
        case EMF_TeamID::TeamA:
            TeamName = TEXT("TEAM A");
            break;
        case EMF_TeamID::TeamB:
            TeamName = TEXT("TEAM B");
            break;
        default:
            TeamName = TEXT("NO TEAM");
            break;
        }
        TeamNameText->SetText(FText::FromString(TeamName));
    }

    // Set team color on border
    if (PanelBorder)
    {
        FLinearColor TeamColor;
        switch (TeamID)
        {
        case EMF_TeamID::TeamA:
            TeamColor = TeamAColor;
            break;
        case EMF_TeamID::TeamB:
            TeamColor = TeamBColor;
            break;
        default:
            TeamColor = FLinearColor::Gray;
            break;
        }
        PanelBorder->SetBrushColor(TeamColor);
    }

    // Set default button text
    if (JoinButtonText)
    {
        FString ButtonText;
        switch (TeamID)
        {
        case EMF_TeamID::TeamA:
            ButtonText = TEXT("JOIN TEAM A");
            break;
        case EMF_TeamID::TeamB:
            ButtonText = TEXT("JOIN TEAM B");
            break;
        default:
            ButtonText = TEXT("JOIN");
            break;
        }
        JoinButtonText->SetText(FText::FromString(ButtonText));
    }
}

void UMF_TeamPanel::SetPlayerData(const TArray<FString> &PlayerNames)
{
    // Use default max players of 3
    SetPlayerDataWithMax(PlayerNames, 3);
}

void UMF_TeamPanel::SetPlayerDataWithMax(const TArray<FString> &PlayerNames, int32 MaxPlayers)
{
    // Update player count text
    if (PlayerCountText)
    {
        FString CountStr = FString::Printf(TEXT("Players: %d/%d"), PlayerNames.Num(), MaxPlayers);
        PlayerCountText->SetText(FText::FromString(CountStr));
    }

    // Clear existing player list
    if (PlayerListBox)
    {
        PlayerListBox->ClearChildren();

        // Add player names
        for (const FString &Name : PlayerNames)
        {
            UTextBlock *NameText = CreatePlayerNameText(Name);
            if (NameText)
            {
                PlayerListBox->AddChild(NameText);
            }
        }

        // Add empty slots
        int32 EmptySlots = MaxPlayers - PlayerNames.Num();
        for (int32 i = 0; i < EmptySlots; ++i)
        {
            UTextBlock *EmptyText = CreatePlayerNameText(TEXT("[Empty Slot]"));
            if (EmptyText)
            {
                EmptyText->SetColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f, 0.7f)));
                PlayerListBox->AddChild(EmptyText);
            }
        }
    }
}

void UMF_TeamPanel::SetJoinButtonEnabled(bool bEnabled)
{
    if (JoinButton)
    {
        JoinButton->SetIsEnabled(bEnabled);
    }
}

void UMF_TeamPanel::SetJoinButtonText(const FString &NewText)
{
    if (JoinButtonText)
    {
        JoinButtonText->SetText(FText::FromString(NewText));
    }
}

void UMF_TeamPanel::HandleJoinButtonClicked()
{
    OnJoinClicked.Broadcast(TeamID);
}

UTextBlock *UMF_TeamPanel::CreatePlayerNameText(const FString &PlayerName)
{
    UTextBlock *TextBlock = NewObject<UTextBlock>(this);
    if (TextBlock)
    {
        TextBlock->SetText(FText::FromString(FString::Printf(TEXT("• %s"), *PlayerName)));
        TextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));

        // Set font size
        FSlateFontInfo FontInfo = TextBlock->GetFont();
        FontInfo.Size = 14;
        TextBlock->SetFont(FontInfo);
    }
    return TextBlock;
}
