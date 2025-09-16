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

#include "KaosWorldDebugger_ActorSubTab_AbilitySystem.h"

#include "KaosGameplayDebuggerModule.h"
#if WITH_KAOS_GAMEPLAYDEBUGGER


void FKaosWorldDebugger_ActorSubTab_AbilitySystem::DrawDetails(const FKaosDebuggerContext& Context)
{
	SlateIM::Fill();
	SlateIM::HAlign(HAlign_Fill);
	SlateIM::VAlign(VAlign_Fill);
	SlateIM::BeginTabGroup(TEXT("MainTabGroup"));
	SlateIM::Fill();
	SlateIM::HAlign(HAlign_Fill);
	SlateIM::VAlign(VAlign_Fill);
	SlateIM::BeginTabStack();

	FKaosGameplayDebuggerModule& Module = FKaosGameplayDebuggerModule::Get();
	TArray<TSharedPtr<IKaosDebuggerBaseItem>> SubTab = Module.GetRegisteredSubCategoriesFor(KaosDebugger_AbilitySystemNames::AbilitySystemMainTabID);

	SubTab.Sort([](const TSharedPtr<IKaosDebuggerBaseItem>& A, const TSharedPtr<IKaosDebuggerBaseItem>& B)
	{
		return A->GetTabOrder() < B->GetTabOrder();
	});
	
	for (const TSharedPtr<IKaosDebuggerBaseItem>& Tab : SubTab)
	{
		if (!Tab.IsValid())
		{
			continue;
		}
		SlateIM::Fill();
		SlateIM::HAlign(HAlign_Fill);
		SlateIM::VAlign(VAlign_Fill);
		SlateIM::InitialTableColumnWidth(200.f);
		if (SlateIM::BeginTab(FName(*Tab->GetTabLabel().ToString()), Tab->GetTabIcon(), Tab->GetTabLabel()))
		{
			Tab->DrawDetails(Context);
		}
		SlateIM::EndTab();
	}

	SlateIM::EndTabStack();
	SlateIM::EndTabGroup();
}

FSlateIcon FKaosWorldDebugger_ActorSubTab_AbilitySystem::GetTabIcon() const
{
	static const FSlateIcon MyIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "Profiler.EventGraph.GameThread");

	return MyIcon;
}
#endif
