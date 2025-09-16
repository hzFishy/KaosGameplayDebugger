// Copyright (C) 2025, Daniel Moss
// 
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include "KaosWorldDebugger_GameplayEffect.h"
#if WITH_KAOS_GAMEPLAYDEBUGGER
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffectExecutionCalculation.h"
#include "GameplayModMagnitudeCalculation.h"
#include "Algo/Compare.h"
#include "EngineUtils.h"
#include "KaosSlateIMHelpers.h"

void FKaosWorldDebugger_GameplayEffects::DrawDetails(const FKaosDebuggerContext& Context)
{
	AActor* ContextActor = Cast<AActor>(Context.ContextObject.Get());

	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ContextActor))
	{
		TArray<FKaosGameplayEffectDebug> Effects = CollectEffectsData(ASC);
		if (ContextActor != LastSelectedActor)
		{
			SelectedEffectReplicationID.Reset();
		}
		LastSelectedActor = ContextActor;
		
		if (Effects.IsEmpty())
		{
			SlateIM::Text(TEXT("No Gameplay Effects found."));
			return;
		}
		SlateIM::BeginHorizontalStack();
		DrawWorldDebugger_Effects(Effects);
		SlateIM::Fill();
		SlateIM::HAlign(HAlign_Fill);
		SlateIM::VAlign(VAlign_Fill);
		DrawWorldDebugger_EffectDetails(Effects);
		SlateIM::EndHorizontalStack();
	}
	else
	{
		if (ContextActor)
		{
			KaosSlateIM::WarningText(TEXT("No Ability System Component found."));
		}
		else
		{
			KaosSlateIM::ErrorText(TEXT("No Actor selected."));
		}
	}
}


void FKaosWorldDebugger_GameplayEffects::DrawWorldDebugger_Effects(const TArray<FKaosGameplayEffectDebug>& Effects)
{
	SlateIM::BeginTable();
                // Column 1: Effect Name (clickable)
                SlateIM::InitialTableColumnWidth(200.f); SlateIM::AddTableColumn(TEXT("Effect Name"));
                // Column 2: Replication ID
                SlateIM::InitialTableColumnWidth(80.f);  SlateIM::AddTableColumn(TEXT("Inhibited"));
                // Column 3: Duration
                SlateIM::InitialTableColumnWidth(100.f); SlateIM::AddTableColumn(TEXT("Duration"));
                // Column 4: Stacks
                SlateIM::InitialTableColumnWidth(100.f); SlateIM::AddTableColumn(TEXT("Stacks"));

                for (const auto& E : Effects)
                {
                    const FString DurationText = E.Duration == -1.f ? "Infinite" : FString::Printf(TEXT("%.1f"), E.Duration);
                    const FString StacksText   = FString::FromInt(E.Stacks);
                    const FString InihibitedText       = E.bInhibited
                                                ? TEXT("Yes")
                                                : TEXT("No");

                    // Name cell: click to select by ReplicationID
                    if (SlateIM::NextTableCell())
                    {
                        if (SlateIM::Button(E.Effect,  &FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton")))
                        {
                            SelectedEffectReplicationID = E.ReplicationID;
                        }
                    }

                    // InihibitedText cell
                    if (SlateIM::NextTableCell())
                    {
                        SlateIM::Text(InihibitedText);
                    }
                    // Duration
                    if (SlateIM::NextTableCell())
                    {
                        SlateIM::Text(DurationText);
                    }
                    // Stacks
                    if (SlateIM::NextTableCell())
                    {
                        SlateIM::Text(StacksText);
                    }
                }
            SlateIM::EndTable();
}

void FKaosWorldDebugger_GameplayEffects::DrawWorldDebugger_EffectDetails(const TArray<FKaosGameplayEffectDebug>& Effects)
{
	
	SlateIM::BeginVerticalStack();
    if (SelectedEffectReplicationID.IsSet())
    {
        const int32 TargetID = SelectedEffectReplicationID.GetValue();
        // Find the effect with that ReplicationID
        const FKaosGameplayEffectDebug* Sel = Effects.FindByPredicate(
            [&](auto& E) { return E.ReplicationID == TargetID; }
        );

        if (Sel)
        {
            // Display all your detailed fields
            SlateIM::HAlign(HAlign_Fill);
            SlateIM::Text(FString::Printf(TEXT("Effect: %s"), *Sel->Effect), &FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("WorldBrowser.StatusBarText"));
            SlateIM::HAlign(HAlign_Fill);
            SlateIM::Text(FString::Printf(TEXT("Context: %s"), *Sel->Context));
            SlateIM::HAlign(HAlign_Fill);
            SlateIM::Text(FString::Printf(TEXT("Replication ID: %d"), Sel->ReplicationID));
            SlateIM::HAlign(HAlign_Fill);
            SlateIM::Text(FString::Printf(TEXT("Duration: %.2f   Period: %.2f"), Sel->Duration, Sel->Period));
            SlateIM::HAlign(HAlign_Fill);
            SlateIM::Text(FString::Printf(TEXT("Stacks: %d   Level: %.1f"), Sel->Stacks, Sel->Level));
            SlateIM::HAlign(HAlign_Fill);
            SlateIM::Text(FString::Printf(TEXT("Inhibited: %s"), Sel->bInhibited ? TEXT("Yes") : TEXT("No")));
            SlateIM::Spacer({0,6});
            if (Sel->SetByCallerNameMagnitudes.Num())
            {
                SlateIM::HAlign(HAlign_Fill);
                SlateIM::Text(TEXT("SetByCaller (Name → Magnitude):"));
                for (auto& P : Sel->SetByCallerNameMagnitudes)
                {
                    SlateIM::HAlign(HAlign_Fill);
                    SlateIM::Text(FString::Printf(TEXT("  %s → %.2f"), *P.Key.ToString(), P.Value));
                }
                SlateIM::Spacer({0,4});
            }

            if (Sel->SetByCallerTagMagnitudes.Num())
            {
                SlateIM::HAlign(HAlign_Fill);
                SlateIM::Text(TEXT("SetByCaller (Tag → Magnitude):"));
                for (auto& P : Sel->SetByCallerTagMagnitudes)
                {
                    SlateIM::HAlign(HAlign_Fill);
                    SlateIM::Text(FString::Printf(TEXT("  %s → %.2f"), *P.Key.ToString(), P.Value));
                }
                SlateIM::Spacer({0,4});
            }

            if (!Sel->DynamicAssetTags.IsEmpty())
            {
                SlateIM::HAlign(HAlign_Fill);
                SlateIM::Text(FString::Printf(TEXT("Dynamic Asset Tags: %s"), *Sel->DynamicAssetTags.ToStringSimple()));
            }
            if (!Sel->DynamicGrantedTags.IsEmpty())
            {
                SlateIM::HAlign(HAlign_Fill);
                SlateIM::Text(FString::Printf(TEXT("Dynamic Granted Tags: %s"), *Sel->DynamicGrantedTags.ToStringSimple()));
            }
            SlateIM::Spacer({0,6});

            if (Sel->ModifiedAttributes.Num())
            {
                SlateIM::HAlign(HAlign_Fill);
                SlateIM::Text(TEXT("Modified Attributes:"));
                for (auto& MA : Sel->ModifiedAttributes)
                {
                    SlateIM::HAlign(HAlign_Fill);
                    SlateIM::Text(FString::Printf(TEXT("  %s → %.2f"), *MA.Attribute.GetName(), MA.TotalMagnitude));
                }
                SlateIM::Spacer({0,6});
            }

            if (Sel->Definition.IsValid())
            {
                SlateIM::HAlign(HAlign_Fill);
                SlateIM::Text(TEXT("Definition Details (CDO):"));
                SlateIM::HAlign(HAlign_Fill);
                SlateIM::Fill();
                SlateIM::BeginScrollBox(Orient_Vertical);
                SlateIM::HAlign(HAlign_Fill);
                SlateIM::BeginVerticalStack();
                SlateIM::HAlign(HAlign_Fill);
                SlateIM::VAlign(VAlign_Fill);
                if (!Sel->Definition->Modifiers.IsEmpty())
                {
                	SlateIM::HAlign(HAlign_Fill);
                	SlateIM::Text(TEXT("Modifiers:"), &FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("WorldBrowser.StatusBarText"));
                	for (const auto& Modifier : Sel->Definition->Modifiers)
                	{
                		// 1) Attribute name
                		const FString AttrName = Modifier.Attribute.AttributeName;
                		SlateIM::HAlign(HAlign_Fill);
                		SlateIM::Text(FString::Printf(TEXT("• %s"), *AttrName));

                		// 2) Magnitude calculation type
                		const auto CalcType = Modifier.ModifierMagnitude.GetMagnitudeCalculationType();
                		const FString CalcTypeName = StaticEnum<EGameplayEffectMagnitudeCalculation>()
							->GetNameStringByValue((int64)CalcType);
                		SlateIM::HAlign(HAlign_Fill);
                		SlateIM::Text(FString::Printf(TEXT("    Calc Type: %s"), *CalcTypeName));

                		// 3) Custom calculation class (if any)
                		const UClass* CustomClass = Modifier.ModifierMagnitude.GetCustomMagnitudeCalculationClass();
                		const FString CustomClassName = CustomClass 
							? CustomClass->GetName() 
							: TEXT("None");
                		SlateIM::HAlign(HAlign_Fill);
                		SlateIM::Text(FString::Printf(TEXT("    Custom Class: %s"), *CustomClassName));

                		// 4) Static magnitude (if available)
                		float StaticMag = 0.f;
                		if (Modifier.ModifierMagnitude.GetStaticMagnitudeIfPossible(Sel->Level, StaticMag))
                		{
                			SlateIM::HAlign(HAlign_Fill);
                			SlateIM::Text(FString::Printf(TEXT("    Static Magnitude: %.2f"), StaticMag));
                		}

                		// spacing between entries
                		SlateIM::HAlign(HAlign_Fill);
                		SlateIM::Spacer(FVector2D(0, 6));
                	}
                }
                if (!Sel->Definition->Executions.IsEmpty())
                {
                	SlateIM::HAlign(HAlign_Fill);
                	SlateIM::Text(TEXT("Executions:"), &FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("WorldBrowser.StatusBarText"));
                	for (const auto& Exec : Sel->Definition->Executions)
                	{
                		const FString ExecClass = GetNameSafe(Exec.CalculationClass);
                		SlateIM::HAlign(HAlign_Fill);
                		SlateIM::Text(FString::Printf(TEXT("• %s"), *ExecClass));

                		const auto PassedInTags = Exec.PassedInTags.ToStringSimple();
                		SlateIM::HAlign(HAlign_Fill);
                		SlateIM::Text(FString::Printf(TEXT("    Passed In Tag: %s"), *PassedInTags));

                		if (!Exec.ConditionalGameplayEffects.IsEmpty())
                		{
                			SlateIM::HAlign(HAlign_Fill);
                			SlateIM::Text(TEXT("    Conditional Effects:"));
                			for (const auto& CGE : Exec.ConditionalGameplayEffects)
                			{
                				const FString CGEName = GetNameSafe(CGE.EffectClass);
                				SlateIM::Text(FString::Printf(TEXT("        %s"), *CGEName));
                			}
                		}
                		if (!Exec.CalculationModifiers.IsEmpty())
                		{
                			SlateIM::HAlign(HAlign_Fill);
                			SlateIM::Text(TEXT("    Calculation Modifiers:"));
                			for (const auto& Modifier : Exec.CalculationModifiers)
                			{
                				const FString CaptureStr = Modifier.CapturedAttribute.ToSimpleString();
                				SlateIM::HAlign(HAlign_Fill);
                				SlateIM::Text(FString::Printf(TEXT("        %s"), *CaptureStr));
                				//Modifier.ModifierMagnitude.GetValueForEditorDisplay()
                			}
                		}
                	
                		// spacing between entries
                		SlateIM::HAlign(HAlign_Fill);
                		SlateIM::Spacer(FVector2D(0, 6));
                	}
                }
                SlateIM::HAlign(HAlign_Fill);
                SlateIM::Text(FString::Printf(TEXT("Stacking Type: %s"), *StaticEnum<EGameplayEffectStackingType>()->GetNameStringByValue((int64)Sel->Definition->StackingType)), &FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("WorldBrowser.StatusBarText"));
                if (Sel->Definition->StackingType > EGameplayEffectStackingType::None)
                {
                	SlateIM::HAlign(HAlign_Fill);
                	SlateIM::Text(FString::Printf(TEXT("Stacking Limit: %d"), Sel->Definition->StackLimitCount));
                	SlateIM::HAlign(HAlign_Fill);
                	SlateIM::Text(FString::Printf(TEXT("Duration Refresh Policy: %s"),  *StaticEnum<EGameplayEffectStackingDurationPolicy>()->GetNameStringByValue((int64)Sel->Definition->StackDurationRefreshPolicy)));
                	SlateIM::HAlign(HAlign_Fill);
                	SlateIM::Text(FString::Printf(TEXT("Stack Expiration Policy: %s"),  *StaticEnum<EGameplayEffectStackingExpirationPolicy>()->GetNameStringByValue((int64)Sel->Definition->StackExpirationPolicy)));
                	SlateIM::HAlign(HAlign_Fill);
                	SlateIM::Text(FString::Printf(TEXT("Stack Period Reset Policy: %s"),  *StaticEnum<EGameplayEffectStackingPeriodPolicy>()->GetNameStringByValue((int64)Sel->Definition->StackPeriodResetPolicy)));
                }
                if (!Sel->Definition->GetGrantedTags().IsEmpty())
                {
                	SlateIM::HAlign(HAlign_Fill);
                	SlateIM::Text(FString::Printf(TEXT("Granted to Target Tags: %s"), *Sel->Definition->GetGrantedTags().ToStringSimple()));
                }
                if (!Sel->Definition->GetAssetTags().IsEmpty())
                {
                	SlateIM::HAlign(HAlign_Fill);
                	SlateIM::Text(FString::Printf(TEXT("Asset Tags: %s"), *Sel->Definition->GetAssetTags().ToStringSimple()));
                }
                SlateIM::EndVerticalStack();
                SlateIM::EndScrollBox();
            }
        }
        else
        {
			KaosSlateIM::ErrorText(TEXT("Selected Effect not found"));
        }
    }
    else
    {
    	KaosSlateIM::WarningText(TEXT("Click an effects name on the left to view details here."));
    }
	SlateIM::EndVerticalStack();
}

FSlateIcon FKaosWorldDebugger_GameplayEffects::GetTabIcon() const
{
	static const FSlateIcon MyIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.GameStateBase");

	return MyIcon;
}


TArray<FKaosWorldDebugger_GameplayEffects::FKaosGameplayEffectDebug> FKaosWorldDebugger_GameplayEffects::CollectEffectsData(const UAbilitySystemComponent* AbilityComp) const
{
	TArray<FKaosGameplayEffectDebug> DebugEffects;

	for (const FActiveGameplayEffect& ActiveGE : &AbilityComp->GetActiveGameplayEffects())
	{
		FKaosGameplayEffectDebug& ItemData = DebugEffects.AddDefaulted_GetRef();
		ItemData.ReplicationID = ActiveGE.ReplicationID;
		ItemData.bInhibited = ActiveGE.bIsInhibited;
		ItemData.Duration = ActiveGE.GetDuration();
		ItemData.Period = ActiveGE.GetPeriod();

		const FGameplayEffectSpec& EffectSpec = ActiveGE.Spec;
		ItemData.Effect = EffectSpec.ToSimpleString();
		ItemData.Effect.RemoveFromStart(DEFAULT_OBJECT_PREFIX);
		ItemData.Effect.RemoveFromEnd(TEXT("_C"));

		ItemData.Context = EffectSpec.GetContext().ToString();
		ItemData.Stacks = EffectSpec.GetStackCount();
		ItemData.Level = EffectSpec.GetLevel();
		ItemData.DynamicGrantedTags = EffectSpec.DynamicGrantedTags;
		ItemData.DynamicAssetTags = EffectSpec.GetDynamicAssetTags();
		ItemData.ModifiedAttributes = EffectSpec.ModifiedAttributes;
		ItemData.SetByCallerTagMagnitudes = EffectSpec.SetByCallerTagMagnitudes;
		ItemData.SetByCallerNameMagnitudes = EffectSpec.SetByCallerNameMagnitudes;
		ItemData.Definition = EffectSpec.Def;
	}

	DebugEffects.Sort([](const FKaosGameplayEffectDebug& A, const FKaosGameplayEffectDebug& B)
	{
		return A.Effect < B.Effect;
	});
	return DebugEffects;
}
#endif