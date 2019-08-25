module.exports = {
    outputDir: "../main/data/www",
    productionSourceMap: false,
    filenameHashing: false,
    chainWebpack: config =>
    {
        config.optimization.delete('splitChunks')
    }
  }