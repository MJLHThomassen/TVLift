// const CompressionPlugin = require("compression-webpack-plugin");

// Set environment variables
process.env.VUE_APP_VERSION = Date().toString();

// Modify configuration
module.exports = {
    outputDir: "../firmware/data/www",
    filenameHashing: false,
    chainWebpack: config => {
        // Remove splitChunks
        config
            .optimization
            .delete("splitChunks");

        // Produce sourcemaps for development, but not for production since they don"t fit on the device
        config
            .devtool(process.env.NODE_ENV === "production" ? false : "source-map");

        // Replace vue-cli-service's progress output
        config
            .plugins
            .delete("progress");

        config
            .plugin("simple-progress-webpack-plugin")
            .use(require.resolve("simple-progress-webpack-plugin"), [
                {
                    format: "verbose", // options are minimal, compact, expanded, verbose
                },
            ]);

        // config
        //     .plugin("compression")
        //     .use(CompressionPlugin, 
        //         [{
        //             deleteOriginalAssets: true,
        //         }]);
    }
}
