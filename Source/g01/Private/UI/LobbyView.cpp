#include "LobbyView.h"

#include "PeerListItemWidget.h"
#include "Components/ScrollBox.h"

void ULobbyView::RefreshPeerList(const TArray<FPeerInfo>& PeerList, FOnConnectClickedDelegate OnConnectClicked)
{
	if (!scroll_peer_list) return;

	scroll_peer_list->ClearChildren();

	if (!PeerListItemClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("LobbyView::RefreshPeerList - PeerListItemClass is not set!"));
		return;
	}

	for (const FPeerInfo& Peer : PeerList)
	{
		UPeerListItemWidget* Item = CreateWidget<UPeerListItemWidget>(this, PeerListItemClass);
		if (!Item) continue;

		Item->SetPeerInfo(Peer);

		// 把 Controller 传进来的 delegate 转接到每个 item 的按钮上
		Item->OnConnectClicked.AddLambda([OnConnectClicked](int64 PeerId)
		{
			OnConnectClicked.ExecuteIfBound(PeerId);
		});

		scroll_peer_list->AddChild(Item);
	}
}
