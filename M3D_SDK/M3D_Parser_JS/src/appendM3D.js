import { CesiumZondy } from './core/Base';
import LayerType from './model3d/LayerType';
import MapGISM3DSet from './model3d/MapGISM3DSet';

function zoomToM3dLayer(viewer, layer) {
    const { boundingSphere } = layer;
    viewer.camera.viewBoundingSphere(boundingSphere, new Cesium.HeadingPitchRange(0.0, -0.5, boundingSphere.radius));
    viewer.camera.lookAtTransform(Cesium.Matrix4.IDENTITY);
}

function appendM3DLayer(viewer, baseUrl, renderIndex, layerIndex, gdbpUrl, visible, igserver, options) {
    const dataUrl = `${baseUrl}/GetDataStreams?sceneIndex=0&layerIndex=${renderIndex}&Level=0&Row=0&Col=0`;
    const showBoundingVolume = Cesium.defaultValue(options.debugShowBoundingVolume, true);
    const maxScreenError = Cesium.defaultValue(options.maximumScreenSpaceError, 16);
    const debugShowMemoryUsage = Cesium.defaultValue(options.debugShowMemoryUsage, false);
    const debugShowUrl = Cesium.defaultValue(options.debugShowUrl, false);

    const m3dLayr = new MapGISM3DSet({
        url: dataUrl,
        layerRenderIndex: renderIndex,
        layerIndex,
        gdbpUrl,
        show: visible,
        igserver,
        debugShowBoundingVolume: showBoundingVolume,
        maximumScreenSpaceError: maxScreenError,
        debugShowMemoryUsage,
        debugShowUrl
    });
    const tileset = viewer.scene.primitives.add(m3dLayr);
    const autoReset = Cesium.defaultValue(options.autoReset, true);
    if (autoReset) {
        tileset.readyPromise.then((layer) => zoomToM3dLayer(viewer, layer));
    }
    return tileset;
}

const callBack = (loaded, params) => {
    if (Cesium.defined(loaded) && typeof loaded === 'function') {
        loaded(params);
    }
};

const parseDocInfo = (viewer, baseUrl, info, docLayers, options) => {
    if (info !== undefined && info.sceneInfos.length > 0) {
        const { layers } = info.sceneInfos[0];
        layers.forEach((layer) => {
            const { layerType } = layer;
            const type = parseInt(layerType, 10);
            if (type === LayerType.M3DLAYER) {
                const { layerRenderIndex, layerIndex, gdbpUrl, isVisible } = layer;
                const m3d = appendM3DLayer(
                    viewer,
                    baseUrl,
                    layerRenderIndex,
                    layerIndex,
                    gdbpUrl,
                    isVisible,
                    true,
                    options
                );
                docLayers.push(m3d);
                m3d.readyPromise.then(callBack);
            }
        });
    }
};

function loadM3DByFile(viewer, options) {
    const m3dLayer = new MapGISM3DSet(options);

    const tileset = viewer.scene.primitives.add(m3dLayer);
    tileset.readyPromise
        .then((layer) => {
            const { boundingSphere } = layer;
            viewer.camera.viewBoundingSphere(
                boundingSphere,
                new Cesium.HeadingPitchRange(0.0, -0.5, boundingSphere.radius)
            );
            viewer.camera.lookAtTransform(Cesium.Matrix4.IDENTITY);
            // const cartographic = Cesium.Cartographic.fromCartesian(layer.boundingSphere.center);
            // const surface = Cesium.Cartesian3.fromRadians(cartographic.longitude, cartographic.latitude, 0.0);
            // const offset = Cesium.Cartesian3.fromRadians(cartographic.longitude, cartographic.latitude, 0.0);
            // const translation = Cesium.Cartesian3.subtract(offset, surface, new Cesium.Cartesian3());
            // const tmpLayer = layer;
            // tmpLayer.modelMatrix = Cesium.Matrix4.fromTranslation(translation);
        })
        .otherwise((error) => {
            throw error;
        });

    return tileset;
}

/**
 *
 * @param {Object} viewer 视图对象
 * @param {String} url 服务地址，可以使用文件服务地址，或者igs服务地址
 * @param {String} type 服务类型，'igs' 为igs服务地址，'file' 为文件服务地址
 * @param {Object} options {@link https://cesium.com/docs/cesiumjs-ref-doc/Cesium3DTileset.html}
 * @example
 * // 使用文件服务
 * let url = './data/景观_建筑模型/景观_建筑模型.mcj';
 * CesiumZondy.M3D.appendM3D(viewer, url, 'file', { maximumScreenSpaceError: 8 });
 *
 * // 使用 igs 服务
 * url = 'http://develop.smaryun.com:6163/igs/rest/g3d/ZondyModels';
 * CesiumZondy.M3D.appendM3D(viewer, url, 'igs', { maximumScreenSpaceError: 8 });
 */
export default function appendM3D(viewer, url, type, options) {
    if (!Cesium.defined(url)) {
        return new Cesium.DeveloperError('必须指定url');
    }

    // 记录文档中图层信息
    const docLayers = [];
    const baseUrl = url;
    let resource;
    let proxy;

    Cesium.defaultValue(options.proxy, new Cesium.DefaultProxy(options.proxy));
    const synchronous = Cesium.defaultValue(options.synchronous, true);

    switch (type) {
        case 'igs':
            {
                const getDoc = '/GetDocInfo';
                const requestUrl = `${baseUrl}${getDoc}`;
                if (synchronous) {
                    resource = new Cesium.Resource({
                        url: requestUrl,
                        proxy
                    });
                    resource.fetchJson().then((json) => parseDocInfo(viewer, baseUrl, json, docLayers, options));
                } else {
                    const request = new Cesium.XMLHttpRequest();
                    request.open('GET', requestUrl, false);
                    request.send(null);
                    if (request.status === 200) {
                        const info = Cesium.JSON.parse(request.responseText);
                        if (info) {
                            parseDocInfo(viewer, baseUrl, info, docLayers, options);
                        }
                    }
                }
            }
            break;
        case 'file':
        default:
            {
                const tmpOptions = options;
                tmpOptions.url = url;
                docLayers.push(loadM3DByFile(viewer, tmpOptions));
            }
            break;
    }

    return docLayers;
}

CesiumZondy.M3D.appendM3D = appendM3D;
