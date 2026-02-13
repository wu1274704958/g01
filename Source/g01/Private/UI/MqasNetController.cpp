// Fill out your copyright notice in the Description page of Project Settings.

#include "MqasNetController.h"
#include "EUtility/UI/UIUtility.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"
#include "P2PHelperStream.h"
#include "EUtility/UI/UIManager.h"

UMqasNetController::UMqasNetController()
{
	AgentNet = nullptr;
	BtnConnect = nullptr;
	EtIp = nullptr;
	EtPort = nullptr;
	EtName = nullptr;
	EtPsd = nullptr;
	TbStatus = nullptr;
	TbBtnConnect = nullptr;
}

void UMqasNetController::OnViewCreated(UUserWidget* InView)
{
	Super::OnViewCreated(InView);
}

void UMqasNetController::OnViewWillAppear()
{
	// 尝试获取Agent
	AgentNet = GetMqasNetAgent();
	
	Super::OnViewWillAppear();

	// 更新UI显示
	UpdateView();
}

void UMqasNetController::OnViewDidDisappear()
{
	Super::OnViewDidDisappear();

	// 断开连接
	if (auto Connect = CurrentConnect.lock())
	{
		if (Connect->IsConnected())
		{
			Connect->Disconnect();
		}
	}
	CurrentConnect.reset();
}

void UMqasNetController::BindUIEvents_Implementation()
{
	Super::BindUIEvents_Implementation();

	if (!View)
	{
		return;
	}

	// 绑定UI控件
	BtnConnect = UIUtility::BindWidget<UButton>(View, TEXT("btn_connect"), [this](UButton* Btn)
	{
		FScriptDelegate Delegate;
		Delegate.BindUFunction(this, "OnConnectButtonClicked");
		Btn->OnClicked.Add(Delegate);
	});

	EtIp = UIUtility::BindWidget<UEditableText>(View, TEXT("et_ip"));
	EtPort = UIUtility::BindWidget<UEditableText>(View, TEXT("et_port"));
	EtName = UIUtility::BindWidget<UEditableText>(View, TEXT("et_name"));
	EtPsd = UIUtility::BindWidget<UEditableText>(View, TEXT("et_psd"));
	TbStatus = UIUtility::BindWidget<UTextBlock>(View, TEXT("tb_status"));
	TbBtnConnect = UIUtility::BindWidget<UTextBlock>(View, TEXT("tb_btn_connect"));
}

void UMqasNetController::UpdateView_Implementation()
{
	Super::UpdateView_Implementation();

	UMqasNetModel* MqasModel = GetMqasModel();
	if (!MqasModel)
	{
		return;
	}

	// 更新输入框
	if (EtIp)
	{
		EtIp->SetText(FText::FromString(MqasModel->IpAddress));
	}

	if (EtPort)
	{
		FNumberFormattingOptions Options;
		Options.SetUseGrouping(false);
		EtPort->SetText(FText::AsNumber(MqasModel->Port, &Options));
	}

	if (EtName)
	{
		EtName->SetText(FText::FromString(MqasModel->UserName));
	}

	if (EtPsd)
	{
		EtPsd->SetText(FText::FromString(MqasModel->Password));
	}

	// 更新状态文本
	if (TbStatus)
	{
		TbStatus->SetText(FText::FromString(MqasModel->StatusText));
	}

	if (TbBtnConnect)
	{
		TbBtnConnect->SetText(FText::FromString(MqasModel->ButtonText));
	}
}

AUMqasNetAgent* UMqasNetController::GetMqasNetAgent()
{
	UMqasNetModel* MqasModel = GetMqasModel();
	if (MqasModel)
	{
		return MqasModel->NetAgent;
	}
	return nullptr;
}

void UMqasNetController::OnConnectButtonClicked()
{
	//test hide self
	UUIManager::Get(GetWorld())->HideUI(FName("TestMqasNet"));
	return;
	UMqasNetModel* MqasModel = GetMqasModel();
	if (!MqasModel)
	{
		return;
	}

	// 如果已经有连接，处理断开逻辑
	if (auto Connect = CurrentConnect.lock())
	{
		if (Connect->IsConnected())
		{
			Connect->Disconnect();
		}
		else
		{
			ShowToast(TEXT("Connecting... Please wait!"));
		}
		return;
	}

	// 验证输入
	if (!ValidateInput())
	{
		return;
	}

	// 检查Agent
	if (!AgentNet)
	{
		AgentNet = GetMqasNetAgent();
	}

	if (!AgentNet)
	{
		ShowToast(TEXT("MqasNetAgent is null!"));
		return;
	}

	// 从UI获取最新的输入值
	FString Ip = EtIp ? EtIp->GetText().ToString() : MqasModel->IpAddress;
	int32 Port = EtPort ? FCString::Atoi(*EtPort->GetText().ToString()) : MqasModel->Port;
	FString Name = EtName ? EtName->GetText().ToString() : MqasModel->UserName;
	FString Password = EtPsd ? EtPsd->GetText().ToString() : MqasModel->Password;

	// 更新Model
	MqasModel->IpAddress = Ip;
	MqasModel->Port = Port;
	MqasModel->UserName = Name;
	MqasModel->Password = Password;

	// 发起连接
	CurrentConnect = AgentNet->ConnectTo(1, TEXT("NetConfig/hole_punching_config.txt"), Ip, Port);

	if (auto Connect = CurrentConnect.lock())
	{
		auto Id = Connect->GetHwnd();
		MqasModel->ConnectionHandle = Id;
		MqasModel->bIsConnecting = true;
		MqasModel->StatusText = FString::Printf(TEXT("Connecting... (Handle=%u)"), Id);

		if (TbStatus)
		{
			TbStatus->SetText(FText::FromString(MqasModel->StatusText));
		}

		// 绑定连接回调
		Connect->OnConnectedDelegate.AddUObject(this, &UMqasNetController::OnConnected);
		Connect->OnConnectErrorDelegate.AddUObject(this, &UMqasNetController::OnConnectError);
		Connect->OnDisconnectedDelegate.AddUObject(this, &UMqasNetController::OnDisconnected);
	}
	else
	{
		ShowToast(TEXT("Connect failed!"));
		MqasModel->bIsConnecting = false;
	}
}

void UMqasNetController::OnConnectError(int Code, GSY_ConnectionHwnd Hwnd)
{
	UMqasNetModel* MqasModel = GetMqasModel();
	if (MqasModel)
	{
		MqasModel->bIsConnected = false;
		MqasModel->bIsConnecting = false;
		MqasModel->StatusText = FString::Printf(TEXT("Connect failed! (Error=%u)"), Code);
		MqasModel->ButtonText = TEXT("Connect");
	}

	if (TbStatus)
	{
		TbStatus->SetText(FText::FromString(FString::Printf(TEXT("Connect failed! (Error=%u)"), Code)));
	}

	if (TbBtnConnect)
	{
		TbBtnConnect->SetText(FText::FromString(TEXT("Connect")));
	}
}

void UMqasNetController::OnConnected(int Code, GSY_ConnectionHwnd Hwnd)
{
	UMqasNetModel* MqasModel = GetMqasModel();
	if (MqasModel)
	{
		MqasModel->bIsConnected = true;
		MqasModel->bIsConnecting = false;
		MqasModel->ConnectionHandle = Hwnd;
		MqasModel->StatusText = FString::Printf(TEXT("Connected (Handle=%u)"), Hwnd);
		MqasModel->ButtonText = TEXT("Disconnect");
	}

	if (TbStatus)
	{
		TbStatus->SetText(FText::FromString(FString::Printf(TEXT("Connected (Handle=%u)"), Hwnd)));
	}

	if (TbBtnConnect)
	{
		TbBtnConnect->SetText(FText::FromString(TEXT("Disconnect")));
	}

	// 注册到Lobby
	auto Conn = CurrentConnect.lock();
	if (Conn && MqasModel)
	{
		Conn->MakeStream<P2PHelperStream, GSY_LobbyStreamContext, const char*, const char*>(
			GSY_RegisterToLobby,
			TCHAR_TO_UTF8(*MqasModel->UserName),
			TCHAR_TO_UTF8(*MqasModel->Password)
		);
	}
}

void UMqasNetController::OnDisconnected(int Code, GSY_ConnectionHwnd Hwnd)
{
	UMqasNetModel* MqasModel = GetMqasModel();
	if (MqasModel)
	{
		MqasModel->bIsConnected = false;
		MqasModel->bIsConnecting = false;
		MqasModel->StatusText = FString::Printf(TEXT("Disconnected (Code=%u)"), Code);
		MqasModel->ButtonText = TEXT("Connect");
	}

	if (TbStatus)
	{
		TbStatus->SetText(FText::FromString(FString::Printf(TEXT("Disconnected (Code=%u)"), Code)));
	}

	if (TbBtnConnect)
	{
		TbBtnConnect->SetText(FText::FromString(TEXT("Connect")));
	}

	CurrentConnect.reset();
}

void UMqasNetController::ShowToast(const FString& Message)
{
	UMqasNetModel* MqasModel = GetMqasModel();
	if (MqasModel && MqasModel->ToastClass)
	{
		UIUtility::ShowToast(this, 0, MqasModel->ToastClass, Message);
	}
}

bool UMqasNetController::ValidateInput()
{
	if (!EtName || !EtPsd || !EtIp || !EtPort)
	{
		ShowToast(TEXT("UI controls not initialized!"));
		return false;
	}

	if (EtName->GetText().IsEmpty() || EtPsd->GetText().IsEmpty())
	{
		ShowToast(TEXT("Please input name and password!"));
		return false;
	}

	if (EtIp->GetText().IsEmpty() || EtPort->GetText().IsEmpty())
	{
		ShowToast(TEXT("Please input IP and Port!"));
		return false;
	}

	return true;
}

UMqasNetModel* UMqasNetController::GetMqasModel() const
{
	auto model = GetModel();
	return Cast<UMqasNetModel>(model);
}


