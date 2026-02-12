// Fill out your copyright notice in the Description page of Project Settings.

#include "UIViewController.h"

UUIViewController::UUIViewController()
{
	View = nullptr;
	Model = nullptr;
}

void UUIViewController::OnViewCreated(UUserWidget* InView)
{
	View = InView;
	BindUIEvents();
}

void UUIViewController::OnViewWillAppear()
{
	// 子类可重写
}

void UUIViewController::OnViewDidAppear()
{
	// 子类可重写
}

void UUIViewController::OnViewWillDisappear()
{
	// 子类可重写
}

void UUIViewController::OnViewDidDisappear()
{
	// 子类可重写
}

void UUIViewController::OnViewDestroyed()
{
	View = nullptr;
	Model = nullptr;
}

void UUIViewController::OnFadeIn_Implementation(float Duration)
{
	// 默认实现，子类可重写
	if (View)
	{
		View->SetRenderOpacity(0.0f);
		// 这里可以通过UMG动画或自定义淡入逻辑实现
	}
}

void UUIViewController::OnFadeOut_Implementation(float Duration)
{
	// 默认实现，子类可重写
	if (View)
	{
		// 这里可以通过UMG动画或自定义淡出逻辑实现
	}
}

void UUIViewController::SetModel(UObject* InModel)
{
	Model = InModel;
	UpdateView();
}

void UUIViewController::BindUIEvents_Implementation()
{
	// 子类实现具体的UI事件绑定
}

void UUIViewController::UpdateView_Implementation()
{
	// 子类实现具体的UI更新逻辑
}

