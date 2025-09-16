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
#include "KaosDebuggerBaseItem.h"


class UGameplayEffect;
class UAbilitySystemComponent;

struct FKaosWorldDebugger_World_Details : public IKaosDebuggerBaseItem
{
public:
	virtual void DrawDetails(const FKaosDebuggerContext& Context) override;

private:

	struct FKaosReplicatedActorInfo
	{
		FString ActorName;
		FString ClassName;
		FString Dormancy;
		float NetUpdateFreq = 0.f;
		float NetUpdatePriority = 0.f;
	};

	struct FKaosNetworkStatsCache
	{
		int32 LastReplicatedActorCount = -1;
		FBox CachedBoundingBox;
		TMap<FString, int32> CachedDormancyCounts;
		TArray<TPair<FString, int32>> CachedClassCounts;
		TArray<TPair<FString, int32>> CachedActualClassCounts;
		TArray<FKaosReplicatedActorInfo> AwakeActors;
		TArray<FKaosReplicatedActorInfo> DormantActors;
	};

	TMap<TWeakObjectPtr<UWorld>, FKaosNetworkStatsCache> CachedNetworkStats;
	void GatherNetworkStatsIfNeeded(UWorld* World, FKaosNetworkStatsCache& Cache);
	void DrawNetworkTab(const FKaosDebuggerContext& Context, FKaosNetworkStatsCache& Cache);
	void DrawActorTable(const TArray<FKaosReplicatedActorInfo>& Actors);

public:
	virtual FText GetTabLabel() const override { return FText::FromString(TEXT("World Details")); }
	virtual FSlateIcon GetTabIcon() const override;;
};

#endif