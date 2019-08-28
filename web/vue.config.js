const CompressionPlugin = require('compression-webpack-plugin');

module.exports = {
    outputDir: "../main/data/www",
    productionSourceMap: false,
    filenameHashing: false,
    chainWebpack: config =>
    {
        config.optimization.delete("splitChunks");
        // config
        //     .plugin("compression")
        //     .use(CompressionPlugin, 
        //         [{
        //             deleteOriginalAssets: true,
        //         }]);
    }
  }