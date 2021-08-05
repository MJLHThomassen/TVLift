import { LiftStatus } from "@/repositories/liftRepository";

export enum ConnectionStatus
{
    Disconnected,
    Connecting,
    Connected,
}

export interface IStatusService
{
    readonly connectionStatus: ConnectionStatus;
    readonly isConnected: boolean;

    readonly liftStatus: LiftStatus;
    readonly isLiftOnline: boolean;
}
