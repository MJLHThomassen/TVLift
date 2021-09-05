import { injectable, inject } from "tsyringe";
import { ConnectionStatus } from "@/services/iStatusService";
import { IWebsocketService, MessageReceivedCallback } from "@/services/iWebsocketService";
import { sleep, promiseWithTimeout } from "@/services/sleep";

enum WebsocketPromiseRejectionReasons
{
    Error,
    Timeout
}

@injectable()
export class WebsocketService implements IWebsocketService
{
    private readonly websocketTimeoutTimeMs = 2000;
    private readonly websocketRetryTimeMs = 1000;
    private readonly websocketUri: URL;

    private webSocket: WebSocket | null = null;
    private messageRecievedCallbacks: MessageReceivedCallback[] = [];

    private _connectionStatus: ConnectionStatus = ConnectionStatus.Disconnected;
    public get connectionStatus(): ConnectionStatus
    {
        return this._connectionStatus;
    }

    public constructor(@inject("apiUri") apiUri: URL)
    {
        apiUri.protocol = "ws";
        this.websocketUri = apiUri;

        console.info("websocketuri: ", this.websocketUri);

        this.initialiseWebSocket();
    }

    public onMessageRecieved(callback: MessageReceivedCallback): void
    {
        this.messageRecievedCallbacks.push(callback);
    }

    private async initialiseWebSocket(): Promise<void>
    {
        this.webSocket = await this.connectWebSocket();

        this.webSocket.onclose = () =>
        {
            this._connectionStatus = ConnectionStatus.Disconnected;
            this.webSocket = null;

            // Re-initialize a new websocket
            this.initialiseWebSocket();
        };

        this.webSocket.onmessage = this.onWebSocketMessage.bind(this);
    }

    private destroyWebSocket(): void
    {
        this.webSocket?.close();
    }

    private connectWebSocket(): Promise<WebSocket>
    {
        this._connectionStatus = ConnectionStatus.Connecting;

        // Construct a websocket
        let webSocket: WebSocket;
        try
        {
            webSocket = new WebSocket(this.websocketUri.toString());
        }
        catch (e)
        {
            // If there is an exception (SECURITY_ERR), retry immediately
            // TODO: Is this the best way to handle this?
            return sleep(this.websocketRetryTimeMs).then(() => this.connectWebSocket());
        }

        // Construct a promise that resolves when the websocket is succesfully connected,
        // or rejects when the websocket could not connect due to an error.
        const websocketPromise = new Promise<WebSocket>((resolve, reject) =>
        {
            webSocket.onopen = () =>
            {
                this._connectionStatus = ConnectionStatus.Connected;
                resolve(webSocket); 
            };
            webSocket.onerror = () =>
            {
                this._connectionStatus = ConnectionStatus.Disconnected;
                reject(WebsocketPromiseRejectionReasons.Error);
            };
        });

        // Race between websocket connection/error and a timeout.
        // Immediately returns the websocket via the resolve of the websocketPromise if everything goes ok.
        // Executes the catch if either the websocket connection errored, or the timeout was reached.
        return promiseWithTimeout(websocketPromise, this.websocketTimeoutTimeMs, WebsocketPromiseRejectionReasons.Timeout)
            .catch((reason: WebsocketPromiseRejectionReasons) =>
            {
                let sleepBeforRetryingMs = 0;
                if(reason !== WebsocketPromiseRejectionReasons.Timeout)
                {
                    // Wait a bit before retrying if we did not time out.
                    // If we did time out, we've already waited long enough.
                    sleepBeforRetryingMs = this.websocketRetryTimeMs;
                }

                // Reconnect if timeout or error ocurred.
                return sleep(sleepBeforRetryingMs).then(() => this.connectWebSocket());
            });
    }

    private onWebSocketMessage(event: MessageEvent<string>): void
    {
        for(const callback of this.messageRecievedCallbacks)
        {
            callback(event);
        }
    }
}