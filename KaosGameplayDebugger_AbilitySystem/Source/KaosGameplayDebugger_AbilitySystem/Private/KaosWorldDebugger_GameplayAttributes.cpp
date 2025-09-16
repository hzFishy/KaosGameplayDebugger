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

#include "KaosWorldDebugger_GameplayAttributes.h"
#if WITH_KAOS_GAMEPLAYDEBUGGER
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "KaosSlateIMHelpers.h"

void FKaosWorldDebugger_GameplayAttributes::DrawDetails(const FKaosDebuggerContext& Context)
{
	AActor* ContextActor = Cast<AActor>(Context.ContextObject.Get());
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ContextActor))
	{
		if (ContextActor != LastSelectedActor)
		{
			SelectedAttributeClass.Reset();
			SelectedAttributeName.Reset();
		}
		LastSelectedActor = ContextActor;
		TArray<FKaosGameplayAttributeDebug> Attributes = CollectGameplayAttributes(ASC);
		if (Attributes.IsEmpty())
		{
			SlateIM::Text(TEXT("No Gameplay Attributes found."));
			return;
		}
		SlateIM::BeginHorizontalStack();
		DrawWorldDebugger_Attributes(Attributes);
		SlateIM::Fill();
		SlateIM::HAlign(HAlign_Fill);
		SlateIM::VAlign(VAlign_Fill);
		DrawWorldDebugger_AttributeDetails(Attributes);
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

TArray<FKaosWorldDebugger_GameplayAttributes::FKaosGameplayAttributeDebug> FKaosWorldDebugger_GameplayAttributes::CollectGameplayAttributes(
	const UAbilitySystemComponent* AbilityComp)
{
		TArray<FKaosGameplayAttributeDebug> DebugAttributes;

	const AActor* Actor = AbilityComp ? AbilityComp->GetOwner() : nullptr;
	if (!Actor || !Actor->GetWorld())
	{
		FKaosGameplayAttributeDebug& DebugAttribute = DebugAttributes.AddDefaulted_GetRef();
		DebugAttribute.AttributeName = TEXT("Invalid AbilitySystemComponent");
		return DebugAttributes;
	}

	// We need to do a lot of work to detect if the AttributeSet is replicated which only occurs of AbilitySystemComponent is replicated
	const ENetRole LocalRole = Actor->GetLocalRole();
	const ELifetimeCondition NetCondition = Actor->AllowActorComponentToReplicate(AbilityComp);

	bool bASCReplicates = (NetCondition != ELifetimeCondition::COND_Never) && AbilityComp->IsSupportedForNetworking() && AbilityComp->IsNameStableForNetworking();

	// Step 2: Collect effects by attribute
	TMultiMap<FGameplayAttribute, FActiveGameplayEffectsContainer::DebugExecutedGameplayEffectData> EffectMap;
	AbilityComp->GetActiveGameplayEffects().GetActiveGameplayEffectDataByAttribute(EffectMap);
	
	// Grab the AttributeSet rather than the Attributes themselves so we can check the network functionality
	for (const UAttributeSet* AttributeSet : AbilityComp->GetSpawnedAttributes())
	{
		const TSubclassOf<UAttributeSet> AttributeSetClass = AttributeSet ? AttributeSet->GetClass() : nullptr;
		if (!AttributeSet || !AttributeSetClass) // !AttributeSet needed for static analysis
		{
			continue;
		}

		// These are all the replication conditions per variable
		TArray<FLifetimeProperty> LifetimeProps;
		AttributeSet->GetLifetimeReplicatedProps(LifetimeProps);

		// Network status can change per AttributeSet, so figure it out
		const bool bAttributeSetReplicates = bASCReplicates && AttributeSet->IsSupportedForNetworking();

		// Now just gather the debug data
		TArray<FGameplayAttribute> LocalAttributes;
		UAttributeSet::GetAttributesFromSetClass(AttributeSetClass, LocalAttributes);
		for (const FGameplayAttribute& Attrib : LocalAttributes)
		{
			TArray<FKaosGameplayAttributeEffectDebugInfo> AttrDebug;
			TArray<FActiveGameplayEffectsContainer::DebugExecutedGameplayEffectData> MatchedEffects;
			EffectMap.MultiFind(Attrib, MatchedEffects);

			for (const auto& DebugData : MatchedEffects)
			{
				FString GEName = DebugData.GameplayEffectName;
				GEName.RemoveFromStart(DEFAULT_OBJECT_PREFIX);
				GEName.RemoveFromEnd(TEXT("_C"));
				AttrDebug.Add({
					GEName,
					DebugData.ActivationState,
					EGameplayModOpToString(DebugData.ModifierOp),
					DebugData.Magnitude,
					DebugData.StackCount
				});
			}
			
			// Add the data to our returned array
			FKaosGameplayAttributeDebug& DebugAttribute = DebugAttributes.Add_GetRef(
				{
					.AttributeName = Attrib.AttributeName,
					.BaseValue = AbilityComp->GetNumericAttributeBase(Attrib),
					.CurrentValue = Attrib.GetNumericValue(AttributeSet),
					.AttributeSetClass = AttributeSetClass->GetName(),
					.ModifyingEffects = AttrDebug
				});
		}
	}
	
	DebugAttributes.Sort([](const FKaosGameplayAttributeDebug& A, const FKaosGameplayAttributeDebug& B)
	{
		return A.AttributeName < B.AttributeName;
	});
	return DebugAttributes;
}

void FKaosWorldDebugger_GameplayAttributes::DrawWorldDebugger_Attributes(const TArray<FKaosGameplayAttributeDebug>& Attributes)
{
    SlateIM::BeginTable();
    SlateIM::InitialTableColumnWidth(250.f); SlateIM::AddTableColumn(TEXT("Attribute"));
    SlateIM::InitialTableColumnWidth( 80.f);  SlateIM::AddTableColumn(TEXT("Base"));
    SlateIM::InitialTableColumnWidth( 80.f);  SlateIM::AddTableColumn(TEXT("Current"));
    SlateIM::InitialTableColumnWidth(180.f); SlateIM::AddTableColumn(TEXT("Class"));

    for (const auto& A : Attributes)
    {
        if (SlateIM::NextTableCell() && SlateIM::Button(A.AttributeName, &FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton")))
        {
        	SlateIM::Fill();
            SelectedAttributeName  = A.AttributeName;
            SelectedAttributeClass = A.AttributeSetClass;
        }
        if (SlateIM::NextTableCell()) { SlateIM::Fill(); SlateIM::Text(FString::Printf(TEXT("%.2f"), A.BaseValue)); }
        if (SlateIM::NextTableCell()) { SlateIM::Fill(); SlateIM::Text(FString::Printf(TEXT("%.2f"), A.CurrentValue)); }
        if (SlateIM::NextTableCell()) { SlateIM::Fill(); SlateIM::Text(A.AttributeSetClass); }
    }
    SlateIM::EndTable();
}

void FKaosWorldDebugger_GameplayAttributes::DrawWorldDebugger_AttributeDetails(const TArray<FKaosGameplayAttributeDebug>& Attributes)
{
	SlateIM::BeginScrollBox();
	SlateIM::BeginVerticalStack();
	if (SelectedAttributeName.IsSet() && SelectedAttributeClass.IsSet())
	{
		const FKaosGameplayAttributeDebug* Sel = Attributes.FindByPredicate(
			[&](auto& X) {
				return X.AttributeName  == SelectedAttributeName.GetValue() &&
					   X.AttributeSetClass == SelectedAttributeClass.GetValue();
			});

		if (Sel)
		{
			SlateIM::HAlign(HAlign_Fill);
			SlateIM::Text(FString::Printf(TEXT("Attribute: %s"), *Sel->AttributeName));
			SlateIM::HAlign(HAlign_Fill);
			SlateIM::Text(FString::Printf(TEXT("Base: %.2f"),   Sel->BaseValue));
			SlateIM::HAlign(HAlign_Fill);
			SlateIM::Text(FString::Printf(TEXT("Current: %.2f"),Sel->CurrentValue));
			SlateIM::HAlign(HAlign_Fill);
			SlateIM::Text(FString::Printf(TEXT("Set Class: %s"),*Sel->AttributeSetClass));
			SlateIM::Spacer(FVector2D(0,8));
			SlateIM::HAlign(HAlign_Fill);
			SlateIM::Text(TEXT("Gameplay Effects:"));
			SlateIM::Spacer(FVector2D(0,4));
			SlateIM::HAlign(HAlign_Fill);
			SlateIM::MaxHeight(300.f); // ⬅ constrain height
			SlateIM::BeginScrollBox(EOrientation::Orient_Vertical);
			for (auto& E : Sel->ModifyingEffects)
			{
				SlateIM::Padding(FMargin(6));
				SlateIM::HAlign(HAlign_Fill);
				SlateIM::BeginVerticalStack();
				SlateIM::HAlign(HAlign_Fill);

				SlateIM::Text(
					FString::Printf(TEXT("• %s (%s)"), *E.EffectName, *E.ActivationState),
					FLinearColor(0.8f, 0.85f, 1.0f)
				);
				SlateIM::HAlign(HAlign_Fill);
				SlateIM::Text(FString::Printf(TEXT("  %s: %.2f"), *E.ModifierOp, E.Magnitude));
				SlateIM::HAlign(HAlign_Fill);
				SlateIM::Text(FString::Printf(TEXT("  Stack Count: %d"), E.StackCount));
				SlateIM::EndVerticalStack();
				SlateIM::Spacer(FVector2D(0,6));
			}
			SlateIM::EndScrollBox();
		}
		else
		{
			KaosSlateIM::ErrorText(TEXT("Selected Attribute not found"));
		}
	}
	else
	{
		KaosSlateIM::WarningText(TEXT("Click an attributes name on the left to view details here."));
	}
	SlateIM::EndVerticalStack();
	SlateIM::EndScrollBox();
}

FSlateIcon FKaosWorldDebugger_GameplayAttributes::GetTabIcon() const
{
	static const FSlateIcon MyIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "AnimGraph.Attribute.Attributes.Icon");

	return MyIcon;
}
#endif
