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
#pragma once

#include "CoreMinimal.h"
#if WITH_KAOS_GAMEPLAYDEBUGGER
#include "GameplayEffect.h"
#include "KaosDebuggerBaseItem.h"
#include "KaosWorldDebugger_AbilitySystemTypes.h"


class UGameplayEffect;
class UAbilitySystemComponent;

struct FKaosWorldDebugger_GameplayAttributes : public IKaosDebuggerBaseItem
{
public:
	virtual void DrawDetails(const FKaosDebuggerContext& Context) override;
	
private:
	TOptional<FString> SelectedAttributeName;
	TOptional<FString> SelectedAttributeClass;
	TWeakObjectPtr<class AActor> LastSelectedActor;


	struct FKaosGameplayAttributeEffectDebugInfo
	{
		FString EffectName;
		FString ActivationState;
		FString ModifierOp;
		float Magnitude = 0.f;
		int32 StackCount = 0;
	};
	
	struct FKaosGameplayAttributeDebug
	{
		FString AttributeName;
		float BaseValue = 0.0f;
		float CurrentValue = 0.0f;
		FString AttributeSetClass;
		TArray<FKaosGameplayAttributeEffectDebugInfo> ModifyingEffects;
	};
	TArray<FKaosGameplayAttributeDebug>  CollectGameplayAttributes(const UAbilitySystemComponent* AbilityComp);

	void DrawWorldDebugger_Attributes(const TArray<FKaosGameplayAttributeDebug>& Attributes);
	void DrawWorldDebugger_AttributeDetails(const TArray<FKaosGameplayAttributeDebug>& Attributes);

public:
	virtual FText GetTabLabel() const override { return FText::FromString(TEXT("Gameplay Attributes")); }
	virtual FSlateIcon GetTabIcon() const override;;

};

#endif