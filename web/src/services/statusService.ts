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

    public async refresh(): Promise<void>
    {
        const statusMessage = await this.liftRepository.getStatus();
        this._liftStatus = statusMessage.status;
    }

    private async pollLiftStatus(): Promise<void>
    {
        await this.refresh();

        if(this._liftStatus === "online")
        {
            await sleep(10000);
        }
        else
        {
            await sleep(1000);
        }
        
        return this.pollLiftStatus();
    }
}
