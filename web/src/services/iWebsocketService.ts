import { ConnectionStatus } from "@/services/iStatusService";

export interface MessageReceivedCallback
{
    (event: MessageEvent<string>): void
}

export interface IWebsocketService
{
    readonly connectionStatus: ConnectionStatus;
    onMessageRecieved(callback: MessageReceivedCallback): void;
}
