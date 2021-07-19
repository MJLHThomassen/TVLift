<template>
    <div class="card fluid home">
        <h2 class="section double-padded">Lift Controls</h2>
        <div class="section double-padded">
            <div class="container">
                <template v-if="statusService.isLiftOnline">
                    <div class="row">
                        <div class="col-sm">Speed: {{ currentSpeed }}</div>
                    </div>

                    <div class="row">
                        <div class="col-sm-4 col-sm-offset-4">
                            <button v-on:click="liftGoUp">
                                <arrow-up-icon size="5x" />
                            </button>
                        </div>
                    </div>

                    <div class="row">
                        <div class="col-sm-4">
                            <button v-on:click="liftSlower">
                                <minus-icon size="5x" />
                            </button>
                        </div>
                        <div class="col-sm-4">
                            <button v-on:click="liftStop">
                                <square-icon size="5x" class="stop-button" />
                            </button>
                        </div>
                        <div class="col-sm-4">
                            <button v-on:click="liftFaster">
                                <plus-icon size="5x" />
                            </button>
                        </div>
                    </div>

                    <div class="row">
                        <div class="col-sm-4 col-sm-offset-4">
                            <button v-on:click="liftGoDown">
                                <arrow-down-icon size="5x" />
                            </button>
                        </div>
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
import { Component, Vue, Inject } from "vue-property-decorator";
import { IStatusService } from "@/services/iStatusService";

import {
    ArrowUpIcon,
    ArrowDownIcon,
    SquareIcon,
    PlusIcon,
    MinusIcon,
} from "vue-feather-icons";
import { AxiosInstance } from "axios";

interface SpeedMessage
{
    speed: number;
}

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
    private currentSpeed: number = 0;

    @Inject()
    private readonly axios!: AxiosInstance;

    @Inject()
    private readonly statusService!: IStatusService;

    private mounted(): void
    {
        this.updateSpeed();
    }

    private liftGoUp(): void
    {
        this.axios.post("lift/up", <SpeedMessage>{ speed: this.currentSpeed });
    }

    private liftGoDown(): void
    {
        this.axios.post("lift/down", <SpeedMessage>{
            speed: this.currentSpeed,
        });
    }

    private liftStop(): void
    {
        this.axios.post("lift/stop");
    }

    private liftSlower(): void
    {
        this.updateSpeed()
            .then(currentSpeed =>
                this.axios.post("lift/speed",<SpeedMessage>{
                    speed: currentSpeed / 2,
                })
            )
            .catch(e =>
            {
                console.error(e);
            })
            .then(this.updateSpeed);
    }

    private liftFaster(): void
    {
        this.updateSpeed()
            .then(currentSpeed =>
                this.axios.post("lift/speed", <SpeedMessage>{
                    speed: currentSpeed * 2,
                })
            )
            .catch(e =>
            {
                console.error(e);
            })
            .then(this.updateSpeed);
    }

    private updateSpeed(): Promise<number>
    {
        return this.axios
            .get<SpeedMessage>("lift/speed")
            .then(x =>
            {
                this.currentSpeed = x.data.speed;
                return this.currentSpeed;
            })
            .catch(e =>
            {
                console.error(e);
                return this.currentSpeed;
            });
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
</style>

