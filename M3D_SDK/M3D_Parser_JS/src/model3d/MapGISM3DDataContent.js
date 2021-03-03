import { CesiumZondy } from '../core/Base';
import MapGISM3DPointCloud from './MapGISM3DPointCloud';
import M3DModelParser from './M3DModelParser';

const sizeOfUint32 = Uint32Array.BYTES_PER_ELEMENT;
const propertyScratch1 = new Array(4);

function getBatchIdAttributeName(gltf) {
    let batchIdAttributeName = Cesium.ModelUtility.getAttributeOrUniformBySemantic(gltf, '_BATCHID');
    if (!Cesium.defined(batchIdAttributeName)) {
        batchIdAttributeName = Cesium.ModelUtility.getAttributeOrUniformBySemantic(gltf, 'BATCHID');
        if (Cesium.defined(batchIdAttributeName)) {
            Cesium.deprecationWarning(
                'm3d-legacy-batchid',
                'The glTF in this m3d uses the semantic `BATCHID`. Application-specific semantics should be prefixed with an underscore: `_BATCHID`.'
            );
        }
    }
    return batchIdAttributeName;
}

function getVertexShaderCallback(contentParam) {
    const content = contentParam;
    return (vs, programId) => {
        const batchTable = content._batchTable;
        const handleTranslucent = !Cesium.defined(content._tileset.classificationType);

        const { gltf } = content._model;
        if (Cesium.defined(gltf)) {
            content._batchIdAttributeName = getBatchIdAttributeName(gltf);
            content._diffuseAttributeOrUniformName[programId] = Cesium.ModelUtility.getDiffuseAttributeOrUniform(
                gltf,
                programId
            );
        }

        const callback = batchTable.getVertexShaderCallback(
            handleTranslucent,
            content._batchIdAttributeName,
            content._diffuseAttributeOrUniformName[programId]
        );
        return Cesium.defined(callback) ? callback(vs) : vs;
    };
}

function getFragmentShaderCallback(contentParam) {
    const content = contentParam;
    return (fs, programId) => {
        const batchTable = content._batchTable;
        const handleTranslucent = !Cesium.defined(content._tileset.classificationType);

        const { gltf } = content._model;
        if (Cesium.defined(gltf)) {
            content._diffuseAttributeOrUniformName[programId] = Cesium.ModelUtility.getDiffuseAttributeOrUniform(
                gltf,
                programId
            );
        }
        const callback = batchTable.getFragmentShaderCallback(
            handleTranslucent,
            content._diffuseAttributeOrUniformName[programId]
        );
        return Cesium.defined(callback) ? callback(fs) : fs;
    };
}

function getPickIdCallback(content) {
    return () => {
        return content._batchTable.getPickId();
    };
}

function getClassificationFragmentShaderCallback(content) {
    return (fs) => {
        const batchTable = content._batchTable;
        const callback = batchTable.getClassificationFragmentShaderCallback();
        return Cesium.defined(callback) ? callback(fs) : fs;
    };
}

function createColorChangedCallback(content) {
    return (batchId, color) => {
        content._model.updateCommands(batchId, color);
    };
}

function getVertexShaderLoaded(content) {
    return (vs) => {
        if (Cesium.defined(content._batchTable)) {
            return content._batchTable.getVertexShaderCallback(false, 'a_batchId', undefined)(vs);
        }
        return vs;
    };
}

function getFragmentShaderLoaded(content) {
    return (fs) => {
        if (Cesium.defined(content._batchTable)) {
            return content._batchTable.getFragmentShaderCallback(false, undefined)(fs);
        }
        return `uniform vec4 czm_pickColor;\n${fs}`;
    };
}

function getUniformMapLoaded(content) {
    return (uniformMap) => {
        if (Cesium.defined(content._batchTable)) {
            return content._batchTable.getUniformMapCallback()(uniformMap);
        }
        return Cesium.combine(uniformMap, {
            czm_pickColor() {
                return content._pickId.color;
            }
        });
    };
}

function getBatchTableLoaded(contentParam) {
    const content = contentParam;
    return (batchLength, batchTableJson, batchTableBinary) => {
        content._batchTable = new Cesium.Cesium3DTileBatchTable(content, batchLength, batchTableJson, batchTableBinary);
    };
}

function getPickIdLoaded(content) {
    return () => {
        return Cesium.defined(content._batchTable) ? content._batchTable.getPickId() : 'czm_pickColor';
    };
}

function getGeometricError(content) {
    const { pointCloudShading } = content._tileset;
    const sphereVolume = content._tile.contentBoundingVolume.boundingSphere.volume();
    const baseResolutionApproximation = Cesium.Math.cbrt(sphereVolume / content.pointsLength);

    let { geometricError } = content._tile;
    if (geometricError === 0) {
        if (Cesium.defined(pointCloudShading) && Cesium.defined(pointCloudShading.baseResolution)) {
            geometricError = pointCloudShading.baseResolution;
        } else {
            geometricError = baseResolutionApproximation;
        }
    }
    return geometricError;
}

function createFeatures(contentParam) {
    const content = contentParam;
    const { featuresLength } = content;
    if (!Cesium.defined(content._features) && featuresLength > 0) {
        const features = new Array(featuresLength);
        for (let i = 0; i < featuresLength; i += 1) {
            features[i] = new Cesium.Cesium3DTileFeature(content, i);
        }
        content._features = features;
    }
}

const scratchColor = new Cesium.Color();

function updateModel(ownerParam, tileset, frameState) {
    const owner = ownerParam;
    const commandStart = frameState.commandList.length;

    if (owner._model === undefined) {
        return;
    }
    // In the PROCESSING state we may be calling update() to move forward
    // the content's resource loading.  In the READY state, it will
    // actually generate commands.
    owner._batchTable.update(tileset, frameState);

    owner._contentModelMatrix = Cesium.Matrix4.multiply(
        owner._tile.computedTransform,
        owner._rtcCenterTransform,
        owner._contentModelMatrix
    );
    owner._model.modelMatrix = owner._contentModelMatrix;

    owner._model.update(frameState);

    // If any commands were pushed, add derived commands
    const commandEnd = frameState.commandList.length;
    if (
        commandStart < commandEnd &&
        (frameState.passes.render || frameState.passes.pick) &&
        !Cesium.defined(tileset.classificationType)
    ) {
        owner._batchTable.addDerivedCommands(frameState, commandStart);
    }
}

function updateInstance(ownerParam, tileset, frameState) {
    const owner = ownerParam;
    const commandStart = frameState.commandList.length;

    // In the PROCESSING state we may be calling update() to move forward
    // the content's resource loading.  In the READY state, it will
    // actually generate commands.
    owner._batchTable.update(tileset, frameState);

    const model = owner._modelInstanceCollection._model;

    owner._modelInstanceCollection.update(frameState);

    // If any commands were pushed, add derived commands
    const commandEnd = frameState.commandList.length;
    if (commandStart < commandEnd && (frameState.passes.render || frameState.passes.pick)) {
        owner._batchTable.addDerivedCommands(frameState, commandStart, false);
    }
}

const defaultShading = new Cesium.PointCloudShading();

function updatePointCloud(ownerParam, tileset, frameState) {
    const owner = ownerParam;
    const pointCloud = owner._pointCloud;
    const pointCloudShading = Cesium.defaultValue(tileset.pointCloudShading, defaultShading);
    const tile = owner._tile;
    const batchTable = owner._batchTable;
    const { mode } = frameState;
    const { clippingPlanes } = tileset;

    if (!Cesium.defined(owner._pickId) && !Cesium.defined(batchTable)) {
        owner._pickId = frameState.context.createPickId({
            primitive: tileset,
            content: owner
        });
    }

    if (Cesium.defined(batchTable)) {
        batchTable.update(tileset, frameState);
    }

    let boundingSphere;
    if (Cesium.defined(tile._contentBoundingVolume)) {
        boundingSphere =
            mode === Cesium.SceneMode.SCENE3D
                ? tile._contentBoundingVolume.boundingSphere
                : tile._contentBoundingVolume2D.boundingSphere;
    } else {
        boundingSphere =
            mode === Cesium.SceneMode.SCENE3D
                ? tile._boundingVolume.boundingSphere
                : tile._boundingVolume2D.boundingSphere;
    }

    const styleDirty = owner._styleDirty;
    owner._styleDirty = false;

    pointCloud.clippingPlanesOriginMatrix = tileset.clippingPlanesOriginMatrix;
    pointCloud.style = Cesium.defined(batchTable) ? undefined : tileset.style;
    pointCloud.styleDirty = styleDirty;
    pointCloud.modelMatrix = tile.computedTransform;
    pointCloud.time = tileset.timeSinceLoad;
    pointCloud.shadows = tileset.shadows;
    pointCloud.boundingSphere = boundingSphere;
    pointCloud.clippingPlanes = clippingPlanes;
    pointCloud.isClipped = Cesium.defined(clippingPlanes) && clippingPlanes.enabled && tile._isClipped;
    pointCloud.clippingPlanesDirty = tile.clippingPlanesDirty;
    pointCloud.attenuation = pointCloudShading.attenuation;
    pointCloud.backFaceCulling = pointCloudShading.backFaceCulling;
    pointCloud.normalShading = pointCloudShading.normalShading;
    pointCloud.geometricError = getGeometricError(owner);
    pointCloud.geometricErrorScale = pointCloudShading.geometricErrorScale;
    if (Cesium.defined(pointCloudShading) && Cesium.defined(pointCloudShading.maximumAttenuation)) {
        pointCloud.maximumAttenuation = pointCloudShading.maximumAttenuation;
    } else if (tile.refine === Cesium.Cesium3DTileRefine.ADD) {
        pointCloud.maximumAttenuation = 5.0;
    } else {
        pointCloud.maximumAttenuation = tileset.maximumScreenSpaceError;
    }

    pointCloud.update(frameState);
}

export default class MapGISM3DDataContent {
    constructor(tileset, tile, resource, arrayBuffer, byteOffset) {
        this._tileset = tileset;
        this._tile = tile;
        this._resource = resource;
        this._model = undefined;
        this._batchTable = undefined;
        this._features = undefined;
        this._batchIdAttributeName = undefined;
        this._diffuseAttributeOrUniformName = {};

        this._rtcCenterTransform = undefined;
        this._contentModelMatrix = undefined;

        this.featurePropertiesDirty = false;

        this._dataType = 0;

        this._pickId = undefined;
        this._styleDirty = false;
        this._pointCloud = undefined;

        MapGISM3DDataContent._initialize(this, arrayBuffer, byteOffset);
    }

    /**
     * @private
     */
    static _initializeModel(param) {
        const { view } = param;
        let { byteOffset } = param;
        const { uint8Array } = param;
        const { tileset } = param;
        const { resource } = param;
        const { parentNode } = param;
        const { arrayBuffer } = param;
        const { content } = param;
        const { byteStart } = param;
        const { allLength } = param;

        let featureTableJsonByteLength = 0; 

        let featureTableBinaryByteLength = 0;

        let batchTableJsonByteLength = 0;

        let batchTableBinaryByteLength = 0;

        if (param.tileset._version === '0.0') {
            const jsonIndexLength = view.getUint32(byteOffset, true);
            byteOffset += sizeOfUint32;
            let indexJson;
            if (jsonIndexLength > 0) {
                const jsonIndexString = Cesium.getStringFromTypedArray(uint8Array, byteOffset, jsonIndexLength);
                byteOffset += jsonIndexLength;
                indexJson = JSON.parse(jsonIndexString);
                console.log(indexJson);
                tileset.loadChildTileSet(resource, indexJson, parentNode);
            }
            byteOffset += sizeOfUint32;
            featureTableJsonByteLength = view.getUint32(byteOffset, true);
            byteOffset += sizeOfUint32;

            featureTableBinaryByteLength = view.getUint32(byteOffset, true);
            byteOffset += sizeOfUint32;

            batchTableJsonByteLength = view.getUint32(byteOffset, true);
            byteOffset += sizeOfUint32;

            batchTableBinaryByteLength = view.getUint32(byteOffset, true);
            byteOffset += sizeOfUint32;
        }

        let batchLength;

        if (batchTableJsonByteLength >= 570425344) {
            // First legacy check
            byteOffset -= sizeOfUint32 * 2;
            batchLength = featureTableJsonByteLength;
            batchTableJsonByteLength = featureTableBinaryByteLength;
            batchTableBinaryByteLength = 0;
            featureTableJsonByteLength = 0;
            featureTableBinaryByteLength = 0;
            MapGISM3DDataContent._deprecationWarning(
                'b3dm-legacy-header',
                'This b3dm header is using the legacy format [batchLength] [batchTableByteLength]. The new format is [featureTableJsonByteLength] [featureTableBinaryByteLength] [batchTableJsonByteLength] [batchTableBinaryByteLength] from https://github.com/AnalyticalGraphicsInc/3d-tiles/blob/master/TileFormats/Batched3DModel/README.md.'
            );
        } else if (batchTableBinaryByteLength >= 570425344) {
            // Second legacy check
            byteOffset -= sizeOfUint32;
            batchLength = batchTableJsonByteLength;
            batchTableJsonByteLength = featureTableJsonByteLength;
            batchTableBinaryByteLength = featureTableBinaryByteLength;
            featureTableJsonByteLength = 0;
            featureTableBinaryByteLength = 0;
            MapGISM3DDataContent._deprecationWarning(
                'b3dm-legacy-header',
                'This b3dm header is using the legacy format [batchTableJsonByteLength] [batchTableBinaryByteLength] [batchLength]. The new format is [featureTableJsonByteLength] [featureTableBinaryByteLength] [batchTableJsonByteLength] [batchTableBinaryByteLength] from https://github.com/AnalyticalGraphicsInc/3d-tiles/blob/master/TileFormats/Batched3DModel/README.md.'
            );
        }

        let featureTableJson;
        if (featureTableJsonByteLength === 0) {
            featureTableJson = {
                BATCH_LENGTH: Cesium.defaultValue(batchLength, 0)
            };
        } else {
            const featureTableString = Cesium.getStringFromTypedArray(
                uint8Array,
                byteOffset,
                featureTableJsonByteLength
            );
            featureTableJson = JSON.parse(featureTableString);
            byteOffset += featureTableJsonByteLength;
        }

        const featureTableBinary = new Uint8Array(arrayBuffer, byteOffset, featureTableBinaryByteLength);
        byteOffset += featureTableBinaryByteLength;

        const featureTable = new Cesium.Cesium3DTileFeatureTable(featureTableJson, featureTableBinary);

        batchLength = featureTable.getGlobalProperty('BATCH_LENGTH');
        featureTable.featuresLength = batchLength;

        let batchTableJson;
        let batchTableBinary;
        if (batchTableJsonByteLength > 0) {
            // PERFORMANCE_IDEA: is it possible to allocate this on-demand?  Perhaps keep the
            // arraybuffer/string compressed in memory and then decompress it when it is first accessed.
            //
            // We could also make another request for it, but that would make the property set/get
            // API async, and would double the number of numbers in some cases.
            const batchTableString = Cesium.getStringFromTypedArray(uint8Array, byteOffset, batchTableJsonByteLength);
            batchTableJson = JSON.parse(batchTableString);
            byteOffset += batchTableJsonByteLength;

            if (batchTableBinaryByteLength > 0) {
                // Has a batch table binary
                batchTableBinary = new Uint8Array(arrayBuffer, byteOffset, batchTableBinaryByteLength);
                // Copy the batchTableBinary section and let the underlying ArrayBuffer be freed
                batchTableBinary = new Uint8Array(batchTableBinary);
                byteOffset += batchTableBinaryByteLength;
            }
        }

        let colorChangedCallback;
        if (Cesium.defined(tileset.classificationType)) {
            colorChangedCallback = createColorChangedCallback(content);
        }

        const batchTable = new Cesium.Cesium3DTileBatchTable(
            content,
            batchLength,
            batchTableJson,
            batchTableBinary,
            colorChangedCallback
        );
        content._batchTable = batchTable;
        const gltfByteLength = byteStart + allLength - byteOffset;
        if (gltfByteLength === 0) {
            throw new Cesium.RuntimeError('模型数据长度为0.');
        }
        if (gltfByteLength < 240) {
            return;
        }

        let gltfView;
        if (byteOffset % 4 === 0) {
            if (param.tileset._version === '0.0') {
                gltfView = new Uint8Array(arrayBuffer, byteOffset, gltfByteLength);
            } else {
                gltfView = new Uint8Array(arrayBuffer, byteOffset, arrayBuffer.byteLength);
            }
        } else {
            MapGISM3DDataContent._deprecationWarning('字节对齐', '数据应该按4字节对齐');
            gltfView = new Uint8Array(uint8Array.subarray(byteOffset, byteOffset + gltfByteLength));
        }

        const pickObject = {
            content,
            primitive: tileset
        };

        content._rtcCenterTransform = Cesium.Matrix4.clone(Cesium.Matrix4.IDENTITY);
        const rtcCenter = featureTable.getGlobalProperty('RTC_CENTER', Cesium.ComponentDatatype.FLOAT, 3);
        if (Cesium.defined(rtcCenter)) {
            content._rtcCenterTransform = Cesium.Matrix4.fromTranslation(
                Cesium.Cartesian3.fromArray(rtcCenter),
                content._rtcCenterTransform
            );
        }

        content._contentModelMatrix = Cesium.Matrix4.multiply(
            parentNode.computedTransform,
            content._rtcCenterTransform,
            new Cesium.Matrix4()
        );
            
        content._model = new M3DModelParser({
            gltf: gltfView,
            cull: false, // The model is already culled by 3D Tiles
            releaseGltfJson: true, // Models are unique and will not benefit from caching so save memory
            opaquePass: Cesium.Pass.CESIUM_3D_TILE, // Draw opaque portions of the model during the 3D Tiles pass
            basePath: resource,
            requestType: Cesium.RequestType.TILES3D,
            modelMatrix: content._contentModelMatrix,
            upAxis: tileset._gltfUpAxis,
            forwardAxis: Cesium.Axis.X,
            shadows: tileset.shadows,
            debugWireframe: tileset.debugWireframe,
            incrementallyLoadTextures: false,
            vertexShaderLoaded: getVertexShaderCallback(content),
            fragmentShaderLoaded: getFragmentShaderCallback(content),
            uniformMapLoaded: batchTable.getUniformMapCallback(),
            pickIdLoaded: getPickIdCallback(content),
            addBatchIdToGeneratedShaders: batchLength > 0, // If the batch table has values in it, generated shaders will need a batchId attribute
            pickObject,
            m3dVersion: param.tileset._version
        });
    }

    /**
     * @private
     */
    static _initializeInstance(param) {
        const { view } = param;
        let { byteOffset } = param;
        const { uint8Array } = param;
        let { tileset } = param;
        const { arrayBuffer } = param;
        const { content } = param;
        const { byteStart } = param;
        const { allLength } = param;

        const indexJsonLength = view.getUint32(byteOffset, true);
        byteOffset += sizeOfUint32;

        const featureTableJsonByteLength = view.getUint32(byteOffset, true);
        if (featureTableJsonByteLength === 0) {
            throw new Cesium.RuntimeError('featureTableJsonByteLength is zero, the feature table must be defined.');
        }
        byteOffset += sizeOfUint32;

        const featureTableBinaryByteLength = view.getUint32(byteOffset, true);
        byteOffset += sizeOfUint32;

        const batchTableJsonByteLength = 0;

        const batchTableBinaryByteLength = 0;

        const gltfFormat = view.getUint32(byteOffset, true);
        if (gltfFormat !== 1 && gltfFormat !== 0) {
            throw new Cesium.RuntimeError(
                `Only glTF format 0 (uri) or 1 (embedded) are supported. Format ${gltfFormat} is not.`
            );
        }
        byteOffset += sizeOfUint32;

        byteOffset += indexJsonLength;

        const featureTableString = Cesium.getStringFromTypedArray(uint8Array, byteOffset, featureTableJsonByteLength);
        const featureTableJson = JSON.parse(featureTableString);
        byteOffset += featureTableJsonByteLength;

        const featureTableBinary = new Uint8Array(arrayBuffer, byteOffset, featureTableBinaryByteLength);
        byteOffset += featureTableBinaryByteLength;

        const featureTable = new Cesium.Cesium3DTileFeatureTable(featureTableJson, featureTableBinary);
        const instancesLength = featureTable.getGlobalProperty('INSTANCES_LENGTH');
        featureTable.featuresLength = instancesLength;

        if (!Cesium.defined(instancesLength)) {
            throw new Cesium.RuntimeError('Feature table global property: INSTANCES_LENGTH must be defined');
        }

        let batchTableJson;
        let batchTableBinary;
        if (batchTableJsonByteLength > 0) {
            const batchTableString = Cesium.getStringFromTypedArray(uint8Array, byteOffset, batchTableJsonByteLength);
            batchTableJson = JSON.parse(batchTableString);
            byteOffset += batchTableJsonByteLength;

            if (batchTableBinaryByteLength > 0) {
                // Has a batch table binary
                batchTableBinary = new Uint8Array(arrayBuffer, byteOffset, batchTableBinaryByteLength);
                // Copy the batchTableBinary section and let the underlying ArrayBuffer be freed
                batchTableBinary = new Uint8Array(batchTableBinary);
                byteOffset += batchTableBinaryByteLength;
            }
        }

        content._batchTable = new Cesium.Cesium3DTileBatchTable(
            content,
            instancesLength,
            batchTableJson,
            batchTableBinary
        );

        const gltfByteLength = byteStart + allLength - byteOffset;
        if (gltfByteLength === 0) {
            throw new Cesium.RuntimeError('glTF byte length is zero, i3dm must have a glTF to instance.');
        }

        let gltfView;
        if (byteOffset % 4 === 0) {
            gltfView = new Uint8Array(arrayBuffer, byteOffset, gltfByteLength);
        } else {
            // Create a copy of the glb so that it is 4-byte aligned
            MapGISM3DDataContent._deprecationWarning(
                'i3dm-glb-unaligned',
                'The embedded glb is not aligned to a 4-byte boundary.'
            );
            gltfView = new Uint8Array(uint8Array.subarray(byteOffset, byteOffset + gltfByteLength));
        }

        tileset = content._tileset;

        // Create model instance collection
        const collectionOptions = {
            instances: new Array(instancesLength),
            batchTable: content._batchTable,
            cull: false, // Already culled by 3D Tiles
            url: undefined,
            requestType: Cesium.RequestType.TILES3D,
            gltf: undefined,
            basePath: undefined,
            incrementallyLoadTextures: false,
            upAxis: tileset._gltfUpAxis,
            forwardAxis: Cesium.Axis.X,
            opaquePass: Cesium.Pass.CESIUM_3D_TILE, // Draw opaque portions during the 3D Tiles pass
            pickIdLoaded: getPickIdCallback(content),
            imageBasedLightingFactor: tileset.imageBasedLightingFactor,
            lightColor: tileset.lightColor,
            luminanceAtZenith: tileset.luminanceAtZenith,
            sphericalHarmonicCoefficients: tileset.sphericalHarmonicCoefficients,
            specularEnvironmentMaps: tileset.specularEnvironmentMaps
        };

        if (gltfFormat === 0) {
            let gltfUrl = Cesium.getStringFromTypedArray(gltfView);

            // We need to remove padding from the end of the model URL in case this tile was part of a composite tile.
            // This removes all white space and null characters from the end of the string.
            gltfUrl = gltfUrl.replace(/[\s\0]+$/, '');

            const resource = content._resource;
            if (resource.url.indexOf('igs/rest/g3d') > 0) {
                const urlGlb = `${
                    resource.url.substring(0, resource.url.lastIndexOf('&dataName=') + 10) + encodeURIComponent(gltfUrl)
                }&webGL=true&compress=false`;
                gltfUrl = urlGlb;
            } else {
                const url = gltfUrl.substring(4);
                gltfUrl = `..${url}`;
            }
            collectionOptions.url = content._resource.getDerivedResource({
                url: gltfUrl
            });
        } else {
            collectionOptions.gltf = gltfView;
            collectionOptions.basePath = content._resource.clone();
        }

        let rtcCenter;
        const rtcCenterArray = featureTable.getGlobalProperty('RTC_CENTER', Cesium.ComponentDatatype.FLOAT, 3);
        if (Cesium.defined(rtcCenterArray)) {
            rtcCenter = Cesium.Cartesian3.unpack(rtcCenterArray);
        }

        const { instances } = collectionOptions;
        const instancePosition = new Cesium.Cartesian3();
        const instancePositionArray = new Array(3);
        const instanceQuaternion = new Cesium.Quaternion();
        let instanceScale = new Cesium.Cartesian3();
        const instanceTranslationRotationScale = new Cesium.TranslationRotationScale();
        const instanceTransform = new Cesium.Matrix4();
        for (let i = 0; i < instancesLength; i += 1) {
            // Get the instance position
            let position = featureTable.getProperty('POSITION', Cesium.ComponentDatatype.FLOAT, 3, i, propertyScratch1);
            if (!Cesium.defined(position)) {
                position = instancePositionArray;
                const positionQuantized = featureTable.getProperty(
                    'POSITION_QUANTIZED',
                    Cesium.ComponentDatatype.UNSIGNED_SHORT,
                    3,
                    i,
                    propertyScratch1
                );
                if (!Cesium.defined(positionQuantized)) {
                    throw new Cesium.RuntimeError(
                        'Either POSITION or POSITION_QUANTIZED must be defined for each instance.'
                    );
                }
                const quantizedVolumeOffset = featureTable.getGlobalProperty(
                    'QUANTIZED_VOLUME_OFFSET',
                    Cesium.ComponentDatatype.FLOAT,
                    3
                );
                if (!Cesium.defined(quantizedVolumeOffset)) {
                    throw new Cesium.RuntimeError(
                        'Global property: QUANTIZED_VOLUME_OFFSET must be defined for quantized positions.'
                    );
                }
                const quantizedVolumeScale = featureTable.getGlobalProperty(
                    'QUANTIZED_VOLUME_SCALE',
                    Cesium.ComponentDatatype.FLOAT,
                    3
                );
                if (!Cesium.defined(quantizedVolumeScale)) {
                    throw new Cesium.RuntimeError(
                        'Global property: QUANTIZED_VOLUME_SCALE must be defined for quantized positions.'
                    );
                }
                for (let j = 0; j < 3; j += 1) {
                    position[j] = (positionQuantized[j] / 65535.0) * quantizedVolumeScale[j] + quantizedVolumeOffset[j];
                }
            }
            Cesium.Cartesian3.unpack(position, 0, instancePosition);
            if (Cesium.defined(rtcCenter)) {
                Cesium.Cartesian3.add(instancePosition, rtcCenter, instancePosition);
            }
            instanceTranslationRotationScale.translation = instancePosition;

            const rotarion = featureTable.getProperty(
                'ROTATION',
                Cesium.ComponentDatatype.FLOAT,
                4,
                i,
                propertyScratch1
            );
            const rotationCar4 = new Cesium.Cartesian4();
            Cesium.Cartesian4.unpack(rotarion, 0, rotationCar4);
            instanceQuaternion.w = rotationCar4.w;
            instanceQuaternion.x = rotationCar4.x;
            instanceQuaternion.y = rotationCar4.y;
            instanceQuaternion.z = rotationCar4.z;
            instanceTranslationRotationScale.rotation = instanceQuaternion;

            // Get the instance scale
            instanceScale = Cesium.Cartesian3.fromElements(1.0, 1.0, 1.0, instanceScale);
            const scale = featureTable.getProperty('SCALE', Cesium.ComponentDatatype.FLOAT, 1, i);
            if (Cesium.defined(scale)) {
                Cesium.Cartesian3.multiplyByScalar(instanceScale, scale, instanceScale);
            }
            const nonUniformScale = featureTable.getProperty(
                'SCALE_NON_UNIFORM',
                Cesium.ComponentDatatype.FLOAT,
                3,
                i,
                propertyScratch1
            );
            if (Cesium.defined(nonUniformScale)) {
                instanceScale.x *= nonUniformScale[0];
                instanceScale.y *= nonUniformScale[1];
                instanceScale.z *= nonUniformScale[2];
            }
            instanceTranslationRotationScale.scale = instanceScale;

            // Get the batchId
            let batchId = featureTable.getProperty('BATCH_ID', Cesium.ComponentDatatype.UNSIGNED_SHORT, 1, i);
            if (!Cesium.defined(batchId)) {
                // If BATCH_ID semantic is undefined, batchId is just the instance number
                batchId = i;
            }

            // Create the model matrix and the instance
            Cesium.Matrix4.fromTranslationRotationScale(instanceTranslationRotationScale, instanceTransform);
            const modelMatrix = instanceTransform.clone();
            instances[i] = {
                modelMatrix,
                batchId
            };
        }

        content._modelInstanceCollection = new Cesium.ModelInstanceCollection(collectionOptions);
    }

    /**
     * @private
     */
    static _initialize(contentParam, arrayBufferParam, byteOffsetParam) {
        const arrayBuffer = arrayBufferParam;
        let byteOffset = byteOffsetParam;
        const content = contentParam;
        const tileset = content._tileset;
        const parentNode = content._tile;
        const resource = content._resource;

        const byteStart = Cesium.defaultValue(byteOffset, 0);
        byteOffset = byteStart;

        const uint8Array = new Uint8Array(arrayBuffer);
        const view = new DataView(arrayBuffer);
      
        let dataType;
        if (contentParam._tileset._version === '0.0') {
            byteOffset += sizeOfUint32; // Skip magic
            const version = view.getUint32(byteOffset, true);
            if (version !== 1) {
                throw new Cesium.RuntimeError(`${'数据版本太旧, 当前数据版本为：'}${version}`);
            }
            byteOffset += sizeOfUint32;

            // 数据类型
            dataType = view.getUint32(byteOffset, true);
            byteOffset += sizeOfUint32;
        }
        if (contentParam._tileset._version === '2.0') {
            dataType = view.getUint32(byteOffset, true);
        }

        const param = {};
        param.view = view;
        param.byteOffset = byteOffset;
        param.uint8Array = uint8Array;
        param.tileset = tileset;
        param.resource = resource;
        param.parentNode = parentNode;
        param.arrayBuffer = arrayBuffer;
        param.content = content;
        param.byteStart = byteStart;

        // 文件总长度
        const allLength = 0;
        switch (dataType) {
            case 0:
                content._dataType = 0;
                allLength = view.getUint32(byteOffset, true);
                byteOffset += sizeOfUint32;
                param.byteOffset = byteOffset;
                param.allLength = allLength;
                MapGISM3DDataContent._initializeModel(param);
                break;
            case 1:
                content._dataType = 1;
                allLength = view.getUint32(byteOffset, true);
                byteOffset += sizeOfUint32;
                param.byteOffset = byteOffset;
                param.allLength = allLength;
                MapGISM3DDataContent._initializeInstance(param);
                break;
            case 2:
                content._dataType = 2;
                allLength = view.getUint32(byteOffset, true);
                byteOffset += sizeOfUint32;
                param.byteOffset = byteOffset;
                param.allLength = allLength;
                content._pointCloud = new MapGISM3DPointCloud({
                    param,
                    cull: false,
                    opaquePass: Cesium.Pass.CESIUM_3D_TILE,
                    vertexShaderLoaded: getVertexShaderLoaded(content),
                    fragmentShaderLoaded: getFragmentShaderLoaded(content),
                    uniformMapLoaded: getUniformMapLoaded(content),
                    batchTableLoaded: getBatchTableLoaded(content),
                    pickIdLoaded: getPickIdLoaded(content)
                });
                break;
            default:
                // 兼容旧版数据
                content._dataType = 0;
                param.allLength = dataType;
                MapGISM3DDataContent._initializeModel(param);
                break;
        }
    }

    static _deprecationWarning() {
        return Cesium.deprecationWarning;
    }

    get featuresLength() {
        if (Cesium.defined(this._batchTable)) {
            return this._batchTable.featuresLength;
        }
        return 0;
    }

    get pointsLength() {
        if (this._dataType === 2) {
            return this._pointCloud.pointsLength;
        }
        return 0;
    }

    get trianglesLength() {
        if (this._dataType === 0) {
            return this._model.trianglesLength;
        }
        if (this._dataType === 1) {
            const model = this._modelInstanceCollection._model;
            if (Cesium.defined(model)) {
                return model.trianglesLength;
            }
        } else if (this._dataType === 2) {
            // 点云
            return 0;
        } else {
            // 默认为模型
            return this._model.trianglesLength;
        }
        return undefined;
    }

    get geometryByteLength() {
        // return this._model.geometryByteLength;
        if (this._dataType === 0) {
            return this._model.geometryByteLength;
        }
        if (this._dataType === 1) {
            const model = this._modelInstanceCollection._model;
            if (Cesium.defined(model)) {
                return model.geometryByteLength;
            }
        } else if (this._dataType === 2) {
            // 点云
            return this._pointCloud.geometryByteLength;
        } else {
            // 默认为模型
            return this._model.geometryByteLength;
        }
        return undefined;
    }

    get texturesByteLength() {
        // return this._model.texturesByteLength;
        if (this._dataType === 0) {
            return this._model.texturesByteLength;
        }
        if (this._dataType === 1) {
            const model = this._modelInstanceCollection._model;
            if (Cesium.defined(model)) {
                return model.texturesByteLength;
            }
        } else if (this._dataType === 2) {
            // 点云
            return 0;
        } else {
            // 默认为模型
            return this._model.texturesByteLength;
        }
        return undefined;
    }

    get batchTableByteLength() {
        if (Cesium.defined(this._batchTable)) {
            return this._batchTable.memorySizeInBytes;
        }
        return 0;
    }

    // eslint-disable-next-line class-methods-use-this
    get innerContents() {
        return undefined;
    }

    get readyPromise() {
        if (this._dataType === 0) {
            if (this._model) {
                return this._model.readyPromise;
            }
        } else if (this._dataType === 1) {
            return this._modelInstanceCollection.readyPromise;
        } else if (this._dataType === 2) {
            // 点云.
            return this._pointCloud.readyPromise;
        } else {
            // 默认为模型.
            if (this._model) {
                return this._model.readyPromise;
            }
            return undefined;
        }
        return undefined;
    }

    get tileset() {
        return this._tileset;
    }

    get tile() {
        return this._tile;
    }

    get url() {
        return this._resource.getUrlComponent(true);
    }

    get batchTable() {
        return this._batchTable;
    }

    hasProperty(batchId, name) {
        return this._batchTable.hasProperty(batchId, name);
    }

    getFeature(batchId) {
        // >>includeStart('debug', pragmas.debug);
        const { featuresLength } = this;
        if (!Cesium.defined(batchId) || batchId < 0 || batchId >= featuresLength) {
            throw new Cesium.DeveloperError(
                `batchId is required and between zero and featuresLength - 1 (${featuresLength - 1}).`
            );
        }
        // >>includeEnd('debug');

        createFeatures(this);
        return this._features[batchId];
    }

    applyDebugSettings(enabled, colorParam) {
        let color = colorParam;
        color = enabled ? color : Cesium.Color.WHITE;
        if (this.featuresLength === 0) {
            this._model.color = color;
        } else {
            this._batchTable.setAllColor(color);
        }
    }

    applyStyle(style) {
        if (this._dataType === 1) {
            // 实例化
            this._batchTable.applyStyle(style);
        } else if (this._dataType === 2) {
            // 点云
            this._batchTable.applyStyle(style);
        } else if (this._dataType === 0) {
            // 原始数据
            if (this.featuresLength === 0) {
                const hasColorStyle = Cesium.defined(style) && Cesium.defined(style.color);
                const hasShowStyle = Cesium.defined(style) && Cesium.defined(style.show);
                this._model.color = hasColorStyle
                    ? style.color.evaluateColor(undefined, scratchColor)
                    : Cesium.Color.WHITE;
                this._model.show = hasShowStyle ? style.show.evaluate(undefined) : true;
            } else {
                this._batchTable.applyStyle(style);
            }
        }
    }

    update(tileset, frameState) {
        if (this._dataType === 0) {
            updateModel(this, tileset, frameState);
        }
        else if (this._dataType === 1) {
            updateInstance(this, tileset, frameState);
        } else if (this._dataType === 2) {
            // 点云
            updatePointCloud(this, tileset, frameState);
        } else {
            updateModel(this, tileset, frameState);
        }
    }

    // eslint-disable-next-line class-methods-use-this
    isDestroyed() {
        return false;
    }

    destroy() {
        this._model = this._model && this._model.destroy();
        this._modelInstanceCollection = this._modelInstanceCollection && this._modelInstanceCollection.destroy();
        this._pickId = this._pickId && this._pickId.destroy();
        this._pointCloud = this._pointCloud && this._pointCloud.destroy();
        this._batchTable = this._batchTable && this._batchTable.destroy();
        return Cesium.destroyObject(this);
    }
}

CesiumZondy.M3D.MapGISM3DDataContent = MapGISM3DDataContent;
