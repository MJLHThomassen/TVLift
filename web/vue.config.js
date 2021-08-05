// const CompressionPlugin = require('compression-webpack-plugin');

// Set environment variables
process.env.VUE_APP_VERSION = Date().toString();

// Modify configuration
module.exports = {
    outputDir: "../firmware/data/www",
    filenameHashing: false,
    // css: {
    //     loaderOptions: {
    //         sass: {
    //             implementation: require('sass'),
    //         },
    //     },
    // },
    chainWebpack: config => {
        // Remove splitChunks
        config
            .optimization
            .delete("splitChunks");

        config
            .devtool("source-map");

        // config
        //     .plugin("compression")
        //     .use(CompressionPlugin, 
        //         [{
        //             deleteOriginalAssets: true,
        //         }]);
    }
}
