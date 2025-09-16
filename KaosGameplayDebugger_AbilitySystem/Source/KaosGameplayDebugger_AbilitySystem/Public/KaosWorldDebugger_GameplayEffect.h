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

struct FKaosWorldDebugger_GameplayEffects : public IKaosDebuggerBaseItem
{
public:
	virtual void DrawDetails(const FKaosDebuggerContext& Context) override;
	
private:
	struct FKaosGameplayEffectDebug
	{
		int32 ReplicationID = INDEX_NONE; // unique & shared between server/client (or INDEX_NONE if local)
		FString Effect;
		FString Context;
		float Duration = 0.0f;
		float Period = 0.0f;
		int32 Stacks = 0;
		float Level = 0.0f;

		bool bInhibited = false;
		TMap<FName, float>			SetByCallerNameMagnitudes;
		TMap<FGameplayTag, float>	SetByCallerTagMagnitudes;
		FGameplayTagContainer DynamicAssetTags;
		FGameplayTagContainer DynamicGrantedTags;
		TArray<FGameplayEffectModifiedAttribute> ModifiedAttributes;
		TWeakObjectPtr<const UGameplayEffect> Definition;
	};
	TArray<FKaosGameplayEffectDebug> CollectEffectsData(const UAbilitySystemComponent* AbilityComp) const;

	
	TOptional<int32> SelectedEffectReplicationID;
	TWeakObjectPtr<class AActor> LastSelectedActor;

	void DrawWorldDebugger_Effects(const TArray<FKaosGameplayEffectDebug>& Effects);
	void DrawWorldDebugger_EffectDetails(const TArray<FKaosGameplayEffectDebug>& Effects);

public:
	virtual FText GetTabLabel() const override { return FText::FromString(TEXT("Gameplay Effects")); }
	virtual FSlateIcon GetTabIcon() const override;;

};

#endif