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
#include "UObject/Object.h"
#include "Brushes/SlateColorBrush.h"
#include "SlateIM.h"
#include "SlateIMWidgetBase.h"

#if WITH_KAOS_GAMEPLAYDEBUGGER

class FSlateIMKaosCheats
{
public:
	void Draw(float DeltaTime);
	void SetWorld(TWeakObjectPtr<UWorld> InWorld);
	bool ShouldExposeFunction(UFunction* Func);
	void CacheCheatFunctions(APlayerController* PC);
	void Reset();
private:
	TWeakObjectPtr<UWorld> World;

	struct FCheatParam
	{
		FProperty* Property = nullptr;
		FString Name;
		FString TypeName;
		int32 Offset = 0;
	};
	
	struct FCheatFunction
	{
		UObject* Target = nullptr;
		UFunction* Function = nullptr;
		FString Name;
		bool bHasParameters = false;
		TArray<FCheatParam> Params;
	};

	
	TArray<FCheatFunction> CachedFunctions;
	TMap<FString, int> IntInputs;
	TMap<FString, float> FloatInputs;
	TMap<FString, double> DoubleInputs;
	TMap<FString, bool> BoolInputs;
	TMap<FString, FString> StringInputs;

	FDelegateHandle CleanupHandle;
};

class FKaosSlateCheatWidget : public FSlateIMWidgetBase
{
public:
	FKaosSlateCheatWidget()
		: FSlateIMWidgetBase(TEXT("Cheat Widget"))
	{
		Layout.Anchors = FAnchors(.95f, .15f);        // Top-right
		Layout.Alignment = FVector2f(1.f, 0.f);     // Align top-right corner of widget
		Layout.Size = FVector2f(300.f, 500.f);      // Fixed size
		Layout.ZOrder = 0;
	}

	void Reset();
	void SetLocalPlayer(ULocalPlayer* LocalPlayer);

protected:
	virtual void DrawWindow(float DeltaTime);

private:
	TWeakObjectPtr<class ULocalPlayer> LocalPlayerCache;
	bool bIsReset = false;
	FDelegateHandle BoundDelegate;

	virtual void DrawWidget(float DeltaTime) final override;
	FSlateIMKaosCheats CheatWidget;
	FString WindowTitle = "Cheat Widget";
	FVector2f WindowSize = FVector2f(700, 900);
	SlateIM::FViewportRootLayout Layout;
	bool bCheatsCollapsed = true; 
	FSlateColorBrush TransparentBackground = FLinearColor(0.f, 0.f, 0.f, 0.3f); // 30% opacity
};
#endif