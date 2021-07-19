import { ConnectionStatus } from "@/services/iStatusService";

export interface MessageReceivedCallback
{
    (event: MessageEvent): void;
}

export abstract class IWebsocketService
{
    readonly connectionStatus!: ConnectionStatus;
    abstract onMessageRecieved(callback: MessageReceivedCallback): void;
}