<template>
    <div class="row">
        <div class="col-sm-12 col-md-6 col-md-offset-3">
            <div class="card fluid home">
                <h2 class="section double-padded">Lift Controls</h2>
                <div class="section double-padded">
                    <div class="container">
                        <template v-if="statusService.isLiftOnline">
                            <div class="row cols-sm">

                                <div>
                                    <div class="row">
                                        <div class="col-sm">
                                            <button v-on:click="liftGoUp">
                                                <arrow-up-icon size="5x" />
                                            </button>
                                        </div>
                                    </div>

                                    <div class="row">
                                        <div class="col-sm">
                                            <button v-on:click="liftStop">
                                                <square-icon size="5x" class="stop-button" />
                                            </button>
                                        </div>
                                    </div>

                                    <div class="row">
                                        <div class="col-sm">
                                            <button v-on:click="liftGoDown">
                                                <arrow-down-icon size="5x" />
                                            </button>
                                        </div>
                                    </div>
                                </div>
                            
                                <div>
                                    <input
                                        type="range"
                                        orient="vertical"
                                        :min="minSpeed"
                                        :max="maxSpeed"
                                        step="1"
                                        v-model="currentSpeed">
                                </div>
                            
                            </div>
                        </template>

                        <template v-else>
                            <div class="row">
                                <div class="col-sm-12">
                                    <div class="card fluid error">
                                        <h3>Error</h3>
                                        <p>Lift is not online!</p>
                                    </div>
                                </div>
                            </div>
                        </template>
                    </div>
                </div>
            </div>
        </div>
    </div>
</template>

<script lang="ts">
import { Component, Vue, Inject, Watch } from "vue-property-decorator";
import _throttle from "lodash/throttle";
import { LiftRepository, LiftStatus } from "@/repositories/liftRepository";
import { SettingsRepository } from "@/repositories/settingsRepository";
import { IStatusService } from "@/services/iStatusService";

import {
    ArrowUpIcon,
    ArrowDownIcon,
    SquareIcon,
    PlusIcon,
    MinusIcon,
} from "vue-feather-icons";

@Component({
    components: {
        ArrowUpIcon,
        ArrowDownIcon,
        SquareIcon,
        PlusIcon,
        MinusIcon,
    },
})
export default class LiftControls extends Vue
{
    private currentSpeed = 0;
    private minSpeed = 0;
    private maxSpeed = 0;
    private liftSetSpeedThrottled = (val: number) =>
    {
        console.warn(`liftSetSpeedThrottled(${val}) called before being initialized`);
    };

    @Inject()
    private readonly liftRepository!: LiftRepository;

    @Inject()
    private readonly settingsRepository!: SettingsRepository;

    @Inject()
    private readonly statusService!: IStatusService;

    private created(): void
    {
        this.liftSetSpeedThrottled = _throttle(this.liftSetSpeed, 500);
    }

    private mounted(): void
    {
        this.updateSpeed();
        this.updateSettings();
    }
    
    @Watch("statusService.liftStatus")
    private onLiftStatusChanged(val: LiftStatus, oldVal: LiftStatus): void
    {
        if(oldVal !== "online" && val === "online")
        {
            this.updateSpeed();
            this.updateSettings();
        }
    }

    @Watch("currentSpeed")
    private onCurrentSpeedChanged(val: number): void
    {
        this.liftSetSpeedThrottled(val);
    }

    private liftGoUp(): void
    {
        this.liftRepository
            .postUp(this.currentSpeed)
            .catch(console.error);
    }

    private liftGoDown(): void
    {
        this.liftRepository
            .postDown(this.currentSpeed)
            .catch(console.error);
    }

    private liftStop(): void
    {
        this.liftRepository
            .postStop()
            .catch(console.error);
    }

    private liftSetSpeed(val: number): Promise<void>
    {
        return this.liftRepository
            .postSpeed(val)
            .catch(console.error);
    }

    private async liftSlower(): Promise<void>
    {
        const currentSpeed = await this.updateSpeed();
        await this.liftSetSpeed(currentSpeed/2);
        await this.updateSpeed();
    }

    private async liftFaster(): Promise<void>
    {
        const currentSpeed = await this.updateSpeed();
        await this.liftSetSpeed(currentSpeed*2);
        await this.updateSpeed();
    }

    private async updateSpeed(): Promise<number>
    {
        try
        {
            const speedMessage = await this.liftRepository.getSpeed();
            this.currentSpeed = speedMessage.speed;
        }
        catch(e)
        {
            console.error(e);
        }

        return this.currentSpeed;
    }

    private async updateSettings(): Promise<void>
    {
        const settings = await this.settingsRepository.getSettings();
        this.minSpeed = settings.liftMinSpeed;
        this.maxSpeed = settings.liftMaxSpeed;
    }
}
</script>

<style lang="scss">
.home
{
    text-align: center;
}

.stop-button
{
    color: var(--error-color);
}

input[type="range"][orient="vertical"]
{
    writing-mode: bt-lr;
    -webkit-appearance: slider-vertical;

    height: 100%;
}
</style>

