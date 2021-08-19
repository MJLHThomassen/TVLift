import { injectable, inject } from "tsyringe";
import { AxiosInstance } from "axios";

export interface LiftSettingsMessage
{
    version?: number;
    liftMinSpeed: number;
    liftMaxSpeed: number;
    liftDefaultSpeed: number;
}

@injectable()
export class SettingsRepository
{
    constructor(@inject("AxiosInstance") private readonly axios: AxiosInstance)
    {
    }

    public getSettings(): Promise<LiftSettingsMessage>
    {
        return this.axios
            .get<LiftSettingsMessage>("settings")
            .then((x) =>
            {
                return x.data as LiftSettingsMessage;
            });
    }

    public postSettings(
        liftMinSpeed: number,
        liftMaxSpeed: number,
        liftDefaultSpeed: number): Promise<void>
    {
        const data: LiftSettingsMessage = {
            liftMinSpeed,
            liftMaxSpeed,
            liftDefaultSpeed
        };

        return this.axios
            .post("settings", data)
            .then(() => { return; });
    }
}
