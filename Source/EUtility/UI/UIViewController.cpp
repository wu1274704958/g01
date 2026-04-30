// Fill out your copyright notice in the Description page of Project Settings.

#include "UIViewController.h"

UUIViewController::UUIViewController()
{
	View = nullptr;
	Model = nullptr;
}

void UUIViewController::OnViewCreated(UBaseWidget* InView)
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
	
}

void UUIViewController::OnFadeOut_Implementation(float Duration)
{
	
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
