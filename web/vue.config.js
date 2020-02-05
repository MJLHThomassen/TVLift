const CompressionPlugin = require('compression-webpack-plugin');

module.exports = {
    outputDir: "../main/data/www",
    productionSourceMap: false,
    filenameHashing: false,
    chainWebpack: config => 
    {
        // Remove splitChunks
        config
            .optimization
            .delete("splitChunks");

        // Proxy websocket to our test tool hosted on port 8081
        config
            .devServer
            .proxy({
                "/api": {
                    target: "http://localhost:8081",
                    ws: true,
                }
            });

        // config
        //     .plugin("compression")
        //     .use(CompressionPlugin, 
        //         [{
        //             deleteOriginalAssets: true,
        //         }]);
    }
}