import { injectable, inject } from "tsyringe";
import { AxiosInstance } from "axios";

export type LiftStatus = "unknown" | "offline" | "online";

export interface LiftStatusMessage
{
    status: LiftStatus;
}

export interface LiftSpeedMessage
{
    speed: number;
}

@injectable()
export class LiftRepository
{
    constructor(@inject("AxiosInstance") private readonly axios: AxiosInstance)
    {
    }

    public getStatus(): Promise<LiftStatusMessage>
    {
        return this.axios
            .get<LiftStatusMessage>("lift/status")
            .then((x) =>
            {
                return x.data as LiftStatusMessage;
            })
            .catch((e) =>
            {
                console.error(e);
                return {
                    status: "offline",
                };
            });
    }

    public postUp(speed: number): Promise<void>
    {
        const data: LiftSpeedMessage = {
            speed,
        };

        return this.axios
            .post("lift/up", data)
            .then(() => { return; });
    }

    public postDown(speed: number): Promise<void>
    {
        const data: LiftSpeedMessage = {
            speed,
        };

        return this.axios
            .post("lift/down", data)
            .then(() => { return; });
    }

    public postStop(): Promise<void>
    {
        return this.axios
            .post("lift/stop")
            .then(() => { return; });
    }

    public getSpeed(): Promise<LiftSpeedMessage>
    {
        return this.axios
            .get<LiftSpeedMessage>("lift/speed")
            .then(x =>
            {
                return x.data as LiftSpeedMessage;
            });
    }

    public postSpeed(speed: number): Promise<void>
    {
        const data: LiftSpeedMessage = {
            speed,
        };

        return this.axios
            .post("lift/speed", data)
            .then(() => { return; });
    }
}
