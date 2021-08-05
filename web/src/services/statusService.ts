import { injectable, inject } from "tsyringe";

import { IStatusService, ConnectionStatus } from "@/services/iStatusService";
import { IWebsocketService } from "@/services/iWebsocketService";
import { LiftRepository, LiftStatus } from "@/repositories/liftRepository";
import { sleep } from "@/services/sleep";

@injectable()
export class StatusService implements IStatusService
{
    public get connectionStatus(): ConnectionStatus
    {
        return this.websocketService.connectionStatus;
    }

    public get isConnected(): boolean
    {
        return this.connectionStatus === ConnectionStatus.Connected;
    }

    public _liftStatus: LiftStatus = "unknown";
    public get liftStatus(): LiftStatus
    {
        return this._liftStatus;
    }

    public get isLiftOnline(): boolean
    {
        return this.liftStatus === "online";
    }

    public constructor(
        @inject("IWebsocketService") private readonly websocketService: IWebsocketService,
        private readonly liftRepository: LiftRepository)
    {
        this.pollLiftStatus();
    }

    private pollLiftStatus(): Promise<void>
    {
        return this.liftRepository.getStatus()
            .then(async (status) =>
            {
                this._liftStatus = status.status;
            })
            .then(() => sleep(1000))
            .then(() => this.pollLiftStatus());
    }
}
