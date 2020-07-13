# Model-3D-Data

## 安装

下载 `Cesium` 库放在根目录的 `libs/` 文件夹下，如果没有请创建 </br>
[Cesium库下载地址](https://github.com/CesiumGS/cesium/releases)

使用 `npm` 安装依赖，打包代码，并启动服务

```cmd
// 进入到 M3D_Parser_JS 目录下
cd ./M3D_SDK/M3D_Parser_JS

// 安装依赖
npm install

// 打包代码
npm run release  // 编译压缩版本代码
npm run debug // 编译调试版本代码

// 启动服务
node server.js

```

最后运行 `index.html`

## 用法

基于 `vue` 框架使用时，为了保证效率，`viewer` 需要挂在 `windows` 变量下，否则大数据场景会严重影响性能

```javascript
const url = 'http://192.168.88.204:8093/M3D/kunshan_osgb/kunshan_osgb.mcj';

windows.viewer = new Cesium.Viewer('GlobeView', { terrainExaggeration: 1, infoBox: false });
```

在 `html` 网页中导入 `Cesium` 包后，再导入打包好的脚本 `m3d-layer-min.js` 或 `m3d-layer-debug.js`

```html
<script type="text/javascript" src="./libs/Cesium/Cesium.js"></script>
<!-- <script type="text/javascript" src="./dist/m3d-layer-min.js"></script> -->
<script type="text/javascript" src="./dist/m3d-layer-debug.js"></script>
```

导入脚本后

```javascript
// 创建场景视图
const viewer = new Cesium.Viewer('cesiumContainer', {
    terrainExaggeration: 1,
    requestRenderMode: true,
    animation: true,
    timeline: true
});
```

使用文件服务，既可以使用本地文件，也可以发布iis文件服务

```javascript
// 加载文件服务数据
const url = './data/景观_建筑模型/景观_建筑模型.mcj';
CesiumZondy.M3D.appendM3D(viewer, url, 'file', { maximumScreenSpaceError: 8 });
```

使用 `igserver` 数据服务

```javascript
// 加载igs服务数据
const url = 'http://develop.smaryun.com:6163/igs/rest/g3d/ZondyModels';
CesiumZondy.M3D.appendM3D(viewer, url, 'igs', { maximumScreenSpaceError: 8 });
```
