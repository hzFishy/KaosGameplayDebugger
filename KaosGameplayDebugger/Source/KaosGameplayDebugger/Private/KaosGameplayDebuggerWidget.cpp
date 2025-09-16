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

#include "KaosGameplayDebuggerWidget.h"

#if WITH_KAOS_GAMEPLAYDEBUGGER
#include "Engine/Engine.h"
#include "Misc/App.h"
#include "KaosGameplayDebuggerInfoProviderInterface.h"
#include "Algo/Compare.h"
#include "Styling/SlateStyle.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "EngineUtils.h"
#include "KaosGameplayDebuggerModule.h"
#include "ProfilingDebugging/ScopedTimers.h"

void FKaosGameplayDebuggerWidget::DrawWindow(float DeltaTime)
{
	// --- Timing ---
	double LastTime = CurrentTime;
	CurrentTime = 0;
	FScopedDurationTimer Timer(CurrentTime);

	TimeSinceLastUpdate += FApp::GetDeltaTime();
	if (TimeSinceLastUpdate > 0.5f)
	{
		TimeText = FString::Printf(TEXT("%.3f ms"), LastTime * 1000);
		TimeSinceLastUpdate = 0;
	}

	SCOPED_NAMED_EVENT_TEXT("FKaosGameplayDebuggerWidget::Draw", FColorList::Goldenrod);
	
	// --- Tab Group ---
	SlateIM::Fill();
	SlateIM::HAlign(HAlign_Fill);
	SlateIM::VAlign(VAlign_Fill);
	SlateIM::BeginTabGroup(TEXT("MainTabGroup"));
	SlateIM::Fill();
	SlateIM::HAlign(HAlign_Fill);
	SlateIM::VAlign(VAlign_Fill);
	SlateIM::BeginTabStack();

	FKaosGameplayDebuggerModule& Module = FKaosGameplayDebuggerModule::Get();
	TArray<TSharedPtr<IKaosDebuggerBaseItem>> MainTabs = Module.GetRegisteredMainTabs();

	MainTabs.Sort([](const TSharedPtr<IKaosDebuggerBaseItem>& A, const TSharedPtr<IKaosDebuggerBaseItem>& B)
	{
		return A->GetTabOrder() < B->GetTabOrder();
	});
	FKaosDebuggerContext Context;
	Context.DeltaTime = DeltaTime;
	for (const TSharedPtr<IKaosDebuggerBaseItem>& Tab : MainTabs)
	{
		if (!Tab.IsValid())
		{
			continue;
		}
		if (SlateIM::BeginTab(FName(*Tab->GetTabLabel().ToString()), Tab->GetTabIcon(), Tab->GetTabLabel()))
		{
			SlateIM::Fill();
			SlateIM::HAlign(HAlign_Fill);
			SlateIM::VAlign(VAlign_Fill);
			Tab->DrawDetails(Context);
		}
		SlateIM::EndTab();
	}

	SlateIM::EndTabStack();
	SlateIM::EndTabGroup();
}
#endif