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

#include "KaosDebuggerContext.h"
#include "SlateIM.h"
#include "Styling/SlateTypes.h"
#include "Brushes/SlateColorBrush.h"
#include "Styling/SlateStyle.h"
#include "Styling/AppStyle.h"
#include "Textures/SlateIcon.h"
#include "Styling/CoreStyle.h"

struct KAOSGAMEPLAYDEBUGGER_API IKaosDebuggerBaseItem
{
public:
	virtual ~IKaosDebuggerBaseItem() = default;
	virtual FText GetTabLabel() const = 0;
	virtual FSlateIcon GetTabIcon() const { return FSlateIcon();}

	virtual void DrawDetails(const FKaosDebuggerContext& Context) = 0;

	int32 GetTabOrder() const { return TabOrder; }
	
private:
	int32 TabOrder = 0;
	FName TabID = NAME_None;
	friend class FKaosGameplayDebuggerModule;
};
#endif