/*
 * @Author: Punal Manalan
 * @Description: MF_TransitionOverlay - Loading/transition screen implementation
 * @Date: 10/12/2025
 */

#include "MF_TransitionOverlay.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Throbber.h"
#include "Animation/WidgetAnimation.h"

void UMF_TransitionOverlay::NativeConstruct()
{
    Super::NativeConstruct();

    // Set default message
    SetStatusText(DefaultStatusMessage);
}

void UMF_TransitionOverlay::SetStatusText(const FString &InStatusText)
{
    if (StatusText)
    {
        StatusText->SetText(FText::FromString(InStatusText));
    }
}

void UMF_TransitionOverlay::ShowOverlay()
{
    SetVisibility(ESlateVisibility::Visible);
    PlayFadeAnimation(true);
}

void UMF_TransitionOverlay::HideOverlay()
{
    PlayFadeAnimation(false);
    // Note: In a full implementation, we'd use an animation callback to set visibility to Hidden
    // For now, we'll set it immediately after starting the fade
    SetVisibility(ESlateVisibility::Hidden);
}

void UMF_TransitionOverlay::ShowWithMessage(const FString &Message)
{
    SetStatusText(Message);
    ShowOverlay();
}

void UMF_TransitionOverlay::PlayFadeAnimation(bool bFadeIn)
{
    // Simple opacity change - in a full implementation, you'd use UWidgetAnimation
    float TargetOpacity = bFadeIn ? 1.0f : 0.0f;
    SetRenderOpacity(TargetOpacity);

    // If using widget animations in UMG designer, you would call:
    // UWidgetAnimation* FadeAnim = bFadeIn ? FadeInAnimation : FadeOutAnimation;
    // if (FadeAnim)
    // {
    //     PlayAnimation(FadeAnim);
    // }
}
