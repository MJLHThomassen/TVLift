<template>
    <div class="debug row">
      <div class="col-sm">
          <h1>Debug</h1>
          <p>Log</p>
          <pre class="console"><span v-for="line in consoleLines" v-bind:style="line.css">{{ line.text }}</span></pre>
      </div>
    </div>
</template>

<script lang="ts">
import { Component, Vue, Prop } from "vue-property-decorator";
import * as Ansicolor from "ansicolor";

@Component({})
export default class Debug extends Vue
{
  private readonly WebsocktUri: string = "ws://" + location.host;

  private webSocket: WebSocket | null = null;
  private consoleText: string = "";
  private consoleLines: any[] = [];

  public constructor()
  {
    super();

    console.log("Setting up websocket to ", this.WebsocktUri);

    try
    {
      this.webSocket = new WebSocket(this.WebsocktUri);
    }
    catch (e)
    {
      this.consoleText = "Could not connect to debug socket: \n" + e;
      return;
    }

    setTimeout(() =>
    {
      if (this.webSocket != null)
      {
        if (this.webSocket.readyState !== WebSocket.OPEN)
        {
          this.webSocket.close();
        }
      }
    }, 1000);

    this.webSocket.onopen = this.onWebSocketOpen;
    this.webSocket.onerror = this.onWebSocketError;
    this.webSocket.onmessage = this.onWebSocketMessage;
    this.webSocket.onclose = this.onWebSocketClose;
  }

  private onWebSocketOpen(event: Event): void
  {
    this.addConsoleLines("Connected\n");
  }

  private onWebSocketError(event: Event): void
  {
    this.addConsoleLines("Could not connect to debug socket\n");
  }

  private onWebSocketMessage(event: MessageEvent): void
  {
    this.addConsoleLines(event.data);
  }

  private onWebSocketClose(event: Event): void
  {
    this.addConsoleLines("Closed\n");
  }

  private addConsoleLines(text: string): void
  {
    const parsed = Ansicolor.parse(text);
    console.log(parsed.spans);
    this.consoleLines.push(...parsed.spans);
  }
}
</script>

<style lang="scss">
.debug
{
  text-align: left;
  
  .console
  {
    
    height: 250px;

    span
    {
      font-family: 'Courier New', Courier, monospace;
    }
  }
}
</style>