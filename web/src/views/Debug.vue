<template>
    <div class="debug">
        <h2>Debug</h2>
        <console :new-entry="newEntry"></console>
    </div>
</template>

<script lang="ts">
import { Vue, Component, Inject } from "vue-property-decorator";
import { IWebsocketService } from "@/services/iWebsocketService";

import Console from "@/components/Console.vue";

@Component({
    components: {
        Console,
    },
})
export default class Debug extends Vue
{
    private readonly WebsocktUri: string = "ws://" + location.host + "/api";

    @Inject()
    private readonly websocketService!: IWebsocketService;

    private newEntry: string = "";

    private created(): void
    {
        this.websocketService.onMessageRecieved((event) => this.onWebSocketMessage(event));
    }

    private onWebSocketMessage(event: MessageEvent): void
    {
        this.newEntry = event.data;
    }
}
</script>

<style lang="scss">
.debug
{
    max-height: 100%;
    overflow: hidden;

    display: flex;
    flex-direction: column;
    flex-wrap: nowrap;
    justify-content: flex-start;
    align-items: stretch;

    text-align: center;

    >*
    {
        flex-grow: 0;
        flex-shrink: 0;
        flex-basis: 0;
    }
}
</style>