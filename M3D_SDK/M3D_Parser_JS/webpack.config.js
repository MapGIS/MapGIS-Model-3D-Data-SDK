const path = require('path');

module.exports = {
    // 页面入口文件配置
    entry: `${__dirname}/src/index.js`,
    // 入口文件输出配置
    output: {
        path: path.resolve(__dirname, './dist'),
        // filename: `${packageName}.js`,
        // Needed to compile multiline strings in Cesium
        sourcePrefix: ''
    },
    amd: {
        // Enable webpack-friendly use of require in Cesium
        toUrlUndefined: true
    },
    node: {
        // Resolve node module use of fs
        fs: 'empty'
    },
    module: {
        rules: [
            {
                test: /\.css$/,
                use: ['style-loader', 'css-loader']
            },
            {
                test: /\.(png|gif|jpg|jpeg|svg|xml)$/,
                use: ['url-loader']
            }
        ]
    }
};
