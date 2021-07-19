
const http = require("http");
const restServer = require("./restServer");
const createWebSocketServer = require("./websocketServer");
const { createHttpTerminator } = require("http-terminator");

class Webserver
{
    constructor(options)
    {
        this._options = {
            port: 8081,
            ...options
        };
    }
    
    start()
    {
        if(!this._terminator)
        {
            this._httpServer = http.createServer(restServer);
            this._terminator = createHttpTerminator({ server: this._httpServer });
            this._websocketServer = createWebSocketServer(this._httpServer);

            this._httpServer.listen(
                this._options.port,
                () => 
                {
                    const addressInfo = this._httpServer.address();
                    const isIPv6 = addressInfo.family === "IPv6";
                    const host = (isIPv6 ? "[" : "") + addressInfo.address + (isIPv6 ? "]" : "");
                    const port = addressInfo.port;
            
                    console.log("Server listening on", "http://" + host + ":" + port);
                }
            );
        }
    }
    
    async stop()
    {
        if(this._terminator)
        {
            await this._terminator.terminate();

            this._httpServer = undefined;
            this._terminator = undefined;
            this._websocketServer = undefined;
        }
    }
}

module.exports = Webserver;