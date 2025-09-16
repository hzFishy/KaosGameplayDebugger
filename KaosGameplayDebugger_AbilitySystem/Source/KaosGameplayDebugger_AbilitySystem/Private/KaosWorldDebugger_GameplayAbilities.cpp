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

#include "KaosWorldDebugger_GameplayAbilities.h"
#if WITH_KAOS_GAMEPLAYDEBUGGER
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "KaosSlateIMHelpers.h"

void FKaosWorldDebugger_GameplayAbilities::DrawDetails(const FKaosDebuggerContext& Context)
{
	AActor* ContextActor = Cast<AActor>(Context.ContextObject.Get());

	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ContextActor))
	{
		if (ContextActor != LastSelectedActor)
		{
			SelectedAbilityName.Reset();
		}
		LastSelectedActor = ContextActor;
		TArray<FKaosGameplayAbilityDebug> Abilities = CollectAbilityData(ASC);
		if (Abilities.IsEmpty())
		{
			SlateIM::Text(TEXT("No Gameplay Abilities found."));
			return;
		}

		SlateIM::BeginHorizontalStack();
		DrawWorldDebugger_Abilities(Abilities);
		SlateIM::Fill();
		SlateIM::HAlign(HAlign_Fill);
		SlateIM::VAlign(VAlign_Fill);
		DrawWorldDebugger_AbilityDetails(Abilities);
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

void FKaosWorldDebugger_GameplayAbilities::DrawWorldDebugger_Abilities(const TArray<FKaosGameplayAbilityDebug>& Abilities)
{
	SlateIM::BeginTable();
	SlateIM::FixedTableColumnWidth(250.f); SlateIM::AddTableColumn(TEXT("Ability"));
	SlateIM::FixedTableColumnWidth( 80.f);  SlateIM::AddTableColumn(TEXT("Source"));
	SlateIM::FixedTableColumnWidth( 80.f);  SlateIM::AddTableColumn(TEXT("Level"));
	SlateIM::FixedTableColumnWidth(120.f); SlateIM::AddTableColumn(TEXT("Active"));

	for (const auto& A : Abilities)
	{
		if (SlateIM::NextTableCell() && SlateIM::Button(A.Ability, &FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton")))
		{
			SelectedAbilityName  = A.Ability;
		}
		if (SlateIM::NextTableCell()) SlateIM::Text(FString::Printf(TEXT("%s"), *A.Source));
		if (SlateIM::NextTableCell()) SlateIM::Text(FString::Printf(TEXT("%d"), A.Level));
		if (SlateIM::NextTableCell()) SlateIM::Text(FString::Printf(TEXT("%s"), A.bIsActive ? TEXT("Active") : TEXT("Inactive")));
	}
	SlateIM::EndTable();
}

void FKaosWorldDebugger_GameplayAbilities::DrawWorldDebugger_AbilityDetails(const TArray<FKaosGameplayAbilityDebug>& Abilities)
{
	SlateIM::Fill();
	SlateIM::BeginScrollBox();
	SlateIM::Fill();
	SlateIM::VAlign(VAlign_Fill);
	SlateIM::BeginVerticalStack();
	if (SelectedAbilityName.IsSet())
	{
		const FKaosGameplayAbilityDebug* Sel = Abilities.FindByPredicate(
			[&](const FKaosGameplayAbilityDebug& X) {
				return X.Ability == SelectedAbilityName.GetValue();
			});

		if (Sel)
		{
			static FTextBlockStyle AbilityTextStyle = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("FoliageEditMode.ActiveToolName.Text");
			KaosSlateIM::DrawLabledText(TEXT("Ability"), FSlateColor(FColor::Red), Sel->Ability );
			KaosSlateIM::DrawLabledText(TEXT("Source Object"), FSlateColor::UseForeground(), Sel->Source);
			KaosSlateIM::DrawLabledText(TEXT("Level"), FSlateColor::UseForeground(), FString::FromInt(Sel->Level));
			KaosSlateIM::DrawLabledText(TEXT("Active"), FSlateColor::UseForeground(), Sel->bIsActive ? TEXT("Active") : TEXT("Inactive"));
			KaosSlateIM::DrawLabledText(TEXT("Asset Tags"), FSlateColor::UseForeground(), Sel->AbilityTags.ToStringSimple());
			KaosSlateIM::DrawLabledText(TEXT("Cooldown Tags"), FSlateColor::UseForeground(), Sel->CooldownTags.ToStringSimple());
			KaosSlateIM::DrawLabledText(TEXT("Input Pressed"), FSlateColor::UseForeground(), Sel->InputPressed ? TEXT("Yes") : TEXT("No"));
			KaosSlateIM::DrawLabledText(TEXT("Active Count"), FSlateColor::UseForeground(), FString::FromInt(Sel->ActiveCount));

			if (Sel->SetByCallerTagMagnitudes.Num() > 0)
			{
				SlateIM::Spacer(FVector2D(0, 8));
				KaosSlateIM::DrawLabledText(TEXT("Set By Callers"), FSlateColor::UseSubduedForeground(), TEXT(""));
				SlateIM::Spacer(FVector2D(0, 4));
				SlateIM::Fill();
				SlateIM::VAlign(VAlign_Fill);
				SlateIM::BeginScrollBox(EOrientation::Orient_Vertical);
				for (const auto& [Tag, Value] : Sel->SetByCallerTagMagnitudes)
				{
					SlateIM::Padding(FMargin(6));
					SlateIM::BeginVerticalStack();

					KaosSlateIM::DrawLabledText(*Tag.ToString(), FLinearColor(0.8f, 0.85f, 1.0f), FString::Printf(TEXT("%.2f"), Value));

					SlateIM::EndVerticalStack();
					SlateIM::Spacer(FVector2D(0, 6));
				}
				SlateIM::EndScrollBox();
			}
		}
		else
		{
			KaosSlateIM::ErrorText(TEXT("Selected Ability not found"));
		}
	}
	else
	{
		KaosSlateIM::WarningText(TEXT("Click an abilities name on the left to view details here."));
	}
	SlateIM::EndVerticalStack();
	SlateIM::EndScrollBox();
}

FSlateIcon FKaosWorldDebugger_GameplayAbilities::GetTabIcon() const
{
	static const FSlateIcon MyIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.GameModeBase");

	return MyIcon;
}


TArray<FKaosWorldDebugger_GameplayAbilities::FKaosGameplayAbilityDebug> FKaosWorldDebugger_GameplayAbilities::CollectAbilityData(
	const UAbilitySystemComponent* AbilityComp) const
{
	TArray<FKaosGameplayAbilityDebug> Abilities;
	const TArray<FGameplayAbilitySpec>& AbilitySpecs = AbilityComp->GetActivatableAbilities();
	for (int32 Idx = 0; Idx < AbilitySpecs.Num(); Idx++)
	{
		const FGameplayAbilitySpec& AbilitySpec = AbilitySpecs[Idx];
		FKaosGameplayAbilityDebug ItemData;

		ItemData.Ability = GetNameSafe(AbilitySpec.Ability);
		ItemData.Ability.RemoveFromStart(DEFAULT_OBJECT_PREFIX);
		ItemData.Ability.RemoveFromEnd(TEXT("_C"));

		ItemData.Source = GetNameSafe(AbilitySpec.SourceObject.Get());
		ItemData.Source.RemoveFromStart(DEFAULT_OBJECT_PREFIX);

		ItemData.Level = AbilitySpec.Level;
		ItemData.bIsActive = AbilitySpec.IsActive();
		ItemData.ActiveCount = AbilitySpec.ActiveCount;;
		ItemData.InputID = AbilitySpec.InputID;
		ItemData.SetByCallerTagMagnitudes = AbilitySpec.SetByCallerTagMagnitudes;
		ItemData.InputPressed = AbilitySpec.InputPressed;
		ItemData.AbilityTags = AbilitySpec.GetDynamicSpecSourceTags();
		if (AbilitySpec.Ability)
		{
			ItemData.AbilityTags.AppendTags(AbilitySpec.Ability->GetAssetTags());
			if (const FGameplayTagContainer* CDTags = AbilitySpec.Ability->GetCooldownTags())
			{
				ItemData.CooldownTags = *CDTags;
			}
		}
		Abilities.Add(ItemData);
	}
		
	Abilities.Sort([](const FKaosGameplayAbilityDebug& A, const FKaosGameplayAbilityDebug& B)
	{
		return A.Ability < B.Ability;
	});
	return Abilities;
}
#endif