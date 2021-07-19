import { singleton, inject } from "tsyringe";
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

@singleton()
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
                    status: "offline"
                };
            });
    }

    public postUp(speed: number): Promise<void>
    {
        const data: LiftSpeedMessage = {
            speed
        };

        return this.axios
            .post("lift/up", data)
            .then(() => {  });
    }

    public postDown(speed: number): Promise<void>
    {
        const data: LiftSpeedMessage = {
            speed
        };

        return this.axios
            .post("lift/down", data)
            .then(() => {  });
    }

    public postSpeed(speed: number): Promise<void>
    {
        const data: LiftSpeedMessage = {
            speed
        };

        return this.axios
            .post("lift/speed", data)
            .then(() => {  });
    }
}