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

#include "KaosSlateIMHelpers.h"
#include "Styling/SlateStyle.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"

namespace KaosSlateIM
{
#if WITH_KAOS_GAMEPLAYDEBUGGER
	void HeaderText(const FStringView& Text, FSlateColor Color)
	{
		static FTextBlockStyle ThisStyle = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("ProjectBrowser.VersionOverlayText");
		SlateIM::HAlign(HAlign_Fill);
		SlateIM::Text(Text, Color, &ThisStyle);
	}

	void HeaderText(const FStringView& Text)
	{
		static FTextBlockStyle ThisStyle = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("ProjectBrowser.VersionOverlayText");
		SlateIM::HAlign(HAlign_Fill);
		SlateIM::Text(Text, &ThisStyle);
	}
	
	void SubHeaderText(const FStringView& Text)
	{
		static FTextBlockStyle ThisStyle = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("AnimViewport.MessageText");
		SlateIM::HAlign(HAlign_Fill);
		SlateIM::Text(Text, &ThisStyle);
	}

	void SubHeaderText(const FStringView& Text, FSlateColor Color)
	{
		static FTextBlockStyle ThisStyle = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("AnimViewport.MessageText");
		SlateIM::HAlign(HAlign_Fill);
		SlateIM::Text(Text, Color, &ThisStyle);
	}

	void WarningText(const FStringView& Text)
	{
		static FTextBlockStyle ThisStyle = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("RichTextBlock.BoldHighlight");
		SlateIM::HAlign(HAlign_Fill);
		SlateIM::Text(Text, FSlateColor(FColor(255, 95, 21)));
	}
	
	void ErrorText(const FStringView& Text)
	{
		static FTextBlockStyle ThisStyle = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("FoliageEditMode.ActiveToolName.Text");
		SlateIM::HAlign(HAlign_Fill);
		SlateIM::Text(Text, FSlateColor(FColor(255,51,51)), &ThisStyle);
	}

	void DrawLabledText(const FStringView& Label, const FStringView& Text)
	{
		static FTextBlockStyle ThisStyle = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("HardwareTargets.Strong");
		static FTextBlockStyle ThisStyleA = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("HardwareTargets.Normal");
		SlateIM::BeginHorizontalStack();
		SlateIM::HAlign(HAlign_Fill);
		SlateIM::Text(Label, &ThisStyle);
		SlateIM::Text(TEXT(" : "), &ThisStyle);
		SlateIM::HAlign(HAlign_Fill);
		SlateIM::Text(Text, &ThisStyleA);
		SlateIM::EndHorizontalStack();
	}

	void DrawLabledText(const FStringView& Label, FSlateColor LabelColor, const FStringView& Text)
	{
		static FTextBlockStyle ThisStyle = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("HardwareTargets.Strong");
		static FTextBlockStyle ThisStyleA = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("HardwareTargets.Normal");

		SlateIM::BeginHorizontalStack();
		SlateIM::HAlign(HAlign_Fill);
		SlateIM::Text(Label, LabelColor, &ThisStyle);
		SlateIM::Text(TEXT(" : "), LabelColor, &ThisStyle);
		SlateIM::HAlign(HAlign_Fill);
		SlateIM::Text(Text, &ThisStyleA);

		SlateIM::EndHorizontalStack();
	}

	void DrawLabledText(const FStringView& Label, FSlateColor LabelColor, const FStringView& Text, FSlateColor TextColor)
	{
		static FTextBlockStyle ThisStyle = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("HardwareTargets.Strong");
		static FTextBlockStyle ThisStyleA = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("HardwareTargets.Normal");

		SlateIM::BeginHorizontalStack();
		SlateIM::HAlign(HAlign_Fill);
		SlateIM::Text(Label, LabelColor, &ThisStyle);
		SlateIM::Text(TEXT(" : "), LabelColor, &ThisStyle);
		SlateIM::HAlign(HAlign_Fill);
		SlateIM::Text(Text, TextColor, &ThisStyleA);

		SlateIM::EndHorizontalStack();
	}

	void DrawLabledText(const FStringView& Label, const FStringView& Text, FSlateColor TextColor)
	{
		static FTextBlockStyle ThisStyle = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("HardwareTargets.Strong");
		static FTextBlockStyle ThisStyleA = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("HardwareTargets.Normal");

		SlateIM::BeginHorizontalStack();
		SlateIM::HAlign(HAlign_Fill);
		SlateIM::Text(Label, &ThisStyle);
		SlateIM::Text(TEXT(" : "), &ThisStyle);
		SlateIM::HAlign(HAlign_Fill);
		SlateIM::Text(Text, TextColor, &ThisStyleA);

		SlateIM::EndHorizontalStack();
	}
#endif
}
