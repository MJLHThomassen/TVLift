<template>
    <div class="debug">
      <h2>Debug</h2>
      <wifi-off-icon class="disconnected-icon" v-if="webSocket === null"></wifi-off-icon>
      <console :new-entry="newEntry"></console>
    </div>
</template>

<script lang="ts">
import { Component, Vue, Prop } from "vue-property-decorator";
import { timeout } from "@/services/sleep";
import Console from "@/components/Console.vue";
import { WifiOffIcon } from "vue-feather-icons";
import * as Ansicolor from "ansicolor";

@Component({
  components: {
    Console,
    WifiOffIcon,
  },
})
export default class Debug extends Vue
{
  private readonly WebsocktUri: string = "ws://" + location.host + "/api";

  private reconnectWebSocket: boolean = true;
  private webSocket: WebSocket | null = null;
  private newEntry: string = "";

  public constructor()
  {
    super();
  }

  public async mounted(): Promise<void>
  {
    this.initialiseWebSocket();
  }

  public beforeDestroy(): void
  {
    this.destroyWebSocket();
  }

  private async initialiseWebSocket(): Promise<void>
  {
    this.webSocket = await this.connectWebSocket();

    this.webSocket.onclose = (e) =>
    {
      this.webSocket = null;

      if (this.reconnectWebSocket)
      {
        // Re-initialize a new websocket
        this.initialiseWebSocket();
      }
    };

    this.webSocket.onmessage = this.onWebSocketMessage;
  }

  private destroyWebSocket(): void
  {
    this.reconnectWebSocket = false;
    this.webSocket?.close();
  }

  private connectWebSocket(): Promise<WebSocket>
  {
    // 2s connection/retry timeout
    const websocketTimeoutTimeMs = 2000;

    // Construct a websocket
    let webSocket: WebSocket;
    try
    {
      webSocket = new WebSocket(this.WebsocktUri);
    }
    catch (e)
    {
      // If there is an exception (SECURITY_ERR), retry immediately
      // TODO: Is this the best way to handle this?
      return this.connectWebSocket();
    }

    // Construct a promise that resolves when the websocket is succesfully connected,
    // or rejects when the websocket could not connect due to an error.
    const websocketPromise = new Promise<WebSocket>((resolve, reject) =>
    {
      webSocket.onopen = (e) => { resolve(webSocket); };
      webSocket.onerror = (e) => { reject(); };
    });

    // Race between websocket connection/error and a timeout.
    // Immediately returns the websocket via the resolve of the websocketPromise if everything goes ok.
    // Executes the catch if either the websocket connection errored, or the timeout was reached.
    return Promise
      .race([websocketPromise, timeout<WebSocket>(websocketTimeoutTimeMs)])
      .catch((reason) =>
      {
        if (this.reconnectWebSocket)
        {
          // Reconnect if timeout or error ocurred.
          return this.connectWebSocket();
        }
        else
        {
          // If we don't want to reconnect, return the (failed) socket.
          return webSocket;
        }
      });
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

    .disconnected-icon 
    {
        position: absolute;
        right: 0;
        margin: calc(.5rem + 4px) calc(.5rem + 4px) 0 0;
        color: var(--error-color);
    }
}
</style>