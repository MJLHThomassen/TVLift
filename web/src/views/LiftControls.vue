<template>
    <div class="card fluid home">
        <h2 class="section double-padded">Lift Controls</h2>
        <div class="section double-padded">
            <div class="container">
                <template v-if="statusService.isLiftOnline">
                    <div class="row">
                        <div class="col-sm-4 col-sm-offset-4">
                            <button v-on:click="liftGoUp">
                                <arrow-up-icon size="5x" />
                            </button>
                        </div>
                    </div>

                    <div class="row">
                        <!-- <div class="col-sm-4">
                            <button v-on:click="liftSlower">
                                <minus-icon size="5x" />
                            </button>
                        </div> -->
                        <div class="col-sm-4">
                            <button v-on:click="liftStop">
                                <square-icon size="5x" class="stop-button" />
                            </button>
                        </div>
                        <!-- <div class="col-sm-4">
                            <button v-on:click="liftFaster">
                                <plus-icon size="5x" />
                            </button>
                        </div> -->
                    </div>

                    <div class="row">
                        <div class="col-sm-4 col-sm-offset-4">
                            <button v-on:click="liftGoDown">
                                <arrow-down-icon size="5x" />
                            </button>
                        </div>
                    </div>

                    <div>
                        <div class="col-sm">Speed: {{ currentSpeed }}</div>
                        <input type="range" orient="vertical" :min="minSpeed" :max="maxSpeed" step="1" v-model="currentSpeed">
                    </div>
                </template>

                <template v-else>
                    <div class="row">
                        <div class="col-sm-4 col-sm-offset-4">
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

    @Inject()
    private readonly liftRepository!: LiftRepository;

    @Inject()
    private readonly settingsRepository!: SettingsRepository;

    @Inject()
    private readonly statusService!: IStatusService;

    private async mounted(): Promise<void>
    {
        this.updateSpeed();

        const settings = await this.settingsRepository.getSettings();
        console.log(settings);
        this.minSpeed = settings.liftMinSpeed;
        this.maxSpeed = settings.liftMaxSpeed;
    }
    
    @Watch("statusService.liftStatus")
    private onLiftStatusChanged(val: LiftStatus, oldVal: LiftStatus): void
    {
        if(oldVal !== "online" && val === "online")
        {
            this.updateSpeed();
        }
    }

    @Watch("currentSpeed")
    private onCurrentSpeedChanged(val: number): void
    {
        console.log("onCurrentSpeedChanged called");
        this.do(val);
    }

    private doit = (val: number) =>
    {
        console.log("onCurrentSpeedChanged executed", this);
        this.liftRepository
            .postSpeed(val)
            .catch(console.error);
    };

    private do: (val: number) => void = _throttle(this.doit, 200);

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

    private async liftSlower(): Promise<void>
    {
        const currentSpeed = await this.updateSpeed();
        await this.liftRepository
            .postSpeed(currentSpeed/2)
            .catch(console.error);
        await this.updateSpeed();
    }

    private async liftFaster(): Promise<void>
    {
        const currentSpeed = await this.updateSpeed();
        await this.liftRepository
            .postSpeed(currentSpeed*2)
            .catch(console.error);
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
}
</style>

