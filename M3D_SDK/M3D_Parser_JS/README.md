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

使用 `ES6` 标准时，可以直接引入对应的js文件，创建对应的图层时，方法如下

```javascript
import MapGISM3DSet from './MapGISM3DSet'

const m3dLayer = new MapGISM3DSet({
    context: windows.viewer.scene._context,
    url: url
});
```

使用 `ES5` 标准时，可以直接在 `html` 页面中，导入了 `Cesium` 包后，再导入打包好的脚本 `m3d-layer-min.js` 或 `m3d-layer-debug.js`，那么我们就可以直接在代码中使用 </br>
[示例代码](./index.html)

```javascript
let m3dLayer = new CesiumZondy.M3D.MapGISM3DSet({
    context: windows.viewer.scene._context,
    url: url
});
```

创建好图层对象后，添加到场景 `primitives` 中，然后跳转到图层位置即可

```javascript
let tileset = windows.viewer.scene.primitives.add(m3dLayer);

return tileset.readyPromise.then(function(tileset1) {
    let boundingSphere = tileset.boundingSphere;
    windows.viewer.camera.viewBoundingSphere(boundingSphere, new Cesium.HeadingPitchRange(0.0, -0.5,boundingSphere.radius));
    windows.viewer.camera.lookAtTransform(Cesium.Matrix4.IDENTITY);

    let cartographic = Cesium.Cartographic.fromCartesian(tileset.boundingSphere.center);
    let surface = Cesium.Cartesian3.fromRadians(cartographic.longitude, cartographic.latitude, 0.0);
    let offset = Cesium.Cartesian3.fromRadians(cartographic.longitude, cartographic.latitude, -0.5);
    let translation = Cesium.Cartesian3.subtract(offset, surface, new Cesium.Cartesian3());
    tileset.modelMatrix = Cesium.Matrix4.fromTranslation(translation);
}).otherwise(function(error) {
    throw (error);
});

```
