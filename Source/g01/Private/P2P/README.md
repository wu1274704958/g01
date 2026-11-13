# P2P role state synchronization

The lobby accepts incoming peer requests. A successful helper result starts one authenticated peer. After the datagram stream is ready, the link controller spawns a remote copy of the local character and adds a state sender component to the local character. The component sends position, rotation and velocity at 15 Hz; the receiver rejects malformed and out-of-order packets. This is a prototype snapshot protocol: it does not yet provide interpolation, action-event delivery, prediction or authoritative validation.

`Content/NetConfig/conf_p2p.txt` is the peer engine configuration and is copied to Saved on startup. It uses the lobby socket and does not need a certificate pin. It must contain a non-empty `[p2p] name` and `[lsquic_settings] es_datagrams=1`; both are needed by the SDK data stream. The locally configured `Content/NetConfig/hole_punching_config.txt` must specify the trusted lobby server's `server_cert_sha256` (64 hexadecimal SHA-256 digits); the helper requires a pinned lobby connection. Provision that fingerprint for the actual server before testing. The existing lobby configuration is local and ignored by git. Both clients must have their own available lobby bind port when running on the same host.

The current lobby UI automatically accepts incoming requests. Only direct connections are supported; relay results are rejected and released. Keep the lobby open while negotiating; once authenticated the peer survives independently of the lobby UI.

The default `TestMqasNet` map uses `GM_TestMqas`, whose default pawn is `DefaultPawn`, not a `Character`. It can test lobby and authenticated P2P connectivity, but character synchronization requires a map with a local Character pawn.
