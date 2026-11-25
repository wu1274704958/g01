# MqasNet P2P C++ 使用示例

在大厅 `ILobbyEventListener::OnHelperResult` 中，保存传入的 `FP2PHelperResult`，并持有下面的两个对象直到连接关闭。所有公开方法和事件在游戏线程使用。

```cpp
Peer = FMqasPeer::Create();
Datagram = Peer->AddStream<FMqasDatagram>(101); // 每条 stream 使用不为零且在本端唯一的业务 ID。
Peer->OnIncomingStream.AddLambda([](std::shared_ptr<BaseDirectStream> Stream) {
    GSY_P2PStreamId Local = 0, Remote = 0;
    if (Stream->GetStreamIds(Local, Remote) == EC_Ok) {
        // 根据双方 ID 绑定接收逻辑；此时控制 stream 已连接。
    }
});
Peer->OnConnected.AddLambda([this, Result] {
    if (!Result.Data.is_server)
        Datagram->Open();
});
Datagram->OnConnected.AddLambda([this] {
    Datagram->OpenChannel(); // 只有业务需要收发 Datagram 时才开通。
});
Datagram->OnChannelOpened.AddLambda([this] {
    TArray<uint8> Message{1, 2, 3};
    Datagram->Send(Message);
});
Datagram->OnReceived.AddLambda([](const TArray<uint8>& Message) { /* 处理数据 */ });
Peer->OnClosed.AddLambda([](ErrorCode Error) { /* 处理关闭 */ });
const ErrorCode ResultCode = Peer->Start(ClientId, Config, LobbyStream, Result);
// EC_Pending 表示请求已接受，其他结果为同步失败。
```

`Peer->OnConnected` 表示 P2P 传输已通过认证，SDK 会在回调前消费 session handle；`Datagram->OnConnected` 表示控制 stream 已建立。需要发送报文的一端显式调用 `OpenChannel`，成功后收到 `OnChannelOpened`，可调用 `CloseChannel` 关闭本端发送通道。两端需要互相发送时分别开通各自的发送通道。接收方通过基础 server 启动，登记会话后由大厅通知主动方。主动方收到对应 session 的 ready 事件后，通过 Agent 的 session 入口建立基础连接。

异步失败时封装清理未消费的会话。同步启动失败由调用方调用 `Result.ReleaseSession()` 清理。主动结束时调用 `Peer->Close()` 并持有对象直到终止事件。当前支持 IPv4 直连。

同一 P2P peer 可以持有多条控制 stream，双方均可发起创建。对端创建的 stream 在握手完成后通过 `OnIncomingStream` 交给业务；如果事先以协商后的本地 ID 绑定了对象，会复用该对象，否则自动创建。`FindDirectStream` 根据对端业务 ID 查找已连接的 stream。每条 stream 独立管理本端发送通道，报文由 SDK 按 stream ID 分发。`FMqasAgent::RetainPeer` 保留 peer 及其管理的 stream；`FindPeers` 可查找同一远端的多个已保留连接。游戏现有的 `P2PLinkController` 仍按远端 ID 限制一个角色同步链接。

SDK 更新：先构建 gsync 的对应平台和配置，再在 Source/MqasNet 目录执行 python copy_file.py <gsync 根目录>；头文件、导入库和运行时 DLL 必须来自同一版本。output 为本机产物，不纳入版本管理。


## Agent lifecycle and PIE

`FMqasAgent` owns the process-wide SDK runtime. Each `UMqasClientSubsystem` creates a separate logical client for its GameInstance; any `AUMqasNetAgent` in that GameInstance forwards to that client. Destroying an actor or changing levels does not close its connections. Releasing the GameInstance closes only its connections and peers; releasing the last client drains and terminates the SDK. GSY_connect reuses a compatible engine for a fixed bind endpoint; port 0 creates a separate engine and UDP socket. Use a lobby configuration with `port=0` for multiple local clients. Runtime configurations are written under `Saved/MqasClients/<process>-<client>/`.

A new level can call `FMqasAgent::FindPeer` with its subsystem client ID and remote peer ID to recover an existing P2P connection and use `FindLocalStream` to retrieve its business stream. The g01 link controller exposes `AttachLink` for this purpose. It does not automatically travel or bind preplaced characters.

To discover links after a level change, call `UMqasClientSubsystem::GetPeerIds()` on that level's GameInstance. It returns the remote IDs of this client’s retained peers, including connections still being established; the caller can use an ID with `AP2PLinkController::AttachLink`. Each PIE GameInstance sees only its own client’s peers. Closing a client invalidates its held streams after the SDK has drained; callbacks queued by an older SDK runtime are discarded after a restart.
